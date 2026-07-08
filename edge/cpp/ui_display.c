// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>
// #include <fcntl.h>
// #include <sys/mman.h>
// #include <sys/ioctl.h>
// #include <linux/fb.h>
// #include <signal.h>
// #include <sys/shm.h>
// #include <time.h>
// #include "image_utils.h"
// #include "image_drawing.h"
// #include "shared_mem.h"

// #define SCREEN_W 1024
// #define SCREEN_H 600

// int skeleton[38] = {16,14,14,12,17,15,15,13,12,13,6,12,7,13,6,7,6,8,7,9,8,10,9,11,2,3,1,2,1,3,2,4,3,5,4,6,5,7};

// static volatile int running = 1;
// void sigint_handler(int sig) { running = 0; }

// static void resize_rgb_bilinear(const unsigned char *src, int src_w, int src_h,
//                                 unsigned char *dst, int dst_w, int dst_h) {
//     float x_ratio = (float)(src_w - 1) / dst_w;
//     float y_ratio = (float)(src_h - 1) / dst_h;
//     for (int y = 0; y < dst_h; y++) {
//         float src_y = y * y_ratio;
//         int y0 = (int)src_y;
//         int y1 = (y0 + 1 < src_h) ? y0 + 1 : y0;
//         float dy = src_y - y0;
//         for (int x = 0; x < dst_w; x++) {
//             float src_x = x * x_ratio;
//             int x0 = (int)src_x;
//             int x1 = (x0 + 1 < src_w) ? x0 + 1 : x0;
//             float dx = src_x - x0;
//             for (int c = 0; c < 3; c++) {
//                 float v00 = src[(y0 * src_w + x0) * 3 + c];
//                 float v01 = src[(y0 * src_w + x1) * 3 + c];
//                 float v10 = src[(y1 * src_w + x0) * 3 + c];
//                 float v11 = src[(y1 * src_w + x1) * 3 + c];
//                 float v0 = v00 * (1-dx) + v01 * dx;
//                 float v1 = v10 * (1-dx) + v11 * dx;
//                 float v = v0 * (1-dy) + v1 * dy;
//                 dst[(y * dst_w + x) * 3 + c] = (v < 0) ? 0 : (v > 255 ? 255 : (unsigned char)v);
//             }
//         }
//     }
// }

// int main() {
//     signal(SIGINT, sigint_handler);

//     // 分配缓冲区
//     unsigned char *rgb_buf = (unsigned char*)malloc(FRAME_SIZE);
//     if (!rgb_buf) { perror("malloc rgb_buf"); return -1; }

//     // 连接摄像头共享内存
//     int shm_id = shmget(SHM_KEY, sizeof(SharedFrame), IPC_CREAT | 0666);
//     if (shm_id < 0) { perror("shmget camera"); return -1; }
//     SharedFrame *frame = (SharedFrame*)shmat(shm_id, NULL, 0);
//     if (frame == (void*)-1) { perror("shmat camera"); return -1; }

//     // 连接结果共享内存
//     int res_shm = shmget(RESULT_SHM_KEY, sizeof(BehaviorResult), IPC_CREAT | 0666);
//     BehaviorResult *result = NULL;
//     if (res_shm >= 0) {
//         result = (BehaviorResult*)shmat(res_shm, NULL, 0);
//         if (result == (void*)-1) result = NULL;
//     }

//     // Framebuffer
//     int fb_fd = open("/dev/fb0", O_RDWR);
//     if (fb_fd < 0) { perror("open fb"); return -1; }
//     struct fb_var_screeninfo vinfo;
//     struct fb_fix_screeninfo finfo;
//     if (ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo) < 0) { perror("fb var"); return -1; }
//     if (ioctl(fb_fd, FBIOGET_FSCREENINFO, &finfo) < 0) { perror("fb fix"); return -1; }
//     if (vinfo.bits_per_pixel != 32) {
//         printf("Only 32bpp supported\n");
//         return -1;
//     }
//     unsigned char *fb_mmap = (unsigned char*)mmap(0, finfo.smem_len, PROT_READ|PROT_WRITE, MAP_SHARED, fb_fd, 0);
//     if (fb_mmap == MAP_FAILED) { perror("fb mmap"); return -1; }

//     unsigned char *scaled_buf = (unsigned char*)malloc(SCREEN_W * SCREEN_H * 3);
//     if (!scaled_buf) { perror("malloc scaled_buf"); return -1; }

//     int last_frame = -1;
//     uint64_t last_heartbeat_time = 0;

//     while (running) {
//         if (frame->frame_count == last_frame) { usleep(10000); continue; }
//         last_frame = frame->frame_count;
//         memcpy(rgb_buf, frame->rgb_data, FRAME_SIZE);

//         image_buffer_t src_image = {
//             .width = frame->width, .height = frame->height,
//             .format = IMAGE_FORMAT_RGB888, .size = FRAME_SIZE,
//             .virt_addr = rgb_buf
//         };

//         // 绘制骨架
//         if (result && result->pose_detected) {
//             for (int i = 0; i < 17; i++) {
//                 float x = result->pose_keypoints[i][0];
//                 float y = result->pose_keypoints[i][1];
//                 if (x > 0 && y > 0) draw_circle(&src_image, (int)x, (int)y, 3, COLOR_YELLOW, 2);
//             }
//             for (int i = 0; i < 38/2; i++) {
//                 int p1 = skeleton[2*i] - 1;
//                 int p2 = skeleton[2*i+1] - 1;
//                 if (p1>=0 && p1<17 && p2>=0 && p2<17) {
//                     float x1 = result->pose_keypoints[p1][0];
//                     float y1 = result->pose_keypoints[p1][1];
//                     float x2 = result->pose_keypoints[p2][0];
//                     float y2 = result->pose_keypoints[p2][1];
//                     if (x1>0 && y1>0 && x2>0 && y2>0)
//                         draw_line(&src_image, (int)x1, (int)y1, (int)x2, (int)y2, COLOR_ORANGE, 2);
//                 }
//             }
//         }

//         // 绘制人脸框（绿色，不画内部关键点）
//         if (result && result->face_detected) {
//             for (int i = 0; i < result->face_count; i++) {
//                 FaceInfo *face = &result->faces[i];
//                 draw_rectangle(&src_image,
//                 face->left, face->top,
//                 face->right - face->left,
//                 face->bottom - face->top,
//                 COLOR_GREEN, 2);

//                  // 左嘴角
//         draw_circle(&src_image, 
//                     face->points[3][0], face->points[3][1], 
//                     3, COLOR_RED, 2);
//         // 右嘴角
//         draw_circle(&src_image, 
//                     face->points[4][0], face->points[4][1], 
//                     3, COLOR_RED, 2);
//             }
//         }       
        
//         // 报警文字
//         if (result && result->frame_seq == frame->frame_count) {
//             int y = 50;
//             if (result->phone_call) draw_text(&src_image, "PHONE CALL!", 10, y, COLOR_RED, 30), y+=40;
//             if (result->smoking) draw_text(&src_image, "SMOKING!", 10, y, COLOR_RED, 30), y+=40;
//             if (result->head_down) draw_text(&src_image, "HEAD DOWN", 10, y, COLOR_YELLOW, 30);
//         }

//         // 心跳检测：检查 behavior engine 是否存活
//         if (result) {
//             uint64_t now = (uint64_t)time(NULL) * 1000000LL;
//             if (result->heartbeat + 2000000 < now) { // 2秒超时
//                 draw_text(&src_image, "ENGINE TIMEOUT", 10, 200, COLOR_RED, 20);
//             }
//         }

//         // 缩放并显示
//         resize_rgb_bilinear(src_image.virt_addr, src_image.width, src_image.height,
//                             scaled_buf, SCREEN_W, SCREEN_H);
//         for (int y = 0; y < SCREEN_H; y++) {
//             unsigned int *line = (unsigned int*)(fb_mmap + y * finfo.line_length);
//             for (int x = 0; x < SCREEN_W; x++) {
//                 int idx = (y * SCREEN_W + x) * 3;
//                 line[x] = (0xFF << 24) | (scaled_buf[idx] << 16) | (scaled_buf[idx+1] << 8) | scaled_buf[idx+2];
//             }
//         }
//     }

//     free(scaled_buf);
//     free(rgb_buf);
//     munmap(fb_mmap, finfo.smem_len);
//     close(fb_fd);
//     shmdt(frame);
//     shmctl(shm_id, IPC_RMID, NULL);
//     if (result) shmdt(result);
//     shmctl(res_shm, IPC_RMID, NULL);
//     return 0;
// }

// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>
// #include <fcntl.h>
// #include <sys/mman.h>
// #include <sys/ioctl.h>
// #include <linux/fb.h>
// #include <signal.h>
// #include <sys/shm.h>
// #include <time.h>
// #include <rga.h>          // RGA 硬件加速头文件
// #include <im2d.h>         // im2d 接口
// #include "image_utils.h"
// #include "image_drawing.h"
// #include "shared_mem.h"

// #define SCREEN_W 1024
// #define SCREEN_H 600

// int skeleton[38] = {16,14,14,12,17,15,15,13,12,13,6,12,7,13,6,7,6,8,7,9,8,10,9,11,2,3,1,2,1,3,2,4,3,5,4,6,5,7};

// static volatile int running = 1;
// void sigint_handler(int sig) { running = 0; }

// // ========== RGA 硬件缩放函数 ==========
// static void resize_rgb_rga(const unsigned char *src, int src_w, int src_h,
//                            unsigned char *dst, int dst_w, int dst_h) {
//     rga_buffer_t src_buf, dst_buf;
//     src_buf = wrapbuffer_virtualaddr((void*)src, src_w, src_h, RK_FORMAT_RGB_888);
//     dst_buf = wrapbuffer_virtualaddr((void*)dst, dst_w, dst_h, RK_FORMAT_RGB_888);
//     int ret = imresize(src_buf, dst_buf);
//     if (ret != 0) {
//         fprintf(stderr, "[RGA] imresize failed: %d, fallback to nearest\n", ret);
//         // 简单后备：最近邻缩放（避免黑屏）
//         for (int y = 0; y < dst_h; y++) {
//             int src_y = y * src_h / dst_h;
//             for (int x = 0; x < dst_w; x++) {
//                 int src_x = x * src_w / dst_w;
//                 memcpy(dst + (y * dst_w + x) * 3,
//                        src + (src_y * src_w + src_x) * 3, 3);
//             }
//         }
//     }
// }

// int main() {
//     signal(SIGINT, sigint_handler);

//     // 分配缓冲区
//     unsigned char *rgb_buf = (unsigned char*)malloc(FRAME_SIZE);
//     if (!rgb_buf) { perror("malloc rgb_buf"); return -1; }

//     // 连接摄像头共享内存
//     int shm_id = shmget(SHM_KEY, sizeof(SharedFrame), IPC_CREAT | 0666);
//     if (shm_id < 0) { perror("shmget camera"); return -1; }
//     SharedFrame *frame = (SharedFrame*)shmat(shm_id, NULL, 0);
//     if (frame == (void*)-1) { perror("shmat camera"); return -1; }

//     // 连接结果共享内存
//     int res_shm = shmget(RESULT_SHM_KEY, sizeof(BehaviorResult), IPC_CREAT | 0666);
//     BehaviorResult *result = NULL;
//     if (res_shm >= 0) {
//         result = (BehaviorResult*)shmat(res_shm, NULL, 0);
//         if (result == (void*)-1) result = NULL;
//     }

//     // Framebuffer
//     int fb_fd = open("/dev/fb0", O_RDWR);
//     if (fb_fd < 0) { perror("open fb"); return -1; }
//     struct fb_var_screeninfo vinfo;
//     struct fb_fix_screeninfo finfo;
//     if (ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo) < 0) { perror("fb var"); return -1; }
//     if (ioctl(fb_fd, FBIOGET_FSCREENINFO, &finfo) < 0) { perror("fb fix"); return -1; }
//     if (vinfo.bits_per_pixel != 32) {
//         printf("Only 32bpp supported\n");
//         return -1;
//     }
//     unsigned char *fb_mmap = (unsigned char*)mmap(0, finfo.smem_len, PROT_READ|PROT_WRITE, MAP_SHARED, fb_fd, 0);
//     if (fb_mmap == MAP_FAILED) { perror("fb mmap"); return -1; }

//     unsigned char *scaled_buf = (unsigned char*)malloc(SCREEN_W * SCREEN_H * 3);
//     if (!scaled_buf) { perror("malloc scaled_buf"); return -1; }

//     int last_frame = -1;
//     uint64_t last_heartbeat_time = 0;

//     while (running) {
//         if (frame->frame_count == last_frame) { usleep(10000); continue; }
//         last_frame = frame->frame_count;
//         memcpy(rgb_buf, frame->rgb_data, FRAME_SIZE);

//         image_buffer_t src_image = {
//             .width = frame->width,
//             .height = frame->height,
//             .format = IMAGE_FORMAT_RGB888,
//             .size = FRAME_SIZE,
//             .virt_addr = rgb_buf
//         };

//         // 绘制骨架
//         if (result && result->pose_detected) {
//             for (int i = 0; i < 17; i++) {
//                 float x = result->pose_keypoints[i][0];
//                 float y = result->pose_keypoints[i][1];
//                 if (x > 0 && y > 0) draw_circle(&src_image, (int)x, (int)y, 3, COLOR_YELLOW, 2);
//             }
//             for (int i = 0; i < 38/2; i++) {
//                 int p1 = skeleton[2*i] - 1;
//                 int p2 = skeleton[2*i+1] - 1;
//                 if (p1>=0 && p1<17 && p2>=0 && p2<17) {
//                     float x1 = result->pose_keypoints[p1][0];
//                     float y1 = result->pose_keypoints[p1][1];
//                     float x2 = result->pose_keypoints[p2][0];
//                     float y2 = result->pose_keypoints[p2][1];
//                     if (x1>0 && y1>0 && x2>0 && y2>0)
//                         draw_line(&src_image, (int)x1, (int)y1, (int)x2, (int)y2, COLOR_ORANGE, 2);
//                 }
//             }
//         }

//         // 绘制人脸框和嘴角点（绿色框 + 红色嘴角点）
//         if (result && result->face_detected) {
//             for (int i = 0; i < result->face_count; i++) {
//                 FaceInfo *face = &result->faces[i];
//                 draw_rectangle(&src_image,
//                                face->left, face->top,
//                                face->right - face->left,
//                                face->bottom - face->top,
//                                COLOR_GREEN, 2);
//                 // 左嘴角
//                 draw_circle(&src_image, face->points[3][0], face->points[3][1], 3, COLOR_RED, 2);
//                 // 右嘴角
//                 draw_circle(&src_image, face->points[4][0], face->points[4][1], 3, COLOR_RED, 2);
//             }
//         }

//         // 报警文字
//         if (result && result->frame_seq == frame->frame_count) {
//             int y = 50;
//             if (result->phone_call) draw_text(&src_image, "PHONE CALL!", 10, y, COLOR_RED, 30), y+=40;
//             if (result->smoking) draw_text(&src_image, "SMOKING!", 10, y, COLOR_RED, 30), y+=40;
//             if (result->head_down) draw_text(&src_image, "HEAD DOWN", 10, y, COLOR_YELLOW, 30);
//         }

//         // 心跳检测
//         if (result) {
//             uint64_t now = (uint64_t)time(NULL) * 1000000LL;
//             if (result->heartbeat + 2000000 < now) {
//                 draw_text(&src_image, "ENGINE TIMEOUT", 10, 200, COLOR_RED, 20);
//             }
//         }

//         // 使用 RGA 硬件缩放（取代原双线性插值）
//         resize_rgb_rga(src_image.virt_addr, src_image.width, src_image.height,
//                        scaled_buf, SCREEN_W, SCREEN_H);

//         // 输出到 framebuffer
//         for (int y = 0; y < SCREEN_H; y++) {
//             unsigned int *line = (unsigned int*)(fb_mmap + y * finfo.line_length);
//             for (int x = 0; x < SCREEN_W; x++) {
//                 int idx = (y * SCREEN_W + x) * 3;
//                 line[x] = (0xFF << 24) | (scaled_buf[idx] << 16) | (scaled_buf[idx+1] << 8) | scaled_buf[idx+2];
//             }
//         }
//     }

//     free(scaled_buf);
//     free(rgb_buf);
//     munmap(fb_mmap, finfo.smem_len);
//     close(fb_fd);
//     shmdt(frame);
//     shmctl(shm_id, IPC_RMID, NULL);
//     if (result) shmdt(result);
//     shmctl(res_shm, IPC_RMID, NULL);
//     return 0;
// }

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <signal.h>
#include <sys/shm.h>
#include <time.h>
#include <rga.h>          // RGA 硬件加速头文件
#include <im2d.h>         // im2d 接口
#include "image_utils.h"
#include "image_drawing.h"
#include "shared_mem.h"

#define SCREEN_W 1024
#define SCREEN_H 600
// 注意：FRAME_SIZE 应该在 shared_mem.h 中定义，或者这里重新定义
#ifndef FRAME_SIZE
#define CAM_WIDTH        1280
#define CAM_HEIGHT       720
#define FRAME_SIZE       (CAM_WIDTH * CAM_HEIGHT * 3)
#endif

int skeleton[38] = {16,14,14,12,17,15,15,13,12,13,6,12,7,13,6,7,6,8,7,9,8,10,9,11,2,3,1,2,1,3,2,4,3,5,4,6,5,7};

static volatile int running = 1;
void sigint_handler(int sig) { running = 0; }

// ========== RGA 硬件缩放函数 ==========
static void resize_rgb_rga(const unsigned char *src, int src_w, int src_h,
                           unsigned char *dst, int dst_w, int dst_h) {
    rga_buffer_t src_buf, dst_buf;
    
    // 封装缓冲区
    src_buf = wrapbuffer_virtualaddr((void*)src, src_w, src_h, RK_FORMAT_RGB_888);
    dst_buf = wrapbuffer_virtualaddr((void*)dst, dst_w, dst_h, RK_FORMAT_RGB_888);
    
    // 直接执行缩放，通过返回值判断结果
    int ret = imresize(src_buf, dst_buf);
    
    if (ret != IM_STATUS_SUCCESS) {
        fprintf(stderr, "[RGA] imresize failed: %d, fallback to nearest\n", ret);
        
        // 简单后备：最近邻缩放（避免黑屏）
        for (int y = 0; y < dst_h; y++) {
            int src_y = y * src_h / dst_h;
            for (int x = 0; x < dst_w; x++) {
                int src_x = x * src_w / dst_w;
                memcpy(dst + (y * dst_w + x) * 3,
                       src + (src_y * src_w + src_x) * 3, 3);
            }
        }
    }
}

int main() {
    signal(SIGINT, sigint_handler);

    unsigned char *rgb_buf = NULL;
    unsigned char *scaled_buf = NULL;

    // 【关键修改】使用 posix_memalign 分配 64-byte 对齐的内存
    if (posix_memalign((void**)&rgb_buf, 64, FRAME_SIZE) != 0) {
        perror("posix_memalign rgb_buf");
        return -1;
    }
    if (posix_memalign((void**)&scaled_buf, 64, SCREEN_W * SCREEN_H * 3) != 0) {
        perror("posix_memalign scaled_buf");
        free(rgb_buf);
        return -1;
    }
    
    printf("[INFO] Buffers aligned to 64 bytes.\n");

    // 连接摄像头共享内存
    int shm_id = shmget(SHM_KEY, sizeof(SharedFrame), IPC_CREAT | 0666);
    if (shm_id < 0) { perror("shmget camera"); goto cleanup; }
    
    SharedFrame *frame = (SharedFrame*)shmat(shm_id, NULL, 0);
    if (frame == (void*)-1) { perror("shmat camera"); goto cleanup; }

    // 连接结果共享内存
    int res_shm = shmget(RESULT_SHM_KEY, sizeof(BehaviorResult), IPC_CREAT | 0666);
    BehaviorResult *result = NULL;
    if (res_shm >= 0) {
        result = (BehaviorResult*)shmat(res_shm, NULL, 0);
        if (result == (void*)-1) result = NULL;
    }

    // 连接温湿度共享内存
    int env_shm = shmget(ENV_SHM_KEY, sizeof(EnvData), 0666);
    EnvData *env_data = NULL;
    if (env_shm >= 0) {
        env_data = (EnvData*)shmat(env_shm, NULL, 0);
        if (env_data == (void*)-1) env_data = NULL;
    }

    // Framebuffer
    int fb_fd = open("/dev/fb0", O_RDWR);
    if (fb_fd < 0) { perror("open fb"); goto cleanup; }
    
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    if (ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo) < 0) { perror("fb var"); goto cleanup; }
    if (ioctl(fb_fd, FBIOGET_FSCREENINFO, &finfo) < 0) { perror("fb fix"); goto cleanup; }
    
    if (vinfo.bits_per_pixel != 32) {
        printf("Only 32bpp supported\n");
        goto cleanup;
    }
    
    unsigned char *fb_mmap = (unsigned char*)mmap(0, finfo.smem_len, PROT_READ|PROT_WRITE, MAP_SHARED, fb_fd, 0);
    if (fb_mmap == MAP_FAILED) { perror("fb mmap"); goto cleanup; }

    int last_frame = -1;

    while (running) {
        if (frame->frame_count == last_frame) { usleep(10000); continue; }
        last_frame = frame->frame_count;
        
        // 拷贝数据到对齐的缓冲区
        memcpy(rgb_buf, frame->rgb_data, FRAME_SIZE);

        image_buffer_t src_image = {
            .width = frame->width,
            .height = frame->height,
            .format = IMAGE_FORMAT_RGB888,
            .size = FRAME_SIZE,
            .virt_addr = rgb_buf
        };

        // ---------- 温湿度显示 ----------
    if (env_data) {
        char temp_text[64];
        snprintf(temp_text, sizeof(temp_text), "Temp: %.1f°C  Hum: %.0f%%", 
                 env_data->temperature, env_data->humidity);
        draw_text(&src_image, temp_text, 10, 20, COLOR_BLUE, 20);
    }

        // --- 绘制逻辑 (骨架、人脸、文字) ---
        // ... (保持你原有的绘制代码不变) ...
        if (result && result->pose_detected) {
             for (int i = 0; i < 17; i++) {
                float x = result->pose_keypoints[i][0];
                float y = result->pose_keypoints[i][1];
                if (x > 0 && y > 0) draw_circle(&src_image, (int)x, (int)y, 3, COLOR_YELLOW, 2);
            }
            for (int i = 0; i < 38/2; i++) {
                int p1 = skeleton[2*i] - 1;
                int p2 = skeleton[2*i+1] - 1;
                if (p1>=0 && p1<17 && p2>=0 && p2<17) {
                    float x1 = result->pose_keypoints[p1][0];
                    float y1 = result->pose_keypoints[p1][1];
                    float x2 = result->pose_keypoints[p2][0];
                    float y2 = result->pose_keypoints[p2][1];
                    if (x1>0 && y1>0 && x2>0 && y2>0)
                        draw_line(&src_image, (int)x1, (int)y1, (int)x2, (int)y2, COLOR_ORANGE, 2);
                }
            }
        }

        if (result && result->face_detected) {
            for (int i = 0; i < result->face_count; i++) {
                FaceInfo *face = &result->faces[i];
                draw_rectangle(&src_image, face->left, face->top, face->right - face->left, face->bottom - face->top, COLOR_GREEN, 2);
                draw_circle(&src_image, face->points[3][0], face->points[3][1], 3, COLOR_RED, 2);
                draw_circle(&src_image, face->points[4][0], face->points[4][1], 3, COLOR_RED, 2);
            }
        }

        if (result && result->frame_seq == frame->frame_count) {
            int y = 50;
            if (result->phone_call) draw_text(&src_image, "PHONE CALL!", 10, y, COLOR_RED, 30), y+=40;
            if (result->smoking) draw_text(&src_image, "SMOKING!", 10, y, COLOR_RED, 30), y+=40;
            if (result->head_down) draw_text(&src_image, "HEAD DOWN", 10, y, COLOR_YELLOW, 30);
        }

        if (result) {
            uint64_t now = (uint64_t)time(NULL) * 1000000LL;
            if (result->heartbeat + 2000000 < now) {
                draw_text(&src_image, "ENGINE TIMEOUT", 10, 200, COLOR_RED, 20);
            }
        }
        // -----------------------------------

        // 使用 RGA 硬件缩放
        resize_rgb_rga(src_image.virt_addr, src_image.width, src_image.height,
                       scaled_buf, SCREEN_W, SCREEN_H);

        // 输出到 framebuffer
        for (int y = 0; y < SCREEN_H; y++) {
            unsigned int *line = (unsigned int*)(fb_mmap + y * finfo.line_length);
            for (int x = 0; x < SCREEN_W; x++) {
                int idx = (y * SCREEN_W + x) * 3;
                line[x] = (0xFF << 24) | (scaled_buf[idx] << 16) | (scaled_buf[idx+1] << 8) | scaled_buf[idx+2];
            }
        }
    }

cleanup:
    if (scaled_buf) free(scaled_buf);
    if (rgb_buf) free(rgb_buf);
    if (fb_mmap) munmap(fb_mmap, finfo.smem_len);
    if (fb_fd >= 0) close(fb_fd);
    if (frame) shmdt(frame);
    if (shm_id >= 0) shmctl(shm_id, IPC_RMID, NULL); // 注意：生产环境建议手动清理
    if (result) shmdt(result);
    if (res_shm >= 0) shmctl(res_shm, IPC_RMID, NULL);
    
    return 0;
}