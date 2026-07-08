#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/shm.h>
#include <linux/fb.h>
#include <signal.h>
#include <sys/time.h>
#include <math.h>
#include "yolov8-pose.h"
#include "retinaface.h"
#include "image_utils.h"
#include "file_utils.h"
#include "image_drawing.h"
#include "shared_mem.h"

// 颜色定义（若头文件未提供）
#ifndef COLOR_CYAN
#define COLOR_CYAN 0x00FFFF
#endif

// 骨架连线（保留备用）
int skeleton[38] = {16,14,14,12,17,15,15,13,12,13,6,12,7,13,6,7,6,8,7,9,8,10,9,11,2,3,1,2,1,3,2,4,3,5,4,6,5,7};

// ==================== 硬件配置 ====================
#define SCREEN_W        1024
#define SCREEN_H        600

// ==================== 疲劳检测参数 ====================
#define EAR_THRESHOLD           0.20f
#define MAR_THRESHOLD           0.65f
#define HEAD_DOWN_PITCH         25.0f
#define YAW_THRESH              25.0f
#define ROLL_THRESH             20.0f
#define PERCLOS_WINDOW          150
#define PERCLOS_LIMIT           0.30f
#define YAWN_FRAMES             5
#define HANDS_OFF_THRESH        80
#define GAZE_OFFSET_RATIO       0.30f
#define GAZE_FRAMES             30
#define WARNING_FRAMES          45
#define ALARM_FRAMES            90
#define ALARM_HOLD_FRAMES       150
#define SCORE_EYE_CLOSED_LONG   60
#define SCORE_EYE_CLOSED_SHORT  30
#define SCORE_HEAD_DOWN         20
#define SCORE_YAWN              40
#define SCORE_HEAD_TURN         20
#define SCORE_HANDS_OFF         30
#define SCORE_GAZE_OFF          15
#define SCORE_DECAY_PER_FRAME   2
#define LEVEL1_SCORE            50
#define LEVEL2_SCORE            100

static volatile int running = 1;
void sigint_handler(int sig) { running = 0; }

// ==================== 辅助函数 ====================
static long long get_time_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (long long)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

static float distance(float x1, float y1, float x2, float y2) {
    float dx = x1 - x2;
    float dy = y1 - y2;
    return sqrtf(dx*dx + dy*dy);
}

// 最近邻缩放（锐度优先）
static void resize_rgb_nearest(const unsigned char *src, int src_w, int src_h,
                               unsigned char *dst, int dst_w, int dst_h) {
    for (int y = 0; y < dst_h; y++) {
        int src_y = y * src_h / dst_h;
        for (int x = 0; x < dst_w; x++) {
            int src_x = x * src_w / dst_w;
            memcpy(dst + (y * dst_w + x) * 3,
                   src + (src_y * src_w + src_x) * 3, 3);
        }
    }
}

// ==================== 卡尔曼滤波器 ====================
typedef struct {
    float x; float P; float Q; float R;
} Kalman1D;

static void kalman_init(Kalman1D *k, float init_val) {
    k->x = init_val; k->P = 1.0f; k->Q = 0.05f; k->R = 0.1f;
}

static float kalman_update(Kalman1D *k, float measurement) {
    float x_pred = k->x;
    float P_pred = k->P + k->Q;
    float K = P_pred / (P_pred + k->R);
    k->x = x_pred + K * (measurement - x_pred);
    k->P = (1 - K) * P_pred;
    return k->x;
}

// ==================== 特征提取 ====================
static float calculate_ear(float left_eye[2], float right_eye[2]) {
    static float last_ear = 0.3f;
    float eye_dist = distance(left_eye[0], left_eye[1], right_eye[0], right_eye[1]);
    float ear = eye_dist / 60.0f;
    if (ear > 0.4f) ear = 0.4f;
    last_ear = last_ear * 0.7f + ear * 0.3f;
    return last_ear;
}

static float calculate_mar(retinaface_object_t *face) {
    if (!face) return 0.0f;
    float left_mouth_x = (float)face->point[3].x;
    float left_mouth_y = (float)face->point[3].y;
    float right_mouth_x = (float)face->point[4].x;
    float right_mouth_y = (float)face->point[4].y;
    float mouth_width = distance(left_mouth_x, left_mouth_y, right_mouth_x, right_mouth_y);
    if (mouth_width < 1e-6) return 0.0f;
    float nose_x = (float)face->point[2].x;
    float nose_y = (float)face->point[2].y;
    float mouth_center_x = (left_mouth_x + right_mouth_x) * 0.5f;
    float mouth_center_y = (left_mouth_y + right_mouth_y) * 0.5f;
    float mouth_height = distance(mouth_center_x, mouth_center_y, nose_x, nose_y);
    return mouth_height / mouth_width;
}

static void calc_head_angles(float nose_x, float nose_y,
                             float left_eye_x, float left_eye_y,
                             float right_eye_x, float right_eye_y,
                             float *yaw, float *roll) {
    float eye_center_x = (left_eye_x + right_eye_x) * 0.5f;
    float eye_dist = distance(left_eye_x, left_eye_y, right_eye_x, right_eye_y);
    if (eye_dist < 1e-6) { *yaw = 0.0f; *roll = 0.0f; return; }
    *yaw = (nose_x - eye_center_x) / eye_dist * 30.0f;
    *roll = atan2(right_eye_y - left_eye_y, eye_dist) * 180.0f / M_PI;
}

// ==================== 状态机 ====================
typedef enum { STATE_NORMAL, STATE_WARNING, STATE_ALARM } FatigueState;

typedef struct {
    float ear, mar, pitch, yaw, roll;
    int hands_off, gaze_off;
    int yawn_cnt;
    int perclos_buffer[PERCLOS_WINDOW];
    int perclos_idx;
    FatigueState state;
    int warning_timer, alarm_timer, alert_hold;
    int fatigue_score;
    Kalman1D kal_ear, kal_pitch;
} DriverState;

static void driver_state_init(DriverState *ds) {
    memset(ds, 0, sizeof(DriverState));
    ds->state = STATE_NORMAL;
    ds->ear = 0.3f;
    ds->fatigue_score = 0;
    kalman_init(&ds->kal_ear, 0.3f);
    kalman_init(&ds->kal_pitch, 0.0f);
}

static void update_perclos(DriverState *ds, int is_eye_closed) {
    ds->perclos_buffer[ds->perclos_idx] = is_eye_closed;
    ds->perclos_idx = (ds->perclos_idx + 1) % PERCLOS_WINDOW;
    int closed = 0;
    for (int i = 0; i < PERCLOS_WINDOW; i++) closed += ds->perclos_buffer[i];
    float perclos = (float)closed / PERCLOS_WINDOW;
    if (perclos > PERCLOS_LIMIT) ds->fatigue_score += 30;
}

static void update_state_machine(DriverState *ds) {
    int risk = (ds->ear < EAR_THRESHOLD) || (ds->mar > MAR_THRESHOLD) ||
               (ds->pitch > HEAD_DOWN_PITCH) || (fabs(ds->yaw) > YAW_THRESH) ||
               ds->hands_off || ds->gaze_off;
    
    ds->fatigue_score -= SCORE_DECAY_PER_FRAME;
    if (ds->fatigue_score < 0) ds->fatigue_score = 0;
    
    if (risk) {
        if (ds->ear < EAR_THRESHOLD) ds->fatigue_score += SCORE_EYE_CLOSED_SHORT;
        if (ds->mar > MAR_THRESHOLD) ds->fatigue_score += SCORE_YAWN;
        if (ds->pitch > HEAD_DOWN_PITCH) ds->fatigue_score += SCORE_HEAD_DOWN;
        if (fabs(ds->yaw) > YAW_THRESH) ds->fatigue_score += SCORE_HEAD_TURN;
        if (ds->hands_off) ds->fatigue_score += SCORE_HANDS_OFF;
        if (ds->gaze_off) ds->fatigue_score += SCORE_GAZE_OFF;
    }
    
    if (ds->alert_hold > 0) { ds->alert_hold--; return; }
    
    if (ds->fatigue_score >= LEVEL2_SCORE) {
        if (ds->state != STATE_ALARM) {
            ds->state = STATE_ALARM;
            ds->alert_hold = ALARM_HOLD_FRAMES;
            printf("🚨 LEVEL 2 ALARM TRIGGERED!\n");
        }
    } else if (ds->fatigue_score >= LEVEL1_SCORE) {
        if (ds->state == STATE_NORMAL) {
            ds->state = STATE_WARNING;
            printf("⚠️ LEVEL 1 WARNING\n");
        }
    } else {
        if (ds->state != STATE_NORMAL) {
            ds->state = STATE_NORMAL;
            printf("✅ Back to normal\n");
        }
    }
}

static void set_alarm_hardware(int level) {
    if (level >= 2) {
        printf("🔔 ALARM: Level 2 - Severe Fatigue!\n");
    } else if (level == 1) {
        printf("⚠️ WARNING: Level 1 - Mild Fatigue\n");
    }
}

// ==================== 主函数 ====================
int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Usage: %s <pose.rknn> <face.rknn>\n", argv[0]);
        return -1;
    }
    signal(SIGINT, sigint_handler);

    // ========== 1. 挂载共享内存 ==========
    int shm_id = shmget(SHM_KEY, sizeof(SharedFrame), 0666);
    if (shm_id < 0) {
        printf("Shared memory not found. Please start gst_producer first!\n");
        printf("Run: ./gst_producer &\n");
        return -1;
    }
    SharedFrame *shared_frame = (SharedFrame*)shmat(shm_id, NULL, 0);
    if (shared_frame == (void*)-1) {
        perror("shmat");
        return -1;
    }
    printf("Connected to shared memory, waiting for frames...\n");

    // ========== 2. 初始化模型 ==========
    rknn_app_context_t pose_ctx, face_ctx;
    memset(&pose_ctx, 0, sizeof(pose_ctx));
    memset(&face_ctx, 0, sizeof(face_ctx));
    init_post_process();
    
    if (init_yolov8_pose_model(argv[1], &pose_ctx) != 0) goto out;
    if (init_retinaface_model(argv[2], &face_ctx) != 0) goto out_pose;

    // ========== 3. 初始化 Framebuffer ==========
    int fb_fd = open("/dev/fb0", O_RDWR);
    if (fb_fd < 0) { perror("open fb"); goto out_face; }
    
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    if (ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo) < 0) { perror("fb var"); goto close_fb; }
    if (ioctl(fb_fd, FBIOGET_FSCREENINFO, &finfo) < 0) { perror("fb fix"); goto close_fb; }
    if (vinfo.bits_per_pixel != 32) {
        printf("Only 32bpp framebuffer supported\n");
        goto close_fb;
    }
    
    unsigned char *fb_mmap = (unsigned char*)mmap(0, finfo.smem_len, PROT_READ|PROT_WRITE, MAP_SHARED, fb_fd, 0);
    if (fb_mmap == MAP_FAILED) { perror("fb mmap"); goto close_fb; }

    // ========== 4. 分配缓冲区 ==========
    unsigned char *rgb_buf = (unsigned char*)malloc(CAM_WIDTH * CAM_HEIGHT * 3);
    unsigned char *scaled_buf = (unsigned char*)malloc(SCREEN_W * SCREEN_H * 3);
    image_buffer_t src_image, retina_input;
    memset(&src_image, 0, sizeof(src_image));
    memset(&retina_input, 0, sizeof(retina_input));
    
    if (!rgb_buf || !scaled_buf) { perror("malloc"); goto close_fb; }

    // ========== 5. 初始化状态机 ==========
    DriverState ds;
    driver_state_init(&ds);
    
    int frame_count = 0;
    int last_frame = 0;

    // ========== 6. 主循环 ==========
    while (running) {
        // 等待新帧
        if (shared_frame->frame_count == last_frame) {
            usleep(10000);
            continue;
        }
        last_frame = shared_frame->frame_count;
        
        // 复制图像到本地缓冲区
        memcpy(rgb_buf, shared_frame->rgb_data, CAM_WIDTH * CAM_HEIGHT * 3);
        
        // 构建 image_buffer_t
        src_image.width = CAM_WIDTH;
        src_image.height = CAM_HEIGHT;
        src_image.format = IMAGE_FORMAT_RGB888;
        src_image.size = CAM_WIDTH * CAM_HEIGHT * 3;
        src_image.virt_addr = rgb_buf;

        // YOLO 推理
        object_detect_result_list pose_results;
        memset(&pose_results, 0, sizeof(pose_results));
        inference_yolov8_pose_model(&pose_ctx, &src_image, &pose_results);

        // RetinaFace 推理
        resize_image_cpu(&src_image, &retina_input, 320, 320);
        retinaface_result face_results;
        memset(&face_results, 0, sizeof(face_results));
        inference_retinaface_model(&face_ctx, &retina_input, &face_results);
        
        // 坐标映射
        float scale_x = (float)CAM_WIDTH / 320.0f;
        float scale_y = (float)CAM_HEIGHT / 320.0f;
        for (int i = 0; i < face_results.count; i++) {
            retinaface_object_t *obj = &face_results.object[i];
            obj->box.left   = (int)(obj->box.left * scale_x);
            obj->box.right  = (int)(obj->box.right * scale_x);
            obj->box.top    = (int)(obj->box.top * scale_y);
            obj->box.bottom = (int)(obj->box.bottom * scale_y);
            for (int j = 0; j < 5; j++) {
                obj->point[j].x = (int)(obj->point[j].x * scale_x);
                obj->point[j].y = (int)(obj->point[j].y * scale_y);
            }
        }

        // 特征提取
        ds.ear = 0.3f; ds.mar = 0.0f; ds.pitch = 0.0f; ds.yaw = 0.0f; ds.roll = 0.0f;
        ds.hands_off = 0; ds.gaze_off = 0;

        if (pose_results.count > 0) {
            object_detect_result *det = &pose_results.results[0];
            if (det->keypoints[0][2] > 0.3f && det->keypoints[1][2] > 0.1f && det->keypoints[2][2] > 0.1f) {
                float nose_y = det->keypoints[0][1];
                float left_eye_y = det->keypoints[1][1];
                float right_eye_y = det->keypoints[2][1];
                float eyes_center_y = (left_eye_y + right_eye_y) * 0.5f;
                ds.pitch = nose_y - eyes_center_y;
                ds.pitch = kalman_update(&ds.kal_pitch, ds.pitch);
            }
        }

        if (face_results.count > 0) {
            retinaface_object_t *face = &face_results.object[0];
            ds.mar = calculate_mar(face);
            float left_eye[2] = {(float)face->point[0].x, (float)face->point[0].y};
            float right_eye[2] = {(float)face->point[1].x, (float)face->point[1].y};
            ds.ear = calculate_ear(left_eye, right_eye);
            ds.ear = kalman_update(&ds.kal_ear, ds.ear);
            calc_head_angles((float)face->point[2].x, (float)face->point[2].y,
                             (float)face->point[0].x, (float)face->point[0].y,
                             (float)face->point[1].x, (float)face->point[1].y,
                             &ds.yaw, &ds.roll);
        }

        int is_eye_closed = (ds.ear < EAR_THRESHOLD);
        update_perclos(&ds, is_eye_closed);
        
        static int yawn_cnt = 0;
        if (ds.mar > MAR_THRESHOLD) { 
            yawn_cnt++; 
            if (yawn_cnt >= YAWN_FRAMES) ds.fatigue_score += SCORE_YAWN; 
        } else { 
            yawn_cnt = 0; 
        }

        update_state_machine(&ds);
        set_alarm_hardware(ds.state);

        // 绘制检测结果
        for (int i = 0; i < pose_results.count; i++) {
            object_detect_result *det = &pose_results.results[i];
            draw_rectangle(&src_image, det->box.left, det->box.top,
                           det->box.right - det->box.left, det->box.bottom - det->box.top,
                           COLOR_BLUE, 2);
        }
        for (int i = 0; i < face_results.count; i++) {
            retinaface_object_t *obj = &face_results.object[i];
            draw_rectangle(&src_image, obj->box.left, obj->box.top,
                           obj->box.right - obj->box.left, obj->box.bottom - obj->box.top,
                           COLOR_GREEN, 2);
        }

        // 绘制文字
        char buf_str[64];
        draw_text(&src_image, "=== Driver Monitor ===", 10, 20, COLOR_WHITE, 18);
        snprintf(buf_str, sizeof(buf_str), "State: %s",
                 ds.state == STATE_NORMAL ? "NORMAL" : (ds.state == STATE_WARNING ? "WARNING" : "ALARM"));
        draw_text(&src_image, buf_str, 10, 50, 
                  ds.state == STATE_ALARM ? COLOR_RED : (ds.state == STATE_WARNING ? COLOR_YELLOW : COLOR_GREEN), 16);
        snprintf(buf_str, sizeof(buf_str), "EAR: %.2f  MAR: %.2f", ds.ear, ds.mar);
        draw_text(&src_image, buf_str, 10, 80, COLOR_WHITE, 14);
        snprintf(buf_str, sizeof(buf_str), "Pitch: %.1f  Yaw: %.1f", ds.pitch, ds.yaw);
        draw_text(&src_image, buf_str, 10, 110, COLOR_WHITE, 14);
        snprintf(buf_str, sizeof(buf_str), "Fatigue Score: %d", ds.fatigue_score);
        draw_text(&src_image, buf_str, 10, 140, COLOR_WHITE, 14);

        // 缩放显示
        resize_rgb_nearest(rgb_buf, CAM_WIDTH, CAM_HEIGHT, scaled_buf, SCREEN_W, SCREEN_H);
        for (int y = 0; y < SCREEN_H; y++) {
            unsigned int *line = (unsigned int*)(fb_mmap + y * finfo.line_length);
            for (int x = 0; x < SCREEN_W; x++) {
                int idx = (y * SCREEN_W + x) * 3;
                line[x] = (0xFF << 24) | (scaled_buf[idx] << 16) | (scaled_buf[idx+1] << 8) | scaled_buf[idx+2];
            }
        }

        frame_count++;
        if (frame_count % 100 == 0)
            printf("Frame %d, Score=%d, State=%d\n", frame_count, ds.fatigue_score, ds.state);
        usleep(50000);
    }

    // ========== 清理资源 ==========
    free(rgb_buf);
    free(scaled_buf);
    if (retina_input.virt_addr) free(retina_input.virt_addr);
    munmap(fb_mmap, finfo.smem_len);
close_fb:
    close(fb_fd);
out_face:
    release_retinaface_model(&face_ctx);
out_pose:
    release_yolov8_pose_model(&pose_ctx);
out:
    deinit_post_process();
    shmdt(shared_frame);
    return 0;
}