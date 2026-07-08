// #include <stdint.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <math.h>

// #include "yolov8-pose.h"
// #include "retinaface.h"      // 新增
// #include "image_utils.h"
// #include "file_utils.h"
// #include "image_drawing.h"

// // 骨架连线（保持不变）
// int skeleton[38] ={16,14,14,12,17,15,15,13,12,13,6,12,7,13,6,7,6,8,7,9,8,10,9,11,2,3,1,2,1,3,2,4,3,5,4,6,5,7};

// // 疲劳检测参数
// #define EYE_CLOSE_THRESHOLD   30.0f
// #define ALARM_FRAME_COUNT     1
// static int eye_close_counter = 0;

// static float distance(float x1, float y1, float x2, float y2) {
//     float dx = x1 - x2;
//     float dy = y1 - y2;
//     return sqrtf(dx*dx + dy*dy);
// }

// // 基于姿态的疲劳检测（原有）
// static void check_drowsiness_by_pose(object_detect_result *det_result) {
//     float left_eye_x = det_result->keypoints[1][0];
//     float left_eye_y = det_result->keypoints[1][1];
//     float right_eye_x = det_result->keypoints[2][0];
//     float right_eye_y = det_result->keypoints[2][1];
    
//     if (left_eye_x == 0 && left_eye_y == 0 && right_eye_x == 0 && right_eye_y == 0) return;
    
//     float eye_dist = distance(left_eye_x, left_eye_y, right_eye_x, right_eye_y);
//     printf("Eye distance: %.2f\n", eye_dist);
    
//     if (eye_dist < EYE_CLOSE_THRESHOLD) {
//         eye_close_counter++;
//         if (eye_close_counter >= ALARM_FRAME_COUNT) {
//             printf("⚠️⚠️⚠️  FATIGUE ALERT (pose): Eyes closed!  ⚠️⚠️⚠️\n");
//             eye_close_counter = 0;
//         }
//     } else {
//         eye_close_counter = 0;
//     }
// }

// // 基于人脸检测的疲劳辅助（新增）
// static void check_drowsiness_by_face(retinaface_result *face_result) {
//     if (face_result->count == 0) {
//         printf("⚠️ No face detected, driver may be out of position.\n");
//         return;
//     }
//     // 这里可以加入更细致的分析：比如人脸大小、姿态等
//     for (int i = 0; i < face_result->count; i++) {
//         printf("Face %d: score=%.2f, box=[%d,%d,%d,%d]\n", i,
//                face_result->object[i].score,
//                face_result->object[i].box.left,
//                face_result->object[i].box.top,
//                face_result->object[i].box.right,
//                face_result->object[i].box.bottom);
//     }
// }

// int main(int argc, char **argv)
// {
//     if (argc != 4)  // 改为4个参数：程序名 姿态模型 人脸模型 图片
//     {
//         printf("%s <pose_model_path> <face_model_path> <image_path>\n", argv[0]);
//         return -1;
//     }

//     const char *pose_model_path = argv[1];
//     const char *face_model_path = argv[2];
//     const char *image_path = argv[3];

//     int ret;
    
//     // 1. 初始化姿态模型上下文
//     rknn_app_context_t pose_ctx;
//     memset(&pose_ctx, 0, sizeof(rknn_app_context_t));
//     init_post_process();
//     ret = init_yolov8_pose_model(pose_model_path, &pose_ctx);
//     if (ret != 0) {
//         printf("init_yolov8_pose_model fail! ret=%d\n", ret);
//         goto out;
//     }
    
//     // 2. 初始化人脸检测模型上下文（注意：需要另一个独立的上下文）
//     rknn_app_context_t face_ctx;
//     memset(&face_ctx, 0, sizeof(rknn_app_context_t));
//     ret = init_retinaface_model(face_model_path, &face_ctx);
//     if (ret != 0) {
//         printf("init_retinaface_model fail! ret=%d\n", ret);
//         goto out_pose;
//     }

//     // 3. 读取图像
//     image_buffer_t src_image;
//     memset(&src_image, 0, sizeof(image_buffer_t));
//     ret = read_image(image_path, &src_image);
//     if (ret != 0) {
//         printf("read image fail! ret=%d\n", ret);
//         goto out_face;
//     }

//     // 4. 运行姿态检测
//     object_detect_result_list pose_results;
//     ret = inference_yolov8_pose_model(&pose_ctx, &src_image, &pose_results);
//     if (ret != 0) {
//         printf("inference_yolov8_pose_model fail! ret=%d\n", ret);
//         goto out_image;
//     }
    
//     // 5. 运行人脸检测
//     retinaface_result face_results;
//     memset(&face_results, 0, sizeof(retinaface_result));
//     ret = inference_retinaface_model(&face_ctx, &src_image, &face_results);
//     if (ret != 0) {
//         printf("inference_retinaface_model fail! ret=%d\n", ret);
//         // 继续，人脸检测失败不影响姿态检测
//     }

//     // 6. 绘制姿态结果（关键点、骨架、框）
//     char text[256];
//     for (int i = 0; i < pose_results.count; i++) {
//         object_detect_result *det = &pose_results.results[i];
//         printf("Pose: %s @ (%d,%d,%d,%d) %.3f\n", coco_cls_to_name(det->cls_id),
//                det->box.left, det->box.top, det->box.right, det->box.bottom, det->prop);
//         draw_rectangle(&src_image, det->box.left, det->box.top,
//                        det->box.right - det->box.left, det->box.bottom - det->box.top,
//                        COLOR_BLUE, 3);
//         sprintf(text, "%s %.1f%%", coco_cls_to_name(det->cls_id), det->prop * 100);
//         draw_text(&src_image, text, det->box.left, det->box.top - 20, COLOR_RED, 10);
        
//         // 骨架连线
//         for (int j = 0; j < 38/2; ++j) {
//             draw_line(&src_image,
//                 (int)(det->keypoints[skeleton[2*j]-1][0]),
//                 (int)(det->keypoints[skeleton[2*j]-1][1]),
//                 (int)(det->keypoints[skeleton[2*j+1]-1][0]),
//                 (int)(det->keypoints[skeleton[2*j+1]-1][1]),
//                 COLOR_ORANGE, 3);
//         }
//         // 关键点
//         for (int j = 0; j < 17; ++j) {
//             draw_circle(&src_image, (int)(det->keypoints[j][0]), (int)(det->keypoints[j][1]), 1, COLOR_YELLOW, 1);
//         }
        
//         // 姿态疲劳检测
//         check_drowsiness_by_pose(det);
//     }
    
//     // 7. 绘制人脸检测结果（矩形框）
//     for (int i = 0; i < face_results.count; i++) {
//         retinaface_object_t *obj = &face_results.object[i];
//         draw_rectangle(&src_image, obj->box.left, obj->box.top,
//                        obj->box.right - obj->box.left, obj->box.bottom - obj->box.top,
//                        COLOR_GREEN, 2);
//         sprintf(text, "Face %.0f%%", obj->score * 100);
//         draw_text(&src_image, text, obj->box.left, obj->box.top - 10, COLOR_GREEN, 8);
//     }
//     check_drowsiness_by_face(&face_results);
    
//     // 8. 保存输出图像
//     write_image("out_fusion.png", &src_image);
//     printf("Result saved to out_fusion.png\n");

// out_image:
//     if (src_image.virt_addr) free(src_image.virt_addr);
// out_face:
//     release_retinaface_model(&face_ctx);
// out_pose:
//     release_yolov8_pose_model(&pose_ctx);
// out:
//     deinit_post_process();
//     return 0;
// }


// #include <stdint.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <math.h>

// #include "yolov8-pose.h"
// #include "retinaface.h"
// #include "image_utils.h"
// #include "file_utils.h"
// #include "image_drawing.h"

// // 骨架连线
// int skeleton[38] = {16,14,14,12,17,15,15,13,12,13,6,12,7,13,6,7,6,8,7,9,8,10,9,11,2,3,1,2,1,3,2,4,3,5,4,6,5,7};

// // ==========================
// // 疲劳检测参数配置
// // ==========================
// #define EYE_CONF_THRESHOLD      0.25f   // 眼睛关键点置信度低于此值视为闭眼
// #define HEAD_DOWN_ANGLE         25.0f   // 低头角度阈值（度）
// #define HEAD_DOWN_DY_THRESH     15      // 鼻子在脖子下方的像素容差
// #define WINDOW_SIZE             15      // 滑动窗口长度（帧数，假设30fps约0.5秒）
// #define ALARM_RATIO             0.6f    // 窗口内异常帧比例超过此值触发报警

// static int eye_close_history[WINDOW_SIZE];
// static int head_down_history[WINDOW_SIZE];
// static int hist_idx = 0;

// // ==========================
// // 辅助函数：两点欧氏距离
// // ==========================
// static float distance(float x1, float y1, float x2, float y2) {
//     float dx = x1 - x2;
//     float dy = y1 - y2;
//     return sqrtf(dx*dx + dy*dy);
// }

// // ==========================
// // 计算低头角度（基于鼻子和脖子中点）
// // 返回值：角度（度），正值表示低头，负值表示仰头
// // ==========================
// static float calculate_head_pitch(float nose_x, float nose_y,
//                                   float left_shoulder_x, float left_shoulder_y,
//                                   float right_shoulder_x, float right_shoulder_y) {
//     float neck_x = (left_shoulder_x + right_shoulder_x) / 2.0f;
//     float neck_y = (left_shoulder_y + right_shoulder_y) / 2.0f;

//     float dx = nose_x - neck_x;
//     float dy = nose_y - neck_y;   // 图像坐标系 Y 轴向下为正

//     if (fabs(dy) < 1e-6) return 0.0f;

//     // 计算向量与垂直向下方向的夹角（弧度）
//     float angle_rad = atan2(fabs(dx), fabs(dy));
//     float angle_deg = angle_rad * 180.0f / M_PI;

//     // 如果鼻子在脖子下方（dy > 0）为正角度（低头），否则为负（仰头）
//     return (dy > 0) ? angle_deg : -angle_deg;
// }

// // ==========================
// // 更新滑动窗口并判断是否报警
// // ==========================
// static void update_fatigue_state(int is_eye_closed, int is_head_down) {
//     eye_close_history[hist_idx] = is_eye_closed;
//     head_down_history[hist_idx] = is_head_down;
//     hist_idx = (hist_idx + 1) % WINDOW_SIZE;

//     int eye_sum = 0, head_sum = 0;
//     for (int i = 0; i < WINDOW_SIZE; i++) {
//         eye_sum += eye_close_history[i];
//         head_sum += head_down_history[i];
//     }

//     if (eye_sum > WINDOW_SIZE * ALARM_RATIO || head_sum > WINDOW_SIZE * ALARM_RATIO) {
//         printf("\n🚨🚨🚨  FATIGUE ALERT!  🚨🚨🚨\n");
//         // 这里可以添加硬件报警：GPIO、声音、保存图片等
//     }
// }

// // ==========================
// // 基于姿态的疲劳检测（改进版）
// // ==========================
// static void check_drowsiness_by_pose(object_detect_result *det_result) {
//     if (det_result == NULL) return;

//     // 获取关键点坐标及置信度
//     float nose_x   = det_result->keypoints[0][0];
//     float nose_y   = det_result->keypoints[0][1];
//     float nose_conf= det_result->keypoints[0][2];

//     float left_eye_x   = det_result->keypoints[1][0];
//     float left_eye_y   = det_result->keypoints[1][1];
//     float left_eye_conf= det_result->keypoints[1][2];

//     float right_eye_x   = det_result->keypoints[2][0];
//     float right_eye_y   = det_result->keypoints[2][1];
//     float right_eye_conf= det_result->keypoints[2][2];

//     float left_shoulder_x  = det_result->keypoints[5][0];
//     float left_shoulder_y  = det_result->keypoints[5][1];
//     float right_shoulder_x = det_result->keypoints[6][0];
//     float right_shoulder_y = det_result->keypoints[6][1];

//     // 有效性检查
//     if (nose_conf < 0.1f) return;

//     // 1. 闭眼检测：鼻子可见但眼睛置信度低
//     int is_eye_closed = 0;
//     if (nose_conf > 0.6f) {
//         if (left_eye_conf < EYE_CONF_THRESHOLD && right_eye_conf < EYE_CONF_THRESHOLD) {
//             is_eye_closed = 1;
//         }
//     }

//     // 2. 低头检测：基于鼻子和脖子中点
//     int is_head_down = 0;
//     float pitch = calculate_head_pitch(nose_x, nose_y,
//                                        left_shoulder_x, left_shoulder_y,
//                                        right_shoulder_x, right_shoulder_y);
//     float dy = nose_y - (left_shoulder_y + right_shoulder_y)/2.0f;
//     if (dy > HEAD_DOWN_DY_THRESH && pitch > HEAD_DOWN_ANGLE) {
//         is_head_down = 1;
//     }

//     // 打印调试信息
//     printf("Pose: conf=(nose=%.2f, L_eye=%.2f, R_eye=%.2f)  pitch=%.1f°  eye_closed=%d  head_down=%d\n",
//            nose_conf, left_eye_conf, right_eye_conf, pitch, is_eye_closed, is_head_down);

//     // 更新时序统计
//     update_fatigue_state(is_eye_closed, is_head_down);
// }

// // ==========================
// // 基于人脸检测的辅助信息（可选）
// // ==========================
// static void check_drowsiness_by_face(retinaface_result *face_result) {
//     if (face_result->count == 0) {
//         printf("⚠️ No face detected (RetinaFace).\n");
//         return;
//     }
//     for (int i = 0; i < face_result->count; i++) {
//         printf("Face %d: score=%.2f, box=[%d,%d,%d,%d]\n", i,
//                face_result->object[i].score,
//                face_result->object[i].box.left,
//                face_result->object[i].box.top,
//                face_result->object[i].box.right,
//                face_result->object[i].box.bottom);
//     }
// }

// // ==========================
// // 主函数
// // ==========================
// int main(int argc, char **argv) {
//     if (argc != 4) {
//         printf("Usage: %s <pose_model_path> <face_model_path> <image_path>\n", argv[0]);
//         return -1;
//     }

//     const char *pose_model_path = argv[1];
//     const char *face_model_path = argv[2];
//     const char *image_path = argv[3];

//     int ret;
//     // 提前声明变量，避免 goto 跨越初始化
//     int retina_w = 320;
//     int retina_h = 320;
//     float scale_x = 1.0f;
//     float scale_y = 1.0f;
//     image_buffer_t retina_image;
//     memset(&retina_image, 0, sizeof(image_buffer_t));

//     // 1. 初始化姿态模型
//     rknn_app_context_t pose_ctx;
//     memset(&pose_ctx, 0, sizeof(rknn_app_context_t));
//     init_post_process();
//     ret = init_yolov8_pose_model(pose_model_path, &pose_ctx);
//     if (ret != 0) {
//         printf("init_yolov8_pose_model fail! ret=%d\n", ret);
//         goto out;
//     }

//     // 2. 初始化人脸检测模型
//     rknn_app_context_t face_ctx;
//     memset(&face_ctx, 0, sizeof(rknn_app_context_t));
//     ret = init_retinaface_model(face_model_path, &face_ctx);
//     if (ret != 0) {
//         printf("init_retinaface_model fail! ret=%d\n", ret);
//         goto out_pose;
//     }

//     // 3. 读取原始图像
//     image_buffer_t src_image;
//     memset(&src_image, 0, sizeof(image_buffer_t));
//     ret = read_image(image_path, &src_image);
//     if (ret != 0) {
//         printf("read_image fail! ret=%d\n", ret);
//         goto out_face;
//     }

//     // 4. 姿态检测（在原图上）
//     object_detect_result_list pose_results;
//     memset(&pose_results, 0, sizeof(object_detect_result_list));
//     ret = inference_yolov8_pose_model(&pose_ctx, &src_image, &pose_results);
//     if (ret != 0) {
//         printf("inference_yolov8_pose_model fail! ret=%d\n", ret);
//         goto out_image;
//     }

//     // 5. 人脸检测（暂时直接使用原图，不缩放；实际应缩放到 320x320，这里先跳过缩放避免编译错误）
//     // 注意：直接使用原图可能导致 RetinaFace 推理失败（输入尺寸不匹配），但不会报编译错误。
//     // 后续你可以自己实现 resize_image 或用 RGA 缩放。
//     retinaface_result face_results;
//     memset(&face_results, 0, sizeof(retinaface_result));
//     ret = inference_retinaface_model(&face_ctx, &src_image, &face_results);
//     if (ret != 0) {
//         printf("inference_retinaface_model fail! ret=%d\n", ret);
//         // 继续，人脸检测失败不影响姿态检测
//     }

//     // 6. 绘制姿态结果
//     char text[256];
//     for (int i = 0; i < pose_results.count; i++) {
//         object_detect_result *det = &pose_results.results[i];
//         printf("Pose: %s @ (%d,%d,%d,%d) %.3f\n", coco_cls_to_name(det->cls_id),
//                det->box.left, det->box.top, det->box.right, det->box.bottom, det->prop);

//         draw_rectangle(&src_image, det->box.left, det->box.top,
//                        det->box.right - det->box.left, det->box.bottom - det->box.top,
//                        COLOR_BLUE, 3);
//         sprintf(text, "%s %.1f%%", coco_cls_to_name(det->cls_id), det->prop * 100);
//         draw_text(&src_image, text, det->box.left, det->box.top - 20, COLOR_RED, 10);

//         // 骨架连线
//         for (int j = 0; j < 38/2; ++j) {
//             draw_line(&src_image,
//                 (int)(det->keypoints[skeleton[2*j]-1][0]),
//                 (int)(det->keypoints[skeleton[2*j]-1][1]),
//                 (int)(det->keypoints[skeleton[2*j+1]-1][0]),
//                 (int)(det->keypoints[skeleton[2*j+1]-1][1]),
//                 COLOR_ORANGE, 3);
//         }
//         // 关键点
//         for (int j = 0; j < 17; ++j) {
//             draw_circle(&src_image, (int)(det->keypoints[j][0]), (int)(det->keypoints[j][1]), 1, COLOR_YELLOW, 1);
//         }

//         // 疲劳检测（改进版）
//         check_drowsiness_by_pose(det);
//     }

//     // 7. 绘制人脸检测结果
//     for (int i = 0; i < face_results.count; i++) {
//         retinaface_object_t *obj = &face_results.object[i];
//         draw_rectangle(&src_image, obj->box.left, obj->box.top,
//                        obj->box.right - obj->box.left, obj->box.bottom - obj->box.top,
//                        COLOR_GREEN, 2);
//         sprintf(text, "Face %.0f%%", obj->score * 100);
//         draw_text(&src_image, text, obj->box.left, obj->box.top - 10, COLOR_GREEN, 8);
//     }
//     check_drowsiness_by_face(&face_results);

//     // 8. 保存输出图像
//     write_image("out_fusion.png", &src_image);
//     printf("Result saved to out_fusion.png\n");

// out_image:
//     if (src_image.virt_addr) free(src_image.virt_addr);
// out_face:
//     release_retinaface_model(&face_ctx);
// out_pose:
//     release_yolov8_pose_model(&pose_ctx);
// out:
//     deinit_post_process();
//     return 0;
// }


// #include <stdint.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <math.h>

// #include "yolov8-pose.h"
// #include "retinaface.h"
// #include "image_utils.h"
// #include "file_utils.h"
// #include "image_drawing.h"

// int skeleton[38] = {16,14,14,12,17,15,15,13,12,13,6,12,7,13,6,7,6,8,7,9,8,10,9,11,2,3,1,2,1,3,2,4,3,5,4,6,5,7};

// // 演示参数（高灵敏度）
// #define BODY_SLUMP_Y_RATIO      0.65f       // 身体瘫软阈值
// #define HEAD_DOWN_THRESH_FACE   30.0f       // 人脸存在时的低头阈值（像素）
// #define HEAD_DOWN_THRESH_LOST   35.0f       // 人脸丢失时的低头阈值（像素）

// // 报警标志位
// #define ALERT_EYE_CLOSED         0x01
// #define ALERT_HEAD_DOWN          0x02
// #define ALERT_FACE_LOST          0x04
// #define ALERT_BODY_SLUMP         0x08

// static float distance(float x1, float y1, float x2, float y2) {
//     float dx = x1 - x2;
//     float dy = y1 - y2;
//     return sqrtf(dx*dx + dy*dy);
// }

// static void check_drowsiness_fusion(object_detect_result *det_result, retinaface_result *face_result, int img_height, int img_width) {
//     if (det_result == NULL) return;

//     // ----- 1. 获取 YOLO 关键点（身体+头部辅助） -----
//     float left_shoulder_y = det_result->keypoints[5][1];
//     float right_shoulder_y = det_result->keypoints[6][1];
//     float nose_x = det_result->keypoints[0][0];
//     float nose_y = det_result->keypoints[0][1];
//     float nose_conf = det_result->keypoints[0][2];
//     float left_eye_y = det_result->keypoints[1][1];
//     float right_eye_y = det_result->keypoints[2][1];
//     float left_eye_conf = det_result->keypoints[1][2];
//     float right_eye_conf = det_result->keypoints[2][2];

//     // 计算 YOLO 俯仰角（低头为正）
//     float eyes_center_y = (left_eye_y + right_eye_y) * 0.5f;
//     float pitch_pixel = nose_y - eyes_center_y;

//     // ----- 2. 获取 RetinaFace 结果 -----
//     int face_detected = (face_result && face_result->count > 0);

//     // ----- 3. 身体瘫软检测（YOLO）-----
//     float avg_shoulder_y = (left_shoulder_y + right_shoulder_y) * 0.5f;
//     int is_body_slump = (avg_shoulder_y > img_height * BODY_SLUMP_Y_RATIO);

//     // ----- 4. 闭眼检测（融合策略）-----
//     int is_eye_closed = 0;
//     if (face_detected && nose_conf > 0.5f) {  // 正脸且鼻子清晰
//         // 当 YOLO 眼睛置信度低于 0.45 时认为闭眼（侧脸时鼻子置信度会低，已被排除）
//         if (left_eye_conf < 0.45f && right_eye_conf < 0.45f) {
//             is_eye_closed = 1;
//         }
//     }

//     // ----- 5. 低头/分神检测 -----
//     int is_head_down = 0;
//     int is_face_lost = 0;
//     if (face_detected) {
//         // 人脸存在时，低头阈值 25
//         if (pitch_pixel > HEAD_DOWN_THRESH_FACE) {
//             is_head_down = 1;
//         }
//     } else {
//         // 人脸丢失时，只有低头（正 pitch）且超过阈值才报警，避免仰拍误报
//         if (pitch_pixel > HEAD_DOWN_THRESH_LOST) {
//             is_face_lost = 1;
//         }
//     }

//     // ----- 6. 融合报警 -----
//     int alert_reason = 0;
//     char alert_msg[128] = {0};

//     if (is_eye_closed) {
//         alert_reason |= ALERT_EYE_CLOSED;
//         strcat(alert_msg, "闭眼 ");
//     }
//     if (is_body_slump) {
//         alert_reason |= ALERT_BODY_SLUMP;
//         strcat(alert_msg, "身体瘫软 ");
//     }
//     if (is_head_down) {
//         alert_reason |= ALERT_HEAD_DOWN;
//         strcat(alert_msg, "严重低头 ");
//     }
//     if (is_face_lost) {
//         alert_reason |= ALERT_FACE_LOST;
//         strcat(alert_msg, "低头分神 ");
//     }

//     // 打印调试信息（演示时可保留）
//     printf("FaceDet=%d, Pitch=%.1f, EyeConf=(%.2f,%.2f), BodySlump=%d\n",
//            face_detected, pitch_pixel, left_eye_conf, right_eye_conf, is_body_slump);

//     if (alert_reason != 0) {
//         printf("\n==================================\n");
//         printf("🚨🚨🚨 FATIGUE ALERT! 🚨🚨🚨\n");
//         printf("原因: %s\n", alert_msg);
//         printf("==================================\n\n");
//     }
// }

// int main(int argc, char **argv) {
//     if (argc != 4) {
//         printf("Usage: %s <pose_model_path> <face_model_path> <image_path>\n", argv[0]);
//         return -1;
//     }

//     const char *pose_model_path = argv[1];
//     const char *face_model_path = argv[2];
//     const char *image_path = argv[3];

//     int ret;
//     image_buffer_t retina_input = {0};
//     float scale_x = 1.0f, scale_y = 1.0f;

//     // 初始化姿态模型
//     rknn_app_context_t pose_ctx;
//     memset(&pose_ctx, 0, sizeof(rknn_app_context_t));
//     init_post_process();
//     ret = init_yolov8_pose_model(pose_model_path, &pose_ctx);
//     if (ret != 0) {
//         printf("init_yolov8_pose_model fail! ret=%d\n", ret);
//         goto out;
//     }

//     // 初始化人脸模型
//     rknn_app_context_t face_ctx;
//     memset(&face_ctx, 0, sizeof(rknn_app_context_t));
//     ret = init_retinaface_model(face_model_path, &face_ctx);
//     if (ret != 0) {
//         printf("init_retinaface_model fail! ret=%d\n", ret);
//         goto out_pose;
//     }

//     // 读取图片
//     image_buffer_t src_image;
//     memset(&src_image, 0, sizeof(image_buffer_t));
//     ret = read_image(image_path, &src_image);
//     if (ret != 0) {
//         printf("read_image fail! ret=%d, path=%s\n", ret, image_path);
//         goto out_face;
//     }

//     // 姿态推理
//     object_detect_result_list pose_results;
//     memset(&pose_results, 0, sizeof(object_detect_result_list));
//     ret = inference_yolov8_pose_model(&pose_ctx, &src_image, &pose_results);
//     if (ret != 0) {
//         printf("inference_yolov8_pose_model fail! ret=%d\n", ret);
//         goto out_image;
//     }

//     // 人脸检测：缩放到 320x320
//     ret = resize_image_cpu(&src_image, &retina_input, 320, 320);
//     if (ret != 0) {
//         printf("resize_image_cpu fail! ret=%d\n", ret);
//         goto out_retina;
//     }

//     retinaface_result face_results;
//     memset(&face_results, 0, sizeof(retinaface_result));
//     ret = inference_retinaface_model(&face_ctx, &retina_input, &face_results);
//     if (ret != 0) {
//         printf("inference_retinaface_model fail! ret=%d\n", ret);
//         // 继续执行
//     }

//     // 坐标映射回原图
//     scale_x = (float)src_image.width / 320.0f;
//     scale_y = (float)src_image.height / 320.0f;
//     for (int i = 0; i < face_results.count; i++) {
//         retinaface_object_t *obj = &face_results.object[i];
//         obj->box.left   = (int)(obj->box.left * scale_x);
//         obj->box.right  = (int)(obj->box.right * scale_x);
//         obj->box.top    = (int)(obj->box.top * scale_y);
//         obj->box.bottom = (int)(obj->box.bottom * scale_y);
//         for (int j = 0; j < 5; j++) {
//             obj->point[j].x = (int)(obj->point[j].x * scale_x);
//             obj->point[j].y = (int)(obj->point[j].y * scale_y);
//         }
//     }

//     // 绘制结果
//     char text[256];
//     for (int i = 0; i < pose_results.count; i++) {
//         object_detect_result *det = &pose_results.results[i];
//         printf("Pose: %s @ (%d,%d,%d,%d) %.3f\n", coco_cls_to_name(det->cls_id),
//                det->box.left, det->box.top, det->box.right, det->box.bottom, det->prop);

//         draw_rectangle(&src_image, det->box.left, det->box.top,
//                        det->box.right - det->box.left, det->box.bottom - det->box.top,
//                        COLOR_BLUE, 3);
//         sprintf(text, "%s %.1f%%", coco_cls_to_name(det->cls_id), det->prop * 100);
//         draw_text(&src_image, text, det->box.left, det->box.top - 20, COLOR_RED, 10);

//         for (int j = 0; j < 38/2; ++j) {
//             draw_line(&src_image,
//                 (int)(det->keypoints[skeleton[2*j]-1][0]),
//                 (int)(det->keypoints[skeleton[2*j]-1][1]),
//                 (int)(det->keypoints[skeleton[2*j+1]-1][0]),
//                 (int)(det->keypoints[skeleton[2*j+1]-1][1]),
//                 COLOR_ORANGE, 3);
//         }
//         for (int j = 0; j < 17; ++j) {
//             draw_circle(&src_image, (int)(det->keypoints[j][0]), (int)(det->keypoints[j][1]), 2, COLOR_YELLOW, 2);
//         }

//         check_drowsiness_fusion(det, &face_results, src_image.height, src_image.width);
//     }

//     // 绘制人脸框（绿色）
//     for (int i = 0; i < face_results.count; i++) {
//         retinaface_object_t *obj = &face_results.object[i];
//         draw_rectangle(&src_image, obj->box.left, obj->box.top,
//                        obj->box.right - obj->box.left, obj->box.bottom - obj->box.top,
//                        COLOR_GREEN, 2);
//         sprintf(text, "Face %.0f%%", obj->score * 100);
//         draw_text(&src_image, text, obj->box.left, obj->box.top - 10, COLOR_GREEN, 8);
//     }

//     write_image("out_fusion.png", &src_image);
//     printf("\nResult saved to out_fusion.png\n");

// out_retina:
//     if (retina_input.virt_addr) free(retina_input.virt_addr);
// out_image:
//     if (src_image.virt_addr) free(src_image.virt_addr);
// out_face:
//     release_retinaface_model(&face_ctx);
// out_pose:
//     release_yolov8_pose_model(&pose_ctx);
// out:
//     deinit_post_process();
//     return 0;
// }

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