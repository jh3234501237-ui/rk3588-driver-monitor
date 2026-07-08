// #include "ui_fatigue.h"
// #include <stdio.h>
// #include <stdbool.h>
// #include <string.h>
// #include <stdlib.h>
// #include <stdint.h>
// #include <time.h>

// //LV_FONT_DECLARE(lv_font_chinese_18);

// /* =========================================================
//  * LVGL 8.2 compatible opacity
//  * ========================================================= */
// #define OPACITY_20      ((lv_opa_t)51)
// #define OPACITY_30      ((lv_opa_t)76)
// #define OPACITY_35      ((lv_opa_t)90)
// #define OPACITY_45      ((lv_opa_t)115)
// #define OPACITY_55      ((lv_opa_t)140)
// #define OPACITY_70      ((lv_opa_t)178)

// /* =========================================================
//  * Colors
//  * ========================================================= */
// #define COLOR_BG        lv_color_hex(0x030914)
// #define COLOR_HEADER    lv_color_hex(0x050B15)
// #define COLOR_SIDEBAR   lv_color_hex(0x06101E)

// #define COLOR_PANEL     lv_color_hex(0x0A1728)
// #define COLOR_PANEL_2   lv_color_hex(0x0E2238)
// #define COLOR_PANEL_3   lv_color_hex(0x061220)

// #define COLOR_BORDER    lv_color_hex(0x0C6D9E)
// #define COLOR_BORDER_DIM lv_color_hex(0x0A314A)

// #define COLOR_TEXT      lv_color_hex(0xE8EEF8)
// #define COLOR_SUB_TEXT  lv_color_hex(0x9AA8C1)

// #define COLOR_BLUE      lv_color_hex(0x168BFF)
// #define COLOR_CYAN      lv_color_hex(0x00E5FF)
// #define COLOR_GREEN     lv_color_hex(0x20F58A)
// #define COLOR_YELLOW    lv_color_hex(0xFFC928)
// #define COLOR_ORANGE    lv_color_hex(0xFF8C00)
// #define COLOR_RED       lv_color_hex(0xFF365F)

// /* =========================================================
//  * Fonts
//  * ========================================================= */
// #define FONT_SMALL      (&lv_font_montserrat_14)
// #define FONT_NORMAL     (&lv_font_montserrat_16)
// #define FONT_MEDIUM     (&lv_font_montserrat_20)
// #define FONT_LARGE      (&lv_font_montserrat_28)
// #define FONT_NUMBER     (&lv_font_montserrat_38)

// /* =========================================================
//  * Layout
//  * ========================================================= */
// #define HEADER_H        62
// #define SIDEBAR_W       150

// #define PAGE_X          160
// #define PAGE_Y          74
// #define PAGE_W          850
// #define PAGE_H          510

// /* =========================================================
//  * Page enum
//  * ========================================================= */
// typedef enum {
//     PAGE_HOME = 0,
//     PAGE_DETECTION,
//     PAGE_ENVIRONMENT,
//     PAGE_RECORDS,
//     PAGE_SETTINGS,
//     PAGE_COUNT
// } UiPage;

// /* =========================================================
//  * Global objects
//  * ========================================================= */
// static lv_obj_t *pages[PAGE_COUNT];
// static lv_obj_t *nav_btns[PAGE_COUNT];

// static lv_obj_t *label_time;
// static lv_obj_t *label_date;

// static FanCtrl *g_fan_ctrl = NULL;
// static bool g_updating_slider = false;

// static lv_obj_t *danger_flash_layer = NULL;
// static int danger_flash_count = 0;

// static int last_auto_popup_level = 0;
// static uint32_t last_auto_popup_tick = 0;

// static int normal_behavior_count = 0;
// static int fan_boost_active = 0;
// static int ai_popup_auto_opened = 0;

// /* =========================================================
//  * Camera preview
//  * SharedFrame: 1280x720 RGB888 -> LVGL image
//  * ========================================================= */
// #define CAM_PREVIEW_W   610
// #define CAM_PREVIEW_H   343
// #define CAM_IMG_Y_OFF   9

// /* =========================================================
//  * Camera view mode
//  * 外部预览框固定，内部图像做裁剪/缩放
//  * ========================================================= */
// typedef enum {
//     CAMERA_VIEW_WIDE = 0,
//     CAMERA_VIEW_NORMAL,
//     CAMERA_VIEW_FOCUS,
//     CAMERA_VIEW_COUNT
// } CameraViewMode;

// static CameraViewMode g_camera_view_mode = CAMERA_VIEW_WIDE;

// /*
//  * Wide   = 完整画面，保持当前效果
//  * Normal = 轻微裁剪放大
//  * Focus  = 明显裁剪放大，突出脸部/上半身
//  */
// static const float camera_view_crop_ratio[CAMERA_VIEW_COUNT] = {
//     1.00f,   /* Wide */
//     0.88f,   /* Normal */
//     0.72f    /* Focus */
// };

// /*
//  * y_bias 控制裁剪区域上下位置：
//  * 0.50 = 居中
//  * 0.38 = 稍微偏上，更适合驾驶员头肩区域
//  */
// static const float camera_view_y_bias[CAMERA_VIEW_COUNT] = {
//     0.50f,   /* Wide */
//     0.42f,   /* Normal */
//     0.36f    /* Focus */
// };

// static lv_obj_t *view_mode_btns[CAMERA_VIEW_COUNT];
// static lv_obj_t *view_mode_label = NULL;

// /* 当前裁剪区域，Overlay 映射也用它 */
// static int g_crop_x = 0;
// static int g_crop_y = 0;
// static int g_crop_w = 0;
// static int g_crop_h = 0;

// /* 前置声明：ui_update_detection_overlay() 会提前调用它 */
// static void camera_compute_crop_rect(int src_w, int src_h);

// static void show_ai_copilot_popup_event_cb(lv_event_t *e);
// static void ai_result_timer_cb(lv_timer_t *timer);

// static SharedFrame *g_camera_frame = NULL;
// static BehaviorResult *g_behavior_result = NULL;

// static lv_obj_t *camera_img = NULL;
// static lv_obj_t *camera_preview_panel = NULL;
// static lv_obj_t *camera_overlay = NULL;

// static lv_obj_t *camera_placeholder_1 = NULL;
// static lv_obj_t *camera_placeholder_2 = NULL;

// static lv_img_dsc_t camera_img_dsc;
// static lv_color_t *camera_preview_buf = NULL;

// static int last_camera_frame_count = -1;



// /* Overlay 开关 */
// static bool g_overlay_enabled = true;
// static lv_obj_t *overlay_switch = NULL;
// static lv_obj_t *overlay_state_label = NULL;


// /* 最多 3 张人脸，每张脸 1 个框 + 5 个关键点 */
// #define MAX_UI_FACES 3
// #define FACE_POINTS  5

// static lv_obj_t *face_boxes[MAX_UI_FACES];
// static lv_obj_t *face_points[MAX_UI_FACES][FACE_POINTS];

// /* 姿态关键点：YOLOv8-Pose / COCO 17 points */
// #define POSE_POINTS 17
// static lv_obj_t *pose_points[POSE_POINTS];

// /* 姿态骨架线：COCO 17 points skeleton */
// #define SKELETON_LINES 16

// static lv_obj_t *pose_lines[SKELETON_LINES];

// static const int skeleton_pairs[SKELETON_LINES][2] = {
//     {0, 1},   /* nose - left eye */
//     {0, 2},   /* nose - right eye */
//     {1, 3},   /* left eye - left ear */
//     {2, 4},   /* right eye - right ear */

//     {5, 6},   /* left shoulder - right shoulder */
//     {5, 7},   /* left shoulder - left elbow */
//     {7, 9},   /* left elbow - left wrist */
//     {6, 8},   /* right shoulder - right elbow */
//     {8, 10},  /* right elbow - right wrist */

//     {5, 11},  /* left shoulder - left hip */
//     {6, 12},  /* right shoulder - right hip */
//     {11, 12}, /* left hip - right hip */

//     {11, 13}, /* left hip - left knee */
//     {13, 15}, /* left knee - left ankle */
//     {12, 14}, /* right hip - right knee */
//     {14, 16}  /* right knee - right ankle */
// };

// /* Home */
// static lv_obj_t *home_state_arc;
// static lv_obj_t *home_state_icon;
// static lv_obj_t *home_state_label;
// static lv_obj_t *home_state_desc;
// static lv_obj_t *home_score_label;
// static lv_obj_t *home_score_bar;
// static lv_obj_t *home_head_pill;
// static lv_obj_t *home_phone_pill;
// static lv_obj_t *home_smoke_pill;
// static lv_obj_t *home_fan_mode;
// static lv_obj_t *home_fan_speed;
// static lv_obj_t *home_comfort;
// static lv_obj_t *home_chip_temp;
// static lv_obj_t *home_alert_text;
// static lv_obj_t *home_wheel_pill = NULL;

// /* Detection */
// static lv_obj_t *det_eye_state;
// static lv_obj_t *det_perclos;
// static lv_obj_t *det_blink;
// static lv_obj_t *det_yawn;
// static lv_obj_t *det_head_pill;
// static lv_obj_t *det_phone_pill;
// static lv_obj_t *det_smoke_pill;
// static lv_obj_t *det_risk_text;
// static lv_obj_t *det_explain_text;
// static lv_obj_t *detect_risk_score_label = NULL;
// static lv_obj_t *detect_risk_score_text_label = NULL;
// static lv_obj_t *det_driver_state_label = NULL;
// static lv_obj_t *det_vehicle_state = NULL;

// /* Environment */
// static lv_obj_t *env_temp_label;
// static lv_obj_t *env_humi_label;
// static lv_obj_t *env_comfort_label;
// static lv_obj_t *env_fan_mode;
// static lv_obj_t *env_fan_speed;
// static lv_obj_t *env_chip_temp;
// static lv_obj_t *env_fan_arc;
// static lv_obj_t *slider_fan;

// /* Records */
// static lv_obj_t *rec_chart;
// static lv_chart_series_t *rec_score_series;


// #define MAX_EVENT_RECORDS 5
// #define EVENT_TEXT_LEN    96

// static lv_obj_t *rec_event_labels[MAX_EVENT_RECORDS];
// static char rec_event_texts[MAX_EVENT_RECORDS][EVENT_TEXT_LEN];
// static int rec_event_count = 0;

// #define REC_TIME_LABELS 5
// static lv_obj_t *rec_time_labels[REC_TIME_LABELS];

// #define EVENT_LOG_FILE "/home/elf/rknn_model_zoo/rknn_model_zoo-2.1.0/examples/yolov8_pose/cpp/logs/events.csv"

// #define HISTORY_MAX_LINES 20
// #define HISTORY_LINE_LEN  256

// static lv_obj_t *history_popup = NULL;
// static lv_obj_t *history_line_labels[HISTORY_MAX_LINES];

// /* =========================================================
//  * AI Copilot popup
//  * ========================================================= */
// static lv_obj_t *ai_popup = NULL;
// static lv_obj_t *ai_answer_box = NULL;
// static lv_obj_t *ai_answer_label = NULL;
// static lv_obj_t *ai_status_label = NULL;
// static char ai_last_answer_cache[4096] = {0};
// //static lv_obj_t *ai_policy_label = NULL;

// /* 用于防止同一个状态每次刷新都重复记录 */
// static int last_head_state = -1;
// static int last_phone_state = -1;
// static int last_smoking_state = -1;
// static int last_fatigue_level = -1;

// /* Settings */
// static lv_obj_t *setting_voice_state;
// static lv_obj_t *sw_voice;

// //背光
// static void brightness_slider_event_cb(lv_event_t *e);

// /* System Metrics */
// static lv_obj_t *metric_runtime_label;
// static lv_obj_t *metric_camera_fps_label;
// static lv_obj_t *metric_ai_fps_label;
// static lv_obj_t *metric_infer_ms_label;
// static lv_obj_t *metric_preview_fps_label;
// static lv_obj_t *metric_cpu_label;
// static lv_obj_t *metric_stability_label;

// static uint32_t metric_start_tick = 0;
// static int power_shutdown_dialog_shown = 0;
// static int camera_preview_frame_count = 0;

// /* Shutdown confirm dialog */
// static lv_obj_t *shutdown_confirm_mask = NULL;
// static lv_obj_t *shutdown_confirm_box  = NULL;

// static lv_obj_t *setting_vehicle_mode_label = NULL;
// static lv_obj_t *btn_vehicle_mode = NULL;
// static int g_vehicle_mode = 0;   /* 0=AUTO, 1=DRIVING, 2=PARKED */

// #define POWER_RESTART_HOLD_MS   1800
// #define POWER_SHUTDOWN_HOLD_MS  5500

// static uint32_t power_press_tick = 0;

// static void power_main_btn_event_cb(lv_event_t *e);
// static void show_shutdown_confirm_dialog(void);
// static void shutdown_confirm_yes_cb(lv_event_t *e);
// static void shutdown_confirm_cancel_cb(lv_event_t *e);

// /* =========================================================
//  * Utility
//  * ========================================================= */
// void ui_set_fan_ctrl(FanCtrl *ctrl)
// {
//     g_fan_ctrl = ctrl;
// }

// void ui_set_camera_frame(SharedFrame *frame)
// {
//     g_camera_frame = frame;
// }

// void ui_set_behavior_result(BehaviorResult *result)
// {
//     g_behavior_result = result;
// }

// static int clamp_int(int value, int min, int max)
// {
//     if (value < min) return min;
//     if (value > max) return max;
//     return value;
// }

// static void set_label_color(lv_obj_t *label, lv_color_t color)
// {
//     if (label) {
//         lv_obj_set_style_text_color(label, color, 0);
//     }
// }

// static lv_obj_t *label_create(lv_obj_t *parent,
//                               const char *txt,
//                               int x,
//                               int y,
//                               lv_color_t color,
//                               const lv_font_t *font)
// {
//     lv_obj_t *label = lv_label_create(parent);
//     lv_label_set_text(label, txt);
//     lv_obj_set_pos(label, x, y);
//     lv_obj_set_style_text_color(label, color, 0);
//     lv_obj_set_style_text_font(label, font, 0);
//     return label;
// }

// static lv_obj_t *glass_panel_create(lv_obj_t *parent, int x, int y, int w, int h)
// {
//     lv_obj_t *obj = lv_obj_create(parent);
//     lv_obj_set_size(obj, w, h);
//     lv_obj_set_pos(obj, x, y);

//     lv_obj_set_style_bg_color(obj, COLOR_PANEL, 0);
//     lv_obj_set_style_bg_opa(obj, OPACITY_70, 0);
//     lv_obj_set_style_radius(obj, 24, 0);

//     lv_obj_set_style_border_width(obj, 1, 0);
//     lv_obj_set_style_border_color(obj, COLOR_BORDER_DIM, 0);

//     lv_obj_set_style_shadow_width(obj, 18, 0);
//     lv_obj_set_style_shadow_color(obj, lv_color_hex(0x003A66), 0);
//     lv_obj_set_style_shadow_opa(obj, OPACITY_35, 0);

//     lv_obj_set_style_pad_all(obj, 0, 0);
//     lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);

//     return obj;
// }

// static lv_obj_t *soft_panel_create(lv_obj_t *parent, int x, int y, int w, int h)
// {
//     lv_obj_t *obj = lv_obj_create(parent);
//     lv_obj_set_size(obj, w, h);
//     lv_obj_set_pos(obj, x, y);

//     lv_obj_set_style_bg_color(obj, COLOR_PANEL_2, 0);
//     lv_obj_set_style_bg_opa(obj, OPACITY_55, 0);
//     lv_obj_set_style_radius(obj, 18, 0);

//     lv_obj_set_style_border_width(obj, 1, 0);
//     lv_obj_set_style_border_color(obj, COLOR_BORDER_DIM, 0);

//     lv_obj_set_style_pad_all(obj, 0, 0);
//     lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);

//     return obj;
// }

// static void title_create(lv_obj_t *parent, const char *title, int x, int y)
// {
//     lv_obj_t *label = lv_label_create(parent);
//     lv_label_set_text(label, title);
//     lv_obj_set_pos(label, x, y);
//     lv_obj_set_style_text_color(label, COLOR_TEXT, 0);
//     lv_obj_set_style_text_font(label, FONT_MEDIUM, 0);
// }

// static void glow_line_create(lv_obj_t *parent, int x, int y, int w)
// {
//     lv_obj_t *line = lv_obj_create(parent);
//     lv_obj_set_size(line, w, 2);
//     lv_obj_set_pos(line, x, y);
//     lv_obj_set_style_bg_color(line, COLOR_CYAN, 0);
//     lv_obj_set_style_bg_opa(line, OPACITY_55, 0);
//     lv_obj_set_style_border_width(line, 0, 0);
//     lv_obj_set_style_shadow_width(line, 12, 0);
//     lv_obj_set_style_shadow_color(line, COLOR_CYAN, 0);
//     lv_obj_set_style_shadow_opa(line, OPACITY_45, 0);
// }

// static void decorative_curve_lines(lv_obj_t *parent)
// {
//     /*
//      * Pure LVGL cannot draw the same irregular curved glass outlines
//      * as concept art, but several thin translucent lines can reduce
//      * the "boxy" feeling.
//      */
//     // glow_line_create(parent, 20, 20, 180);
//     // glow_line_create(parent, 650, 20, 180);
//     // glow_line_create(parent, 40, PAGE_H - 25, 220);
//     // glow_line_create(parent, 580, PAGE_H - 25, 240);
//     (void)parent;
// }

// static lv_obj_t *pill_create(lv_obj_t *parent, int x, int y, int w, const char *txt)
// {
//     lv_obj_t *pill = lv_obj_create(parent);
//     lv_obj_set_size(pill, w, 32);
//     lv_obj_set_pos(pill, x, y);
//     lv_obj_set_style_radius(pill, 16, 0);
//     lv_obj_set_style_bg_color(pill, lv_color_hex(0x092C25), 0);
//     lv_obj_set_style_bg_opa(pill, OPACITY_70, 0);
//     lv_obj_set_style_border_width(pill, 1, 0);
//     lv_obj_set_style_border_color(pill, COLOR_GREEN, 0);
//     lv_obj_clear_flag(pill, LV_OBJ_FLAG_SCROLLABLE);

//     lv_obj_t *label = lv_label_create(pill);
//     lv_label_set_text(label, txt);
//     lv_obj_set_style_text_color(label, COLOR_GREEN, 0);
//     lv_obj_set_style_text_font(label, FONT_SMALL, 0);
//     lv_obj_center(label);

//     return label;
// }

// static void pill_update(lv_obj_t *label, int detected)
// {
//     if (!label) return;

//     lv_obj_t *pill = lv_obj_get_parent(label);
//     if (!pill) return;

//     /*
//      * detected:
//      * -1 = No Driver / Unknown
//      *  0 = Normal
//      *  1 = Detected
//      */
//     if (detected < 0) {
//         lv_label_set_text(label, "--");
//         lv_obj_set_style_text_color(label, COLOR_SUB_TEXT, 0);
//         lv_obj_set_style_border_color(pill, COLOR_BORDER_DIM, 0);
//         lv_obj_set_style_bg_color(pill, lv_color_hex(0x101826), 0);
//     } else if (detected) {
//         lv_label_set_text(label, "Detected");
//         lv_obj_set_style_text_color(label, COLOR_RED, 0);
//         lv_obj_set_style_border_color(pill, COLOR_RED, 0);
//         lv_obj_set_style_bg_color(pill, lv_color_hex(0x361224), 0);
//     } else {
//         lv_label_set_text(label, "Normal");
//         lv_obj_set_style_text_color(label, COLOR_GREEN, 0);
//         lv_obj_set_style_border_color(pill, COLOR_GREEN, 0);
//         lv_obj_set_style_bg_color(pill, lv_color_hex(0x092C25), 0);
//     }
// }

// /* =========================================================
//  * Navigation helpers
//  * ========================================================= */
// static void page_show(UiPage page);

// static void nav_event_cb(lv_event_t *e)
// {
//     UiPage page = (UiPage)(uintptr_t)lv_event_get_user_data(e);
//     page_show(page);
// }

// /* =========================================================
//  * Header
//  * ========================================================= */
// static void create_header(lv_obj_t *scr)
// {
//     lv_obj_t *top = lv_obj_create(scr);
//     lv_obj_set_size(top, SCREEN_W, HEADER_H);
//     lv_obj_set_pos(top, 0, 0);
//     lv_obj_set_style_bg_color(top, COLOR_HEADER, 0);
//     lv_obj_set_style_bg_opa(top, LV_OPA_COVER, 0);
//     lv_obj_set_style_border_width(top, 0, 0);
//     lv_obj_set_style_pad_all(top, 0, 0);
//     lv_obj_clear_flag(top, LV_OBJ_FLAG_SCROLLABLE);

//     /* 左侧硬件状态 */
//     label_create(top, "Camera OK", 20, 18, COLOR_GREEN, FONT_SMALL);
//     label_create(top, "Mic OK", 140, 18, COLOR_CYAN, FONT_SMALL);
//     label_create(top, "AHT20 OK", 235, 18, COLOR_YELLOW, FONT_SMALL);

//     /* 中间标题 */
//     lv_obj_t *title = lv_label_create(top);
//     lv_label_set_text(title, "DRIVER WELLNESS & ENVIRONMENT SYSTEM");
//     lv_obj_set_style_text_color(title, COLOR_TEXT, 0);
//     lv_obj_set_style_text_font(title, FONT_SMALL, 0);
//     lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 8);

//     glow_line_create(top, 445, 46, 120);

//     /* 右侧时间日期，避免和标题重叠 */
//     label_time = label_create(top, "--:--:--", 840, 10, COLOR_CYAN, FONT_MEDIUM);
//     label_date = label_create(top, "----/--/--", 840, 38, COLOR_SUB_TEXT, FONT_SMALL);
// }

// /* =========================================================
//  * Sidebar navigation
//  * ========================================================= */
// static lv_obj_t *side_btn_create(lv_obj_t *parent,
//                                  const char *icon,
//                                  const char *text,
//                                  int y,
//                                  UiPage page)
// {
//     lv_obj_t *btn = lv_btn_create(parent);
//     lv_obj_set_size(btn, 128, 54);
//     lv_obj_set_pos(btn, 11, y);

//     lv_obj_set_style_radius(btn, 16, 0);
//     lv_obj_set_style_bg_color(btn, COLOR_PANEL_3, 0);
//     lv_obj_set_style_bg_opa(btn, OPACITY_70, 0);
//     lv_obj_set_style_border_width(btn, 1, 0);
//     lv_obj_set_style_border_color(btn, COLOR_BORDER_DIM, 0);

//     lv_obj_t *label_icon = lv_label_create(btn);
//     lv_label_set_text(label_icon, icon);
//     lv_obj_set_style_text_color(label_icon, COLOR_CYAN, 0);
//     lv_obj_set_style_text_font(label_icon, FONT_NORMAL, 0);
//     lv_obj_set_pos(label_icon, 14, 17);

//     lv_obj_t *label_text = lv_label_create(btn);
//     lv_label_set_text(label_text, text);
//     lv_obj_set_style_text_color(label_text, COLOR_TEXT, 0);
//     lv_obj_set_style_text_font(label_text, FONT_SMALL, 0);
//     lv_obj_set_pos(label_text, 42, 17);

//     lv_obj_add_event_cb(btn, nav_event_cb, LV_EVENT_CLICKED, (void *)(uintptr_t)page);
//     nav_btns[page] = btn;

//     return btn;
// }

// static void create_sidebar(lv_obj_t *scr)
// {
//     lv_obj_t *bar = lv_obj_create(scr);
//     lv_obj_set_size(bar, SIDEBAR_W, SCREEN_H - HEADER_H);
//     lv_obj_set_pos(bar, 0, HEADER_H);
//     lv_obj_set_style_bg_color(bar, COLOR_SIDEBAR, 0);
//     lv_obj_set_style_bg_opa(bar, OPACITY_70, 0);
//     lv_obj_set_style_border_width(bar, 0, 0);
//     lv_obj_set_style_pad_all(bar, 0, 0);
//     lv_obj_clear_flag(bar, LV_OBJ_FLAG_SCROLLABLE);

//     side_btn_create(bar, LV_SYMBOL_HOME,     "Home",    35,  PAGE_HOME);
//     side_btn_create(bar, LV_SYMBOL_EYE_OPEN, "Detect",  105, PAGE_DETECTION);
//     side_btn_create(bar, LV_SYMBOL_REFRESH,  "Env",     175, PAGE_ENVIRONMENT);
//     side_btn_create(bar, LV_SYMBOL_LIST,     "Record",  245, PAGE_RECORDS);
//     side_btn_create(bar, LV_SYMBOL_SETTINGS, "Setting", 315, PAGE_SETTINGS);

//     label_create(bar, "System", 28, 430, COLOR_SUB_TEXT, FONT_SMALL);
//     label_create(bar, "OK", 28, 455, COLOR_GREEN, FONT_NORMAL);
//     label_create(bar, "All normal", 28, 480, COLOR_SUB_TEXT, FONT_SMALL);
// }

// static void nav_set_active(UiPage page)
// {
//     for (int i = 0; i < PAGE_COUNT; i++) {
//         if (!nav_btns[i]) continue;

//         if (i == (int)page) {
//             lv_obj_set_style_bg_color(nav_btns[i], COLOR_BLUE, 0);
//             lv_obj_set_style_border_color(nav_btns[i], COLOR_CYAN, 0);
//             lv_obj_set_style_shadow_width(nav_btns[i], 16, 0);
//             lv_obj_set_style_shadow_color(nav_btns[i], COLOR_CYAN, 0);
//             lv_obj_set_style_shadow_opa(nav_btns[i], OPACITY_55, 0);
//         } else {
//             lv_obj_set_style_bg_color(nav_btns[i], COLOR_PANEL_3, 0);
//             lv_obj_set_style_border_color(nav_btns[i], COLOR_BORDER_DIM, 0);
//             lv_obj_set_style_shadow_width(nav_btns[i], 0, 0);
//         }
//     }
// }

// static void page_show(UiPage page)
// {
//     for (int i = 0; i < PAGE_COUNT; i++) {
//         if (!pages[i]) continue;

//         if (i == (int)page) {
//             lv_obj_clear_flag(pages[i], LV_OBJ_FLAG_HIDDEN);
//         } else {
//             lv_obj_add_flag(pages[i], LV_OBJ_FLAG_HIDDEN);
//         }
//     }

//     nav_set_active(page);
// }

// /* =========================================================
//  * Home page
//  * ========================================================= */
// static void create_home_page(lv_obj_t *scr)
// {
//     lv_obj_t *page = lv_obj_create(scr);
//     pages[PAGE_HOME] = page;

//     lv_obj_set_size(page, PAGE_W, PAGE_H);
//     lv_obj_set_pos(page, PAGE_X, PAGE_Y);
//     lv_obj_set_style_bg_opa(page, LV_OPA_TRANSP, 0);
//     lv_obj_set_style_border_width(page, 0, 0);
//     lv_obj_set_style_pad_all(page, 0, 0);
//     lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);

//     decorative_curve_lines(page);

//     lv_obj_t *main = glass_panel_create(page, 0, 0, 465, 440);
//     title_create(main, "1. HOME", 28, 18);

//     home_state_arc = lv_arc_create(main);
//     lv_obj_set_size(home_state_arc, 295, 295);
//     lv_obj_set_pos(home_state_arc, 82, 58);
//     lv_arc_set_range(home_state_arc, 0, 100);
//     lv_arc_set_value(home_state_arc, 20);
//     lv_arc_set_bg_angles(home_state_arc, 0, 360);
//     lv_obj_remove_style(home_state_arc, NULL, LV_PART_KNOB);
//     lv_obj_clear_flag(home_state_arc, LV_OBJ_FLAG_CLICKABLE);
//     lv_obj_set_style_arc_width(home_state_arc, 16, LV_PART_MAIN);
//     lv_obj_set_style_arc_width(home_state_arc, 16, LV_PART_INDICATOR);
//     lv_obj_set_style_arc_color(home_state_arc, lv_color_hex(0x0D2D44), LV_PART_MAIN);
//     lv_obj_set_style_arc_color(home_state_arc, COLOR_GREEN, LV_PART_INDICATOR);

//     home_state_icon = label_create(main, ":)", 206, 142, COLOR_GREEN, FONT_LARGE);
//     home_state_label = label_create(main, "NORMAL", 155, 220, COLOR_GREEN, FONT_LARGE);
//     label_create(main, "Drive Safely", 188, 260, COLOR_GREEN, FONT_SMALL);
//     // home_state_desc = label_create(main, "Driver status is good. Keep focused.", 92, 342, COLOR_SUB_TEXT, FONT_SMALL);
//     // home_alert_text = label_create(main, "No abnormal alert", 170, 372, COLOR_GREEN, FONT_SMALL);
//     home_state_desc = label_create(main,
//                                 "Driver status is good. Keep focused.",
//                                 40, 355,
//                                 COLOR_SUB_TEXT,
//                                 FONT_NORMAL);
//     lv_obj_set_width(home_state_desc, 385);
//     lv_obj_set_style_text_align(home_state_desc, LV_TEXT_ALIGN_CENTER, 0);

//     home_alert_text = label_create(main,
//                                 "No abnormal alert",
//                                 40, 390,
//                                 COLOR_GREEN,
//                                 FONT_NORMAL);
//     lv_obj_set_width(home_alert_text, 385);
//     lv_obj_set_style_text_align(home_alert_text, LV_TEXT_ALIGN_CENTER, 0);

//     lv_obj_t *score = glass_panel_create(page, 490, 0, 350, 130);
//     title_create(score, "FATIGUE SCORE", 24, 14);

//     home_score_label = label_create(score, "0", 105, 40, COLOR_GREEN, FONT_NUMBER);
//     label_create(score, "Normal", 118, 84, COLOR_GREEN, FONT_SMALL);

//     home_score_bar = lv_bar_create(score);
//     lv_obj_set_size(home_score_bar, 250, 10);
//     lv_obj_set_pos(home_score_bar, 48, 104);
//     lv_bar_set_range(home_score_bar, 0, 100);
//     lv_bar_set_value(home_score_bar, 0, LV_ANIM_OFF);
//     lv_obj_set_style_bg_color(home_score_bar, lv_color_hex(0x20364A), LV_PART_MAIN);
//     lv_obj_set_style_bg_color(home_score_bar, COLOR_GREEN, LV_PART_INDICATOR);
//     lv_obj_set_style_radius(home_score_bar, 5, LV_PART_MAIN);
//     lv_obj_set_style_radius(home_score_bar, 5, LV_PART_INDICATOR);

//     lv_obj_t *summary = glass_panel_create(page, 490, 150, 350, 125);
//     title_create(summary, "DETECTION SUMMARY", 24, 14);

//     label_create(summary, "Head", 28, 58, COLOR_SUB_TEXT, FONT_SMALL);
//     home_head_pill = pill_create(summary, 80, 50, 80, "Normal");
//     label_create(summary, "Phone", 180, 58, COLOR_SUB_TEXT, FONT_SMALL);
//     home_phone_pill = pill_create(summary, 245, 50, 80, "Normal");
//     label_create(summary, "Mouth", 28, 98, COLOR_SUB_TEXT, FONT_SMALL);
//     home_smoke_pill = pill_create(summary, 80, 90, 80, "Normal");
//     label_create(summary, "Wheel", 180, 98, COLOR_SUB_TEXT, FONT_SMALL);
//     home_wheel_pill = pill_create(summary, 245, 90, 80, "On");

//     lv_obj_t *sys = glass_panel_create(page, 490, 295, 350, 145);
//     title_create(sys, "SYSTEM SUMMARY", 24, 14);

//     label_create(sys, "Fan", 28, 60, COLOR_SUB_TEXT, FONT_SMALL);
//     home_fan_mode = label_create(sys, "Auto", 28, 84, COLOR_GREEN, FONT_NORMAL);

//     label_create(sys, "Speed", 118, 60, COLOR_SUB_TEXT, FONT_SMALL);
//     home_fan_speed = label_create(sys, "0%", 118, 84, COLOR_CYAN, FONT_NORMAL);

//     label_create(sys, "Comfort", 215, 60, COLOR_SUB_TEXT, FONT_SMALL);
//     home_comfort = label_create(sys, "Good", 215, 84, COLOR_GREEN, FONT_NORMAL);

//     label_create(sys, "Chip", 28, 112, COLOR_SUB_TEXT, FONT_SMALL);
//     home_chip_temp = label_create(sys, "--C", 78, 112, COLOR_YELLOW, FONT_SMALL);
// }

// /* =========================================================
//  * Detection page
//  * ========================================================= */
// static int clamp_coord(int v, int min_v, int max_v)
// {
//     if (v < min_v) return min_v;
//     if (v > max_v) return max_v;
//     return v;
// }

// static void hide_all_overlay_objects(void)
// {
//     for (int i = 0; i < MAX_UI_FACES; i++) {
//         if (face_boxes[i]) {
//             lv_obj_add_flag(face_boxes[i], LV_OBJ_FLAG_HIDDEN);
//         }

//         for (int j = 0; j < FACE_POINTS; j++) {
//             if (face_points[i][j]) {
//                 lv_obj_add_flag(face_points[i][j], LV_OBJ_FLAG_HIDDEN);
//             }
//         }
//     }

//     for (int i = 0; i < POSE_POINTS; i++) {
//         if (pose_points[i]) {
//             lv_obj_add_flag(pose_points[i], LV_OBJ_FLAG_HIDDEN);
//         }
//     }

//     for (int i = 0; i < SKELETON_LINES; i++) {
//         if (pose_lines[i]) {
//             lv_obj_add_flag(pose_lines[i], LV_OBJ_FLAG_HIDDEN);
//         }
//     }
// }

// //骨架线刷新函数
// static int pose_valid[POSE_POINTS];
// static lv_point_t pose_line_points[SKELETON_LINES][2];

// static void update_pose_skeleton_lines(void)
// {
//     for (int i = 0; i < SKELETON_LINES; i++) {
//         int p1 = skeleton_pairs[i][0];
//         int p2 = skeleton_pairs[i][1];

//         if (!pose_lines[i]) {
//             continue;
//         }

//         if (!pose_valid[p1] || !pose_valid[p2]) {
//             lv_obj_add_flag(pose_lines[i], LV_OBJ_FLAG_HIDDEN);
//             continue;
//         }

//         int x1 = lv_obj_get_x(pose_points[p1]) + 3;
//         int y1 = lv_obj_get_y(pose_points[p1]) + 3;
//         int x2 = lv_obj_get_x(pose_points[p2]) + 3;
//         int y2 = lv_obj_get_y(pose_points[p2]) + 3;

//         pose_line_points[i][0].x = x1;
//         pose_line_points[i][0].y = y1;
//         pose_line_points[i][1].x = x2;
//         pose_line_points[i][1].y = y2;

//         lv_line_set_points(pose_lines[i], pose_line_points[i], 2);
//         lv_obj_clear_flag(pose_lines[i], LV_OBJ_FLAG_HIDDEN);
//     }
// }

// void ui_update_detection_overlay(void)
// {
//     if (!camera_overlay || !g_behavior_result || !g_camera_frame) {
//         return;
//     }

//     if (!g_overlay_enabled) {
//         hide_all_overlay_objects();
//         return;
//     }

//     int src_w = g_camera_frame->width;
//     int src_h = g_camera_frame->height;

//     if (src_w <= 0 || src_h <= 0) {
//         src_w = CAM_WIDTH;
//         src_h = CAM_HEIGHT;
//     }

//     camera_compute_crop_rect(src_w, src_h);

//     float scale_x = (float)CAM_PREVIEW_W / (float)g_crop_w;
//     float scale_y = (float)CAM_PREVIEW_H / (float)g_crop_h;

//     hide_all_overlay_objects();

//     /* ===================== 人脸框 + 5点关键点 ===================== */
//     if (g_behavior_result->face_detected && g_behavior_result->face_count > 0) {
//         int count = g_behavior_result->face_count;
//         if (count > MAX_UI_FACES) {
//             count = MAX_UI_FACES;
//         }

//         for (int i = 0; i < count; i++) {
//             FaceInfo *face = &g_behavior_result->faces[i];

//             int x1 = (int)((face->left   - g_crop_x) * scale_x);
//             int y1 = (int)((face->top    - g_crop_y) * scale_y);
//             int x2 = (int)((face->right  - g_crop_x) * scale_x);
//             int y2 = (int)((face->bottom - g_crop_y) * scale_y);

//             x1 = clamp_coord(x1, 0, CAM_PREVIEW_W - 1);
//             y1 = clamp_coord(y1, 0, CAM_PREVIEW_H - 1);
//             x2 = clamp_coord(x2, 0, CAM_PREVIEW_W - 1);
//             y2 = clamp_coord(y2, 0, CAM_PREVIEW_H - 1);

//             int w = x2 - x1;
//             int h = y2 - y1;

//             if (w >= 5 && h >= 5 && face_boxes[i]) {
//                 lv_obj_clear_flag(face_boxes[i], LV_OBJ_FLAG_HIDDEN);
//                 lv_obj_set_pos(face_boxes[i], x1, y1);
//                 lv_obj_set_size(face_boxes[i], w, h);
//             }

//             for (int j = 0; j < FACE_POINTS; j++) {
//                 if (!face_points[i][j]) continue;

//                 int px = (int)((face->points[j][0] - g_crop_x) * scale_x);
//                 int py = (int)((face->points[j][1] - g_crop_y) * scale_y);

//                 px = clamp_coord(px, 0, CAM_PREVIEW_W - 1);
//                 py = clamp_coord(py, 0, CAM_PREVIEW_H - 1);

//                 lv_obj_clear_flag(face_points[i][j], LV_OBJ_FLAG_HIDDEN);
//                 lv_obj_set_pos(face_points[i][j], px - 3, py - 3);

//                 if (j == 0 || j == 1) {
//                     lv_obj_set_style_bg_color(face_points[i][j], COLOR_CYAN, 0);
//                 } else if (j == 2) {
//                     lv_obj_set_style_bg_color(face_points[i][j], COLOR_YELLOW, 0);
//                 } else {
//                     lv_obj_set_style_bg_color(face_points[i][j], COLOR_GREEN, 0);
//                 }
//             }
//         }
//     }

// /* ===================== 姿态 17 个关键点 + 骨架线 ===================== */
// for (int i = 0; i < POSE_POINTS; i++) {
//     pose_valid[i] = 0;
// }

// if (g_behavior_result->pose_detected) {
//     for (int i = 0; i < POSE_POINTS; i++) {
//         if (!pose_points[i]) continue;

//         float raw_x = g_behavior_result->pose_keypoints[i][0];
//         float raw_y = g_behavior_result->pose_keypoints[i][1];

//         /*
//          * 无效点过滤：
//          * 模型未检测到时，可能是 0,0。
//          */
//         if (raw_x <= 1.0f && raw_y <= 1.0f) {
//             continue;
//         }

//         if (raw_x < g_crop_x || raw_x > g_crop_x + g_crop_w ||
//             raw_y < g_crop_y || raw_y > g_crop_y + g_crop_h) {
//             continue;
//         }

//         int px = (int)((raw_x - g_crop_x) * scale_x);
//         int py = (int)((raw_y - g_crop_y) * scale_y);

//         px = clamp_coord(px, 0, CAM_PREVIEW_W - 1);
//         py = clamp_coord(py, 0, CAM_PREVIEW_H - 1);

//         lv_obj_clear_flag(pose_points[i], LV_OBJ_FLAG_HIDDEN);
//         lv_obj_set_pos(pose_points[i], px - 3, py - 3);

//         pose_valid[i] = 1;

//         if (i <= 4) {
//             lv_obj_set_style_bg_color(pose_points[i], COLOR_CYAN, 0);
//         } else if (i <= 10) {
//             lv_obj_set_style_bg_color(pose_points[i], COLOR_GREEN, 0);
//         } else {
//             lv_obj_set_style_bg_color(pose_points[i], COLOR_ORANGE, 0);
//         }
//     }

//     update_pose_skeleton_lines();
// }

// }

// static void overlay_switch_event_cb(lv_event_t *e)
// {
//     lv_obj_t *sw = lv_event_get_target(e);
//     g_overlay_enabled = lv_obj_has_state(sw, LV_STATE_CHECKED);

//     if (overlay_state_label) {
//         lv_label_set_text(overlay_state_label, g_overlay_enabled ? "On" : "Off");
//         lv_obj_set_style_text_color(overlay_state_label,
//                                     g_overlay_enabled ? COLOR_GREEN : COLOR_SUB_TEXT,
//                                     0);
//     }

//     ui_update_detection_overlay();
// }

// static void update_view_mode_buttons(void)
// {
//     for (int i = 0; i < CAMERA_VIEW_COUNT; i++) {
//         if (!view_mode_btns[i]) continue;

//         if (i == (int)g_camera_view_mode) {
//             lv_obj_set_style_bg_color(view_mode_btns[i], COLOR_BLUE, 0);
//             lv_obj_set_style_border_color(view_mode_btns[i], COLOR_CYAN, 0);
//             lv_obj_set_style_shadow_width(view_mode_btns[i], 12, 0);
//             lv_obj_set_style_shadow_color(view_mode_btns[i], COLOR_CYAN, 0);
//             lv_obj_set_style_shadow_opa(view_mode_btns[i], OPACITY_45, 0);
//         } else {
//             lv_obj_set_style_bg_color(view_mode_btns[i], COLOR_PANEL_2, 0);
//             lv_obj_set_style_border_color(view_mode_btns[i], COLOR_BORDER_DIM, 0);
//             lv_obj_set_style_shadow_width(view_mode_btns[i], 0, 0);
//         }
//     }

//     if (view_mode_label) {
//         if (g_camera_view_mode == CAMERA_VIEW_WIDE) {
//             lv_label_set_text(view_mode_label, "Wide");
//         } else if (g_camera_view_mode == CAMERA_VIEW_NORMAL) {
//             lv_label_set_text(view_mode_label, "Normal");
//         } else {
//             lv_label_set_text(view_mode_label, "Focus");
//         }
//     }
// }

// static void view_mode_event_cb(lv_event_t *e)
// {
//     CameraViewMode mode = (CameraViewMode)(uintptr_t)lv_event_get_user_data(e);

//     if (mode < 0 || mode >= CAMERA_VIEW_COUNT) {
//         return;
//     }

//     g_camera_view_mode = mode;
//     update_view_mode_buttons();

//     /*
//      * 强制下一帧刷新图像，否则如果 frame_count 没变，
//      * 画面可能不会立刻切换。
//      */
//     last_camera_frame_count = -1;

//     ui_update_camera_preview();
//     ui_update_detection_overlay();
// }

// static lv_obj_t *view_mode_btn_create(lv_obj_t *parent,
//                                       const char *txt,
//                                       int x,
//                                       int y,
//                                       CameraViewMode mode)
// {
//     lv_obj_t *btn = lv_btn_create(parent);
//     lv_obj_set_size(btn, 72, 30);
//     lv_obj_set_pos(btn, x, y);

//     lv_obj_set_style_radius(btn, 14, 0);
//     lv_obj_set_style_bg_color(btn, COLOR_PANEL_2, 0);
//     lv_obj_set_style_bg_opa(btn, OPACITY_70, 0);
//     lv_obj_set_style_border_width(btn, 1, 0);
//     lv_obj_set_style_border_color(btn, COLOR_BORDER_DIM, 0);

//     lv_obj_t *label = lv_label_create(btn);
//     lv_label_set_text(label, txt);
//     lv_obj_set_style_text_font(label, FONT_SMALL, 0);
//     lv_obj_set_style_text_color(label, COLOR_TEXT, 0);
//     lv_obj_center(label);

//     lv_obj_add_event_cb(btn,
//                         view_mode_event_cb,
//                         LV_EVENT_CLICKED,
//                         (void *)(uintptr_t)mode);

//     view_mode_btns[mode] = btn;

//     return btn;
// }

// static void create_overlay_objects(lv_obj_t *parent)
// {
//     camera_overlay = lv_obj_create(parent);
//     lv_obj_set_size(camera_overlay, CAM_PREVIEW_W, CAM_PREVIEW_H);
//     lv_obj_set_pos(camera_overlay, 0, CAM_IMG_Y_OFF);

//     lv_obj_set_style_bg_opa(camera_overlay, LV_OPA_TRANSP, 0);
//     lv_obj_set_style_border_width(camera_overlay, 0, 0);
//     lv_obj_set_style_pad_all(camera_overlay, 0, 0);
//     lv_obj_clear_flag(camera_overlay, LV_OBJ_FLAG_SCROLLABLE);
//     lv_obj_clear_flag(camera_overlay, LV_OBJ_FLAG_CLICKABLE);

//     /* 人脸框 + 5点 */
//     for (int i = 0; i < MAX_UI_FACES; i++) {
//         face_boxes[i] = lv_obj_create(camera_overlay);
//         lv_obj_set_size(face_boxes[i], 10, 10);
//         lv_obj_set_pos(face_boxes[i], 0, 0);
//         lv_obj_set_style_bg_opa(face_boxes[i], LV_OPA_TRANSP, 0);
//         lv_obj_set_style_border_width(face_boxes[i], 2, 0);
//         lv_obj_set_style_border_color(face_boxes[i], COLOR_CYAN, 0);
//         lv_obj_set_style_radius(face_boxes[i], 4, 0);
//         lv_obj_clear_flag(face_boxes[i], LV_OBJ_FLAG_SCROLLABLE);
//         lv_obj_add_flag(face_boxes[i], LV_OBJ_FLAG_HIDDEN);

//         for (int j = 0; j < FACE_POINTS; j++) {
//             face_points[i][j] = lv_obj_create(camera_overlay);
//             lv_obj_set_size(face_points[i][j], 7, 7);
//             lv_obj_set_pos(face_points[i][j], 0, 0);
//             lv_obj_set_style_radius(face_points[i][j], 4, 0);
//             lv_obj_set_style_bg_color(face_points[i][j], COLOR_GREEN, 0);
//             lv_obj_set_style_bg_opa(face_points[i][j], LV_OPA_COVER, 0);
//             lv_obj_set_style_border_width(face_points[i][j], 0, 0);
//             lv_obj_clear_flag(face_points[i][j], LV_OBJ_FLAG_SCROLLABLE);
//             lv_obj_add_flag(face_points[i][j], LV_OBJ_FLAG_HIDDEN);
//         }
//     }

//     /*
//      * 先创建骨架线，再创建姿态点。
//      * 这样点会显示在线上面，更清楚。
//      */
//     for (int i = 0; i < SKELETON_LINES; i++) {
//         pose_lines[i] = lv_line_create(camera_overlay);

//         static lv_point_t dummy_points[2] = {
//             {0, 0},
//             {1, 1}
//         };

//         lv_line_set_points(pose_lines[i], dummy_points, 2);
//         lv_obj_set_style_line_width(pose_lines[i], 2, 0);
//         lv_obj_set_style_line_color(pose_lines[i], COLOR_CYAN, 0);
//         lv_obj_set_style_line_opa(pose_lines[i], LV_OPA_70, 0);
//         lv_obj_add_flag(pose_lines[i], LV_OBJ_FLAG_HIDDEN);
//     }

//     /* 创建姿态 17 个关键点 */
//     for (int i = 0; i < POSE_POINTS; i++) {
//         pose_points[i] = lv_obj_create(camera_overlay);
//         lv_obj_set_size(pose_points[i], 6, 6);
//         lv_obj_set_pos(pose_points[i], 0, 0);
//         lv_obj_set_style_radius(pose_points[i], 3, 0);
//         lv_obj_set_style_bg_color(pose_points[i], COLOR_ORANGE, 0);
//         lv_obj_set_style_bg_opa(pose_points[i], LV_OPA_COVER, 0);
//         lv_obj_set_style_border_width(pose_points[i], 0, 0);
//         lv_obj_clear_flag(pose_points[i], LV_OBJ_FLAG_SCROLLABLE);
//         lv_obj_add_flag(pose_points[i], LV_OBJ_FLAG_HIDDEN);
//     }
// }







// static void create_detection_page(lv_obj_t *scr)
// {
//     lv_obj_t *page = lv_obj_create(scr);
//     pages[PAGE_DETECTION] = page;

//     lv_obj_set_size(page, PAGE_W, PAGE_H);
//     lv_obj_set_pos(page, PAGE_X, PAGE_Y);
//     lv_obj_set_style_bg_opa(page, LV_OPA_TRANSP, 0);
//     lv_obj_set_style_border_width(page, 0, 0);
//     lv_obj_set_style_pad_all(page, 0, 0);
//     lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);

//     decorative_curve_lines(page);

//     /* 左侧大摄像头区域 */
//     lv_obj_t *camera = glass_panel_create(page, 0, 0, 620, 440);
//     title_create(camera, "2. DETECTION  /  LIVE DRIVER MONITORING", 28, 18);

//     lv_obj_t *preview = lv_obj_create(camera);
//     camera_preview_panel = preview;

//     /*
//      * 原来是 555 x 330。
//      * 这里小幅放大到 575 x 345，不大改布局，比较安全。
//      */
//     lv_obj_set_size(preview, 610, 355);
//     lv_obj_set_pos(preview, 5, 50);
//     lv_obj_set_style_radius(preview, 20, 0);
//     lv_obj_set_style_bg_color(preview, lv_color_hex(0x071423), 0);
//     lv_obj_set_style_bg_opa(preview, OPACITY_70, 0);
//     lv_obj_set_style_border_width(preview, 1, 0);
//     lv_obj_set_style_border_color(preview, COLOR_BORDER_DIM, 0);
//     lv_obj_set_style_pad_all(preview, 0, 0);
//     lv_obj_clear_flag(preview, LV_OBJ_FLAG_SCROLLABLE);

//     /*
//      * 真实摄像头图像显示对象。
//      * 注意：
//      * 如果 CAM_PREVIEW_W / CAM_PREVIEW_H 仍然是 555 / 312，
//      * 那图像本身不会明显变大，只是外框变大。
//      * 如果你想图像也变大，后面再统一改 CAM_PREVIEW_W/H。
//      */
//     camera_img = lv_img_create(preview);
//     lv_obj_set_size(camera_img, CAM_PREVIEW_W, CAM_PREVIEW_H);

//     /*
//      * 让图像在新的 preview 里居中一些。
//      * 如果你的 CAM_IMG_Y_OFF 已经适配，可以继续用原来的。
//      */
//     //lv_obj_set_pos(camera_img, (600 - CAM_PREVIEW_W) / 2, CAM_IMG_Y_OFF);
//     lv_obj_set_pos(camera_img, 0, 11);

//     /*
//      * Overlay 对象放在图像上层。
//      */
//     create_overlay_objects(preview);

//     /* 占位文字：有画面后隐藏 */
//     camera_placeholder_1 = label_create(preview,
//                                         "CAMERA PREVIEW AREA",
//                                         190,
//                                         135,
//                                         COLOR_CYAN,
//                                         FONT_MEDIUM);

//     camera_placeholder_2 = label_create(preview,
//                                         "Waiting for SharedFrame...",
//                                         190,
//                                         175,
//                                         COLOR_SUB_TEXT,
//                                         FONT_SMALL);

//     /* 四角 HUD 装饰线：跟随新 preview 尺寸调整 */
//     glow_line_create(preview, 40, 35, 80);
//     glow_line_create(preview, 490, 35, 80);
//     glow_line_create(preview, 40, 325, 80);
//     glow_line_create(preview, 490, 325, 80);

//     /* Camera view mode：整体下移，利用底部空间 */
//     label_create(camera, "View", 36, 420, COLOR_SUB_TEXT, FONT_SMALL);
//     view_mode_label = label_create(camera, "Wide", 80, 420, COLOR_GREEN, FONT_SMALL);

//     view_mode_btn_create(camera, "Wide",   150, 413, CAMERA_VIEW_WIDE);
//     view_mode_btn_create(camera, "Normal", 240, 413, CAMERA_VIEW_NORMAL);
//     view_mode_btn_create(camera, "Focus",  340, 413, CAMERA_VIEW_FOCUS);

//     update_view_mode_buttons();

//     /* 右上：驾驶状态 */
//     lv_obj_t *state = glass_panel_create(page, 630, 0, 210, 200);
//     title_create(state, "DRIVER STATE", 20, 14);

//     det_risk_text = label_create(state, "Current Risk:", 28, 50, COLOR_SUB_TEXT, FONT_SMALL);
//     //label_create(state, "Normal", 28, 72, COLOR_GREEN, FONT_LARGE);
//     det_driver_state_label = label_create(state, "Normal", 28, 72, COLOR_GREEN, FONT_NORMAL);

//     /*
//      * Detect 页状态分：
//      * 演示时不用再切回 Home 页面看总分。
//      */
//     detect_risk_score_text_label = label_create(state, "Risk Score", 28, 108, COLOR_SUB_TEXT, FONT_SMALL);
//     detect_risk_score_label = label_create(state, "0", 145, 106, COLOR_GREEN, FONT_NORMAL);

//     /* 新增：车辆运动状态 */
//     label_create(state, "Vehicle", 28, 134, COLOR_SUB_TEXT, FONT_SMALL);
//     det_vehicle_state = label_create(state, "PARKED", 105, 134, COLOR_YELLOW, FONT_SMALL);

//     det_explain_text = label_create(state, "No abnormal behavior.", 28, 154, COLOR_SUB_TEXT, FONT_SMALL);

//     /* Overlay 开关 */
//     label_create(state, "Overlay", 28, 176, COLOR_SUB_TEXT, FONT_SMALL);

//     overlay_state_label = label_create(state, "On", 92, 176, COLOR_GREEN, FONT_SMALL);

//     overlay_switch = lv_switch_create(state);
//     lv_obj_set_size(overlay_switch, 46, 24);
//     lv_obj_set_pos(overlay_switch, 145, 170);
//     lv_obj_add_state(overlay_switch, LV_STATE_CHECKED);
//     lv_obj_add_event_cb(overlay_switch, overlay_switch_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

//     /* 右下：行为信号 */
//     lv_obj_t *behavior = glass_panel_create(page, 630, 220, 210, 220);
//     title_create(behavior, "BEHAVIOR SIGNAL", 20, 14);

//     label_create(behavior, "Eyes", 25, 52, COLOR_SUB_TEXT, FONT_SMALL);
//     det_eye_state = label_create(behavior, "Normal", 105, 52, COLOR_GREEN, FONT_SMALL);

//     label_create(behavior, "EyeRisk", 25, 79, COLOR_SUB_TEXT, FONT_SMALL);
//     det_perclos = label_create(behavior, "0%", 105, 79, COLOR_CYAN, FONT_SMALL);

//     label_create(behavior, "Yaw", 25, 106, COLOR_SUB_TEXT, FONT_SMALL);
//     det_blink = label_create(behavior, "Normal", 105, 106, COLOR_CYAN, FONT_SMALL);

//     label_create(behavior, "Wheel", 25, 133, COLOR_SUB_TEXT, FONT_SMALL);
//     det_yawn = label_create(behavior, "On", 105, 133, COLOR_GREEN, FONT_SMALL);

//     /*
//      * 底部三个状态胶囊。
//      * Head / Phone / Mouth 三项保持现在的布局。
//      */
//     det_head_pill  = pill_create(behavior, 25, 163, 85, "Normal");
//     det_phone_pill = pill_create(behavior, 120, 163, 85, "Normal");
//     det_smoke_pill = pill_create(behavior, 72, 193, 85, "Normal");
// }

// /* =========================================================
//  * Environment page
//  * ========================================================= */
// static void fan_slider_event_cb(lv_event_t *e)
// {
//     if (g_updating_slider) return;
//     if (!g_fan_ctrl || g_fan_ctrl->mode != 2) return;

//     lv_obj_t *slider = lv_event_get_target(e);
//     int value = lv_slider_get_value(slider);

//     g_fan_ctrl->mode = 2;
//     g_fan_ctrl->manual_speed = value;

//     ui_update_fan_speed(value);
// }

// static void fan_mode_btn_event_cb(lv_event_t *e)
// {
//     const char *mode = (const char*)lv_event_get_user_data(e);

//     if (!g_fan_ctrl || !mode) return;

//     if (strcmp(mode, "auto") == 0) {
//         g_fan_ctrl->mode = 0;
//         ui_update_fan_mode("Auto");

//         if (slider_fan) {
//             lv_obj_clear_flag(slider_fan, LV_OBJ_FLAG_CLICKABLE);
//             lv_obj_add_state(slider_fan, LV_STATE_DISABLED);
//         }
//     } else if (strcmp(mode, "manual") == 0) {
//         g_fan_ctrl->mode = 2;
//         ui_update_fan_mode("Manual");

//         if (slider_fan) {
//             lv_obj_add_flag(slider_fan, LV_OBJ_FLAG_CLICKABLE);
//             lv_obj_clear_state(slider_fan, LV_STATE_DISABLED);

//             g_updating_slider = true;
//             lv_slider_set_value(slider_fan, g_fan_ctrl->manual_speed, LV_ANIM_OFF);
//             g_updating_slider = false;
//         }

//         ui_update_fan_speed(g_fan_ctrl->manual_speed);
//     } else if (strcmp(mode, "off") == 0) {
//         g_fan_ctrl->mode = 3;
//         ui_update_fan_mode("Off");

//         if (slider_fan) {
//             lv_obj_clear_flag(slider_fan, LV_OBJ_FLAG_CLICKABLE);
//             lv_obj_add_state(slider_fan, LV_STATE_DISABLED);
//         }
//     }
// }

// static lv_obj_t *mode_btn_create(lv_obj_t *parent,
//                                  const char *txt,
//                                  int x,
//                                  int y,
//                                  const char *mode)
// {
//     lv_obj_t *btn = lv_btn_create(parent);
//     lv_obj_set_size(btn, 110, 38);
//     lv_obj_set_pos(btn, x, y);
//     lv_obj_set_style_radius(btn, 12, 0);
//     lv_obj_set_style_bg_color(btn, COLOR_PANEL_2, 0);
//     lv_obj_set_style_border_width(btn, 1, 0);
//     lv_obj_set_style_border_color(btn, COLOR_BORDER_DIM, 0);

//     lv_obj_t *label = lv_label_create(btn);
//     lv_label_set_text(label, txt);
//     lv_obj_center(label);

//     lv_obj_add_event_cb(btn, fan_mode_btn_event_cb, LV_EVENT_CLICKED, (void*)mode);

//     return btn;
// }

// static void create_environment_page(lv_obj_t *scr)
// {
//     lv_obj_t *page = lv_obj_create(scr);
//     pages[PAGE_ENVIRONMENT] = page;

//     lv_obj_set_size(page, PAGE_W, PAGE_H);
//     lv_obj_set_pos(page, PAGE_X, PAGE_Y);
//     lv_obj_set_style_bg_opa(page, LV_OPA_TRANSP, 0);
//     lv_obj_set_style_border_width(page, 0, 0);
//     lv_obj_set_style_pad_all(page, 0, 0);
//     lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);

//     decorative_curve_lines(page);

//     lv_obj_t *fan = glass_panel_create(page, 0, 0, 455, 440);
//     title_create(fan, "3. ENVIRONMENT / FAN CONTROL", 24, 18);

//     env_fan_arc = lv_arc_create(fan);
//     lv_obj_set_size(env_fan_arc, 230, 230);
//     lv_obj_set_pos(env_fan_arc, 70, 70);
//     lv_arc_set_range(env_fan_arc, 0, 100);
//     lv_arc_set_value(env_fan_arc, 40);
//     lv_obj_remove_style(env_fan_arc, NULL, LV_PART_KNOB);
//     lv_obj_clear_flag(env_fan_arc, LV_OBJ_FLAG_CLICKABLE);
//     lv_obj_set_style_arc_width(env_fan_arc, 16, LV_PART_MAIN);
//     lv_obj_set_style_arc_width(env_fan_arc, 16, LV_PART_INDICATOR);
//     lv_obj_set_style_arc_color(env_fan_arc, lv_color_hex(0x0D2D44), LV_PART_MAIN);
//     lv_obj_set_style_arc_color(env_fan_arc, COLOR_CYAN, LV_PART_INDICATOR);

//     label_create(fan, "FAN", 155, 160, COLOR_CYAN, FONT_LARGE);
//     env_fan_speed = label_create(fan, "0%", 155, 220, COLOR_CYAN, FONT_NUMBER);

//     mode_btn_create(fan, "Auto", 45, 330, "auto");
//     mode_btn_create(fan, "Manual", 170, 330, "manual");
//     mode_btn_create(fan, "Off", 295, 330, "off");

//     slider_fan = lv_slider_create(fan);
//     lv_obj_set_size(slider_fan, 350, 14);
//     lv_obj_set_pos(slider_fan, 52, 390);
//     lv_slider_set_range(slider_fan, 0, 100);
//     lv_slider_set_value(slider_fan, 40, LV_ANIM_OFF);
//     lv_obj_set_style_bg_color(slider_fan, lv_color_hex(0x20364A), LV_PART_MAIN);
//     lv_obj_set_style_bg_color(slider_fan, COLOR_CYAN, LV_PART_INDICATOR);
//     lv_obj_set_style_bg_color(slider_fan, COLOR_TEXT, LV_PART_KNOB);
//     lv_obj_add_event_cb(slider_fan, fan_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
//     lv_obj_clear_flag(slider_fan, LV_OBJ_FLAG_CLICKABLE);
//     lv_obj_add_state(slider_fan, LV_STATE_DISABLED);

//     lv_obj_t *info = glass_panel_create(page, 490, 0, 350, 200);
//     title_create(info, "CABIN ENVIRONMENT", 24, 14);

//     label_create(info, "Temperature", 35, 70, COLOR_SUB_TEXT, FONT_NORMAL);
//     env_temp_label = label_create(info, "--C", 35, 105, COLOR_CYAN, FONT_LARGE);

//     label_create(info, "Humidity", 205, 70, COLOR_SUB_TEXT, FONT_NORMAL);
//     env_humi_label = label_create(info, "--%", 205, 105, COLOR_CYAN, FONT_LARGE);

//     lv_obj_t *thermal = glass_panel_create(page, 490, 225, 350, 215);
//     title_create(thermal, "THERMAL STATUS", 24, 14);

//     label_create(thermal, "Mode", 35, 70, COLOR_SUB_TEXT, FONT_NORMAL);
//     env_fan_mode = label_create(thermal, "Auto", 140, 70, COLOR_GREEN, FONT_NORMAL);

//     label_create(thermal, "Comfort", 35, 110, COLOR_SUB_TEXT, FONT_NORMAL);
//     env_comfort_label = label_create(thermal, "Good", 140, 110, COLOR_GREEN, FONT_NORMAL);

//     label_create(thermal, "Chip Temp", 35, 150, COLOR_SUB_TEXT, FONT_NORMAL);
//     env_chip_temp = label_create(thermal, "--C", 140, 150, COLOR_YELLOW, FONT_NORMAL);
// }

// /* =========================================================
//  * Records page
//  * ========================================================= */
// static const char *ui_get_time_text(void)
// {
//     if (label_time) {
//         return lv_label_get_text(label_time);
//     }

//     return "--:--:--";
// }

// static void records_refresh_ui(void)
// {
//     for (int i = 0; i < MAX_EVENT_RECORDS; i++) {
//         if (rec_event_labels[i]) {
//             lv_label_set_text(rec_event_labels[i], rec_event_texts[i]);
//         }
//     }
// }

// static void records_add_event(const char *event_text, lv_color_t color)
// {
//     if (!event_text) {
//         return;
//     }

//     /*
//      * 最新事件放在第 0 行，旧事件往下移动。
//      */
//     for (int i = MAX_EVENT_RECORDS - 1; i > 0; i--) {
//         strncpy(rec_event_texts[i], rec_event_texts[i - 1], EVENT_TEXT_LEN - 1);
//         rec_event_texts[i][EVENT_TEXT_LEN - 1] = '\0';
//     }

//     snprintf(rec_event_texts[0],
//              EVENT_TEXT_LEN,
//              "%s   %s",
//              ui_get_time_text(),
//              event_text);

//     if (rec_event_count < MAX_EVENT_RECORDS) {
//         rec_event_count++;
//     }

//     records_refresh_ui();

//     /*
//      * 最新一条根据事件类型变色，其他保持普通颜色。
//      */
//     for (int i = 0; i < MAX_EVENT_RECORDS; i++) {
//         if (!rec_event_labels[i]) continue;

//         if (i == 0) {
//             lv_obj_set_style_text_color(rec_event_labels[i], color, 0);
//         } else {
//             lv_obj_set_style_text_color(rec_event_labels[i], COLOR_SUB_TEXT, 0);
//         }
//     }
// }

// static void records_update_time_axis(void)
// {
//     if (!rec_time_labels[0]) {
//         return;
//     }

//     time_t now = time(NULL);

//     /*
//      * 这 5 个时间点对应：
//      * 60秒前、45秒前、30秒前、15秒前、当前
//      */
//     int offsets[REC_TIME_LABELS] = {
//         -60, -45, -30, -15, 0
//     };

//     char buf[REC_TIME_LABELS][16];

//     for (int i = 0; i < REC_TIME_LABELS; i++) {
//         time_t t = now + offsets[i];
//         struct tm *tm_info = localtime(&t);

//         if (tm_info) {
//             snprintf(buf[i],
//                      sizeof(buf[i]),
//                      "%02d:%02d:%02d",
//                      tm_info->tm_hour,
//                      tm_info->tm_min,
//                      tm_info->tm_sec);
//         } else {
//             snprintf(buf[i], sizeof(buf[i]), "--:--:--");
//         }

//         lv_label_set_text(rec_time_labels[i], buf[i]);
//     }
// }

// static const char *history_event_pretty_name(const char *event)
// {
//     if (!event) return "UNKNOWN";

//     if (strcmp(event, "HEAD_DOWN") == 0) {
//         return "Head Down";
//     } else if (strcmp(event, "PHONE_CALL") == 0) {
//         return "Phone Call";
//     } else if (strcmp(event, "MOUTH_RISK") == 0) {
//         return "Mouth Risk";
//     } else if (strcmp(event, "WHEEL_OFF") == 0) {
//         return "Wheel Off";
//     } else if (strcmp(event, "DANGEROUS") == 0) {
//         return "Dangerous";
//     }

//     return event;
// }

// static void format_history_csv_line(const char *csv_line, char *out, size_t out_size)
// {
//     if (!csv_line || !out || out_size == 0) {
//         return;
//     }

//     char time_str[32] = {0};
//     char event[32] = {0};
//     char vehicle_mode[32] = {0};

//     int score = 0;
//     int head_down = 0;
//     int phone_call = 0;
//     int mouth_risk = 0;
//     int hands_off = 0;
//     int vehicle_moving = 0;
//     int voice_broadcast = 0;

//     int ret = sscanf(csv_line,
//                      "%31[^,],%31[^,],%d,%d,%d,%d,%d,%31[^,],%d,%d",
//                      time_str,
//                      event,
//                      &score,
//                      &head_down,
//                      &phone_call,
//                      &mouth_risk,
//                      &hands_off,
//                      vehicle_mode,
//                      &vehicle_moving,
//                      &voice_broadcast);

//     if (ret < 10) {
//         snprintf(out, out_size, "%s", csv_line);
//         out[out_size - 1] = '\0';

//         size_t len = strlen(out);
//         if (len > 0 && (out[len - 1] == '\n' || out[len - 1] == '\r')) {
//             out[len - 1] = '\0';
//         }
//         return;
//     }

//     /*
//      * time_str 格式一般是：
//      * 2026-06-03 23:04:46
//      * UI 里只显示 HH:MM:SS，更清爽。
//      */
//     const char *clock_str = time_str;
//     if (strlen(time_str) >= 19) {
//         clock_str = time_str + 11;
//     }

//     snprintf(out,
//              out_size,
//              "%s  %-10s  S:%3d  %-7s  %s  Voice:%s",
//              clock_str,
//              history_event_pretty_name(event),
//              score,
//              vehicle_mode,
//              vehicle_moving ? "MOVING" : "PARKED",
//              voice_broadcast ? "ON" : "OFF");
// }

// static int load_recent_history_lines(char lines[HISTORY_MAX_LINES][HISTORY_LINE_LEN])
// {
//     FILE *fp = fopen(EVENT_LOG_FILE, "r");
//     if (!fp) {
//         return 0;
//     }

//     char ring[HISTORY_MAX_LINES][HISTORY_LINE_LEN];
//     int total = 0;

//     char buf[HISTORY_LINE_LEN];

//     while (fgets(buf, sizeof(buf), fp)) {
//         /*
//          * 跳过表头。
//          */
//         if (strncmp(buf, "time,event", 10) == 0) {
//             continue;
//         }

//         /*
//          * 跳过空行。
//          */
//         if (strlen(buf) < 5) {
//             continue;
//         }

//         int idx = total % HISTORY_MAX_LINES;
//         snprintf(ring[idx], HISTORY_LINE_LEN, "%s", buf);
//         total++;
//     }

//     fclose(fp);

//     int count = total < HISTORY_MAX_LINES ? total : HISTORY_MAX_LINES;

//     /*
//      * 最新的放在最上面。
//      */
//     for (int i = 0; i < count; i++) {
//         int src_index = (total - 1 - i) % HISTORY_MAX_LINES;
//         if (src_index < 0) {
//             src_index += HISTORY_MAX_LINES;
//         }

//         format_history_csv_line(ring[src_index], lines[i], HISTORY_LINE_LEN);
//     }

//     return count;
// }

// static void close_history_popup_event_cb(lv_event_t *e)
// {
//     (void)e;

//     if (history_popup) {
//         lv_obj_del(history_popup);
//         history_popup = NULL;
//     }
// }

// static void show_history_popup_event_cb(lv_event_t *e)
// {
//     (void)e;

//     if (history_popup) {
//         lv_obj_del(history_popup);
//         history_popup = NULL;
//     }

//     /*
//      * 半透明遮罩层。
//      */
//     history_popup = lv_obj_create(lv_layer_top());
//     lv_obj_set_size(history_popup, 900, 560);
//     lv_obj_set_pos(history_popup, 0, 0);
//     lv_obj_set_style_bg_color(history_popup, lv_color_hex(0x020A12), 0);
//     lv_obj_set_style_bg_opa(history_popup, LV_OPA_80, 0);
//     lv_obj_set_style_border_width(history_popup, 0, 0);
//     lv_obj_clear_flag(history_popup, LV_OBJ_FLAG_SCROLLABLE);

//     /*
//      * 中间历史面板。
//      */
//     lv_obj_t *panel = lv_obj_create(history_popup);
//     lv_obj_set_size(panel, 760, 470);
//     lv_obj_set_pos(panel, 70, 45);
//     lv_obj_set_style_bg_color(panel, lv_color_hex(0x071A2F), 0);
//     lv_obj_set_style_bg_opa(panel, LV_OPA_90, 0);
//     lv_obj_set_style_border_width(panel, 1, 0);
//     lv_obj_set_style_border_color(panel, COLOR_CYAN, 0);
//     lv_obj_set_style_border_opa(panel, LV_OPA_60, 0);
//     lv_obj_set_style_radius(panel, 18, 0);
//     lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);

//     label_create(panel, "EVENT HISTORY / CSV RECENT RECORDS", 26, 18, COLOR_CYAN, FONT_SMALL);

//     /*
//      * Close 按钮。
//      */
//     lv_obj_t *btn_close = lv_btn_create(panel);
//     lv_obj_set_size(btn_close, 88, 34);
//     lv_obj_set_pos(btn_close, 640, 14);
//     lv_obj_set_style_radius(btn_close, 16, 0);
//     lv_obj_set_style_bg_color(btn_close, lv_color_hex(0x102B45), 0);
//     lv_obj_set_style_bg_opa(btn_close, LV_OPA_80, 0);
//     lv_obj_set_style_border_width(btn_close, 1, 0);
//     lv_obj_set_style_border_color(btn_close, COLOR_CYAN, 0);
//     lv_obj_add_event_cb(btn_close, close_history_popup_event_cb, LV_EVENT_CLICKED, NULL);

//     lv_obj_t *close_label = lv_label_create(btn_close);
//     lv_label_set_text(close_label, "Close");
//     lv_obj_set_style_text_color(close_label, COLOR_CYAN, 0);
//     lv_obj_set_style_text_font(close_label, FONT_SMALL, 0);
//     lv_obj_center(close_label);

//     char lines[HISTORY_MAX_LINES][HISTORY_LINE_LEN];
//     int count = load_recent_history_lines(lines);

//     if (count <= 0) {
//         label_create(panel,
//                      "No CSV history found. Trigger an alert first.",
//                      35,
//                      78,
//                      COLOR_SUB_TEXT,
//                      FONT_SMALL);
//         return;
//     }

//     /*
//      * 显示最近 20 条。最新在最上面。
//      */
//     for (int i = 0; i < count; i++) {
//         history_line_labels[i] = label_create(panel,
//                                               lines[i],
//                                               35,
//                                               70 + i * 19,
//                                               (i == 0) ? COLOR_GREEN : COLOR_SUB_TEXT,
//                                               FONT_SMALL);
//     }
// }

// /* =========================================================
//  * AI Copilot helpers
//  * ========================================================= */
// static int read_text_file_ui(const char *path, char *buf, int size)
// {
//     FILE *fp = fopen(path, "r");
//     if (!fp) {
//         return -1;
//     }

//     int n = fread(buf, 1, size - 1, fp);
//     fclose(fp);

//     if (n < 0) {
//         return -1;
//     }

//     buf[n] = '\0';
//     return 0;
// }

// static void danger_flash_timer_cb(lv_timer_t *timer)
// {
//     (void)timer;

//     char alert[64] = {0};
//     char fan_action[64] = {0};

//     if (read_text_file_ui("/home/elf/ai_cloud/runtime/ui_alert",
//                           alert,
//                           sizeof(alert)) == 0) {
//         if (strstr(alert, "RED_FLASH") != NULL && danger_flash_count == 0) {
//             danger_flash_count = 8;

//             FILE *fp = fopen("/home/elf/ai_cloud/runtime/ui_alert", "w");
//             if (fp) {
//                 fprintf(fp, "NONE");
//                 fclose(fp);
//             }
//         }
//     }

//     if (danger_flash_count > 0) {
//         if (!danger_flash_layer) {
//             danger_flash_layer = lv_obj_create(lv_layer_top());
//             lv_obj_set_size(danger_flash_layer, SCREEN_W, SCREEN_H);
//             lv_obj_set_pos(danger_flash_layer, 0, 0);
//             lv_obj_set_style_bg_color(danger_flash_layer, COLOR_RED, 0);
//             lv_obj_set_style_bg_opa(danger_flash_layer, LV_OPA_40, 0);
//             lv_obj_set_style_border_width(danger_flash_layer, 0, 0);
//             lv_obj_clear_flag(danger_flash_layer, LV_OBJ_FLAG_SCROLLABLE);
//             lv_obj_clear_flag(danger_flash_layer, LV_OBJ_FLAG_CLICKABLE);

//             lv_obj_t *label = lv_label_create(danger_flash_layer);
//             lv_label_set_text(label, "HIGH RISK WARNING");
//             lv_obj_set_style_text_color(label, COLOR_TEXT, 0);
//             lv_obj_set_style_text_font(label, FONT_LARGE, 0);
//             lv_obj_center(label);
//         }

//         if (danger_flash_count % 2 == 0) {
//             lv_obj_set_style_bg_opa(danger_flash_layer, LV_OPA_60, 0);
//         } else {
//             lv_obj_set_style_bg_opa(danger_flash_layer, LV_OPA_20, 0);
//         }

//         danger_flash_count--;

//         if (danger_flash_count == 0 && danger_flash_layer) {
//             lv_obj_del(danger_flash_layer);
//             danger_flash_layer = NULL;
//         }
//     }

//     if (read_text_file_ui("/home/elf/ai_cloud/runtime/fan_action",
//                           fan_action,
//                           sizeof(fan_action)) == 0) {
//     if (strstr(fan_action, "FAN_BOOST") != NULL) {
//         fan_boost_active = 1;
//         normal_behavior_count = 0;

//         if (g_fan_ctrl) {
//             g_fan_ctrl->mode = 2;
//             g_fan_ctrl->manual_speed = 100;
//             ui_update_fan_mode("AI Boost");
//             ui_update_fan_speed(100);
//         }

//         /*
//         * fan_action 是一次性命令，UI 消费后可以清成 NONE。
//         * fan_state 是真实风扇状态，必须保持 BOOST，给 Dashboard 同步。
//         */
//         FILE *fp_state = fopen("/home/elf/ai_cloud/runtime/fan_state", "w");
//         if (fp_state) {
//             fprintf(fp_state, "BOOST");
//             fclose(fp_state);
//         }

//         FILE *fp = fopen("/home/elf/ai_cloud/runtime/fan_action", "w");
//         if (fp) {
//             fprintf(fp, "NONE");
//             fclose(fp);
//         }
//     }
//     }

//     /*
//  * 如果AI风扇强干预已经开启，并且风险恢复正常一段时间，
//  * 自动把风扇恢复到正常模式。
//  */
// if (fan_boost_active) {
//     char level_buf[32] = {0};
//     char score_buf[32] = {0};

//     int level = 0;
//     int score = 0;

//     if (read_text_file_ui("/home/elf/ai_cloud/runtime/risk_level",
//                           level_buf,
//                           sizeof(level_buf)) == 0) {
//         level = atoi(level_buf);
//     }

//     if (read_text_file_ui("/home/elf/ai_cloud/runtime/cognitive_risk_score",
//                           score_buf,
//                           sizeof(score_buf)) == 0) {
//         score = atoi(score_buf);
//     }

//     /*
//      * 这里的timer周期如果是180ms，
//      * normal_behavior_count > 40 大概是7秒。
//      * 你也可以改成60，大概10秒。
//      */
//     if (level == 0 && score < 4) {
//         normal_behavior_count++;
//     } else {
//         normal_behavior_count = 0;
//     }

//     if (normal_behavior_count > 40) {
//         fan_boost_active = 0;
//         normal_behavior_count = 0;

//         if (g_fan_ctrl) {
//             g_fan_ctrl->mode = 0;
//             g_fan_ctrl->manual_speed = 40;
//             ui_update_fan_mode("Auto");
//             ui_update_fan_speed(40);
//         }

//         FILE *fp_action = fopen("/home/elf/ai_cloud/runtime/fan_action", "w");
//         if (fp_action) {
//             fprintf(fp_action, "NORMAL");
//             fclose(fp_action);
//         }

//         FILE *fp_state = fopen("/home/elf/ai_cloud/runtime/fan_state", "w");
//         if (fp_state) {
//             fprintf(fp_state, "NORMAL");
//             fclose(fp_state);
//         }

//         FILE *fp = fopen("/home/elf/ai_cloud/runtime/ai_action_status", "w");
//         if (fp) {
//             fprintf(fp,
//                     "Driver behavior recovered | Fan:NORMAL | Feedback:RECOVERED");
//             fclose(fp);
//         }
//     }
// }

// }

// static void ai_auto_popup_timer_cb(lv_timer_t *timer)
// {
//     (void)timer;

//     char level_buf[32] = {0};

//     if (read_text_file_ui("/home/elf/ai_cloud/runtime/risk_level",
//                           level_buf,
//                           sizeof(level_buf)) != 0) {
//         return;
//     }

//     int level = atoi(level_buf);

//     /*
//      * Level 1 / Level 2 不自动弹窗。
//      * 只有 Level 3 才自动弹出 AI 认知干预窗口。
//      */
//     if (level < 3) {
//         return;
//     }

//     uint32_t now = lv_tick_get();

//     /*
//      * 关键修正：
//      * last_auto_popup_tick == 0 表示从未自动弹过，
//      * 第一次 Level 3 必须允许弹出。
//      *
//      * 已经弹过后，120秒内不再自动弹。
//      */
//     if (last_auto_popup_tick != 0 &&
//         (now - last_auto_popup_tick) < 120000) {
//         return;
//     }

//     last_auto_popup_level = level;
//     last_auto_popup_tick = now;

//     /*
//      * 标记：这是系统自动弹出的 Level 3 AI 干预窗口。
//      * 只有这种弹窗关闭后，才允许恢复风扇。
//      */
//     ai_popup_auto_opened = 1;

//     if (!ai_popup) {
//         show_ai_copilot_popup_event_cb(NULL);
//     } else {
//         ai_result_timer_cb(NULL);
//     }
// }

// static void close_ai_popup_event_cb(lv_event_t *e)
// {
//     (void)e;

//     /*
//      * 关闭弹窗后进入2分钟冷却。
//      * 不管手动还是自动，都避免马上又弹。
//      */
//     last_auto_popup_tick = lv_tick_get();

//     /*
//      * 关键：
//      * 只有系统自动 Level 3 弹出的 AI 干预窗口，
//      * 关闭时才认为驾驶员确认风险，并恢复风扇。
//      *
//      * 用户手动打开 AI Copilot 查看分析，关闭时不改变风扇状态。
//      */
//     if (ai_popup_auto_opened) {
//         fan_boost_active = 0;
//         normal_behavior_count = 0;

//         if (g_fan_ctrl) {
//             g_fan_ctrl->mode = 0;
//             g_fan_ctrl->manual_speed = 40;
//             ui_update_fan_mode("Auto");
//             ui_update_fan_speed(40);
//         }

//         FILE *fp = fopen("/home/elf/ai_cloud/runtime/fan_action", "w");
//         if (fp) {
//             fprintf(fp, "NORMAL");
//             fclose(fp);
//         }

//         FILE *fp2 = fopen("/home/elf/ai_cloud/runtime/ai_action_status", "w");
//         if (fp2) {
//             fprintf(fp2,
//                     "Auto AI intervention acknowledged | Fan:NORMAL | Feedback:RECOVERED");
//             fclose(fp2);
//         }
//     }

//     ai_popup_auto_opened = 0;

//     if (ai_popup) {
//         lv_obj_del(ai_popup);
//         ai_popup = NULL;
//         ai_answer_box = NULL;
//         ai_answer_label = NULL;
//         ai_status_label = NULL;
//     }
// }

// static void ai_result_timer_cb(lv_timer_t *timer)
// {
//     (void)timer;

//     if (!ai_popup || !ai_answer_label) {
//         return;
//     }

//     char answer[2048] = {0};
//     char status[64] = {0};
//     char latency[64] = {0};
//     char policy_status[64] = {0};

//     char action_status[256] = {0};
//     char risk_level[32] = {0};
//     char cognitive_score[32] = {0};

//     char display[4300] = {0};

//     if (read_text_file_ui("/home/elf/ai_cloud/runtime/llm_answer",
//                           answer,
//                           sizeof(answer)) != 0) {
//         return;
//     }

//     read_text_file_ui("/home/elf/ai_cloud/runtime/cloud_status",
//                       status,
//                       sizeof(status));

//     read_text_file_ui("/home/elf/ai_cloud/runtime/cloud_latency",
//                       latency,
//                       sizeof(latency));

//     read_text_file_ui("/home/elf/ai_cloud/runtime/ai_policy_status",
//                       policy_status,
//                       sizeof(policy_status));

//     read_text_file_ui("/home/elf/ai_cloud/runtime/ai_action_status",
//                   action_status,
//                   sizeof(action_status));

//     read_text_file_ui("/home/elf/ai_cloud/runtime/risk_level",
//                     risk_level,
//                     sizeof(risk_level));

//     read_text_file_ui("/home/elf/ai_cloud/runtime/cognitive_risk_score",
//                     cognitive_score,
//                     sizeof(cognitive_score));

//     if (status[0] == '\0') {
//         snprintf(status, sizeof(status), "--");
//     }

//     if (latency[0] == '\0') {
//         snprintf(latency, sizeof(latency), "--");
//     }

//     if (policy_status[0] == '\0') {
//         snprintf(policy_status, sizeof(policy_status), "DEFAULT_POLICY");
//     }

//     if (ai_status_label) {
//         char header[256];

//         snprintf(header,
//                  sizeof(header),
//                  "Cloud AI: %s    Latency: %s    Policy: %s",
//                  status,
//                  latency,
//                  policy_status);

//         lv_label_set_text(ai_status_label, header);

//         if (strcmp(policy_status, "AI_POLICY_ACTIVE") == 0) {
//             lv_obj_set_style_text_color(ai_status_label, COLOR_GREEN, 0);
//         } else if (strcmp(status, "OFFLINE") == 0) {
//             lv_obj_set_style_text_color(ai_status_label, COLOR_YELLOW, 0);
//         } else if (strcmp(status, "BUSY") == 0) {
//             lv_obj_set_style_text_color(ai_status_label, COLOR_CYAN, 0);
//         } else if (strcmp(status, "ONLINE") == 0) {
//             lv_obj_set_style_text_color(ai_status_label, COLOR_GREEN, 0);
//         } else {
//             lv_obj_set_style_text_color(ai_status_label, COLOR_SUB_TEXT, 0);
//         }
//     }

//     // snprintf(display, sizeof(display), "%s", answer);

//     // /*
//     // * 只有内容变化时才刷新 label。
//     // * 否则用户正在滑动时，定时器不要打断滚动位置。
//     // */
//     // if (strcmp(display, ai_last_answer_cache) != 0) {
//     //     snprintf(ai_last_answer_cache, sizeof(ai_last_answer_cache), "%s", display);

//     //     lv_label_set_text(ai_answer_label, display);

//     //     if (ai_answer_box) {
//     //         lv_obj_scroll_to_y(ai_answer_box, 0, LV_ANIM_OFF);
//     //     }
//     // }
//     snprintf(display,
//             sizeof(display),
//             "Cognitive Risk Level: %s\n"
//             "Cognitive Score: %s\n"
//             "%s\n\n"
//             "%s",
//             risk_level[0] ? risk_level : "--",
//             cognitive_score[0] ? cognitive_score : "--",
//             action_status[0] ? action_status : "Action: --",
//             answer);

//     lv_label_set_text(ai_answer_label, display);
//     lv_obj_set_style_text_color(ai_answer_label, lv_color_hex(0xD8F3FF), 0);
//     lv_obj_set_style_text_font(ai_answer_label, FONT_SMALL, 0);
// }

// static void ai_start_request(const char *mode)
// {
//     if (!mode) {
//         mode = "summary";
//     }

//     system("date '+AI button clicked at %H:%M:%S' >> /tmp/ai_button_debug.log");

//     system("rm -f /home/elf/ai_cloud/llm_busy");

//     if (strcmp(mode, "why") == 0) {
//         system("echo 'Loading AI POLICY GENERATOR...' > /home/elf/ai_cloud/runtime/llm_answer");
//         system("echo 'WHY clicked' >> /tmp/ai_button_debug.log");
//         system("echo 'BUSY' > /home/elf/ai_cloud/runtime/cloud_status");
//         system("echo '--' > /home/elf/ai_cloud/runtime/cloud_latency");
//         system("nohup /usr/bin/python3 /home/elf/ai_cloud/ask_cloud.py why > /home/elf/ai_cloud/runtime/ask_cloud_ui.log 2>&1 &");
//     }
//     else if (strcmp(mode, "advice") == 0) {
//         system("echo 'Loading SCENARIO PLAN...' > /home/elf/ai_cloud/runtime/llm_answer");
//         system("echo 'ADVICE clicked' >> /tmp/ai_button_debug.log");
//         system("echo 'BUSY' > /home/elf/ai_cloud/runtime/cloud_status");
//         system("echo '--' > /home/elf/ai_cloud/runtime/cloud_latency");
//         system("nohup /usr/bin/python3 /home/elf/ai_cloud/ask_cloud.py advice > /home/elf/ai_cloud/runtime/ask_cloud_ui.log 2>&1 &");
//     }
//     else {
//         system("echo 'Loading AI RISK BRAIN...' > /home/elf/ai_cloud/runtime/llm_answer");
//         system("echo 'SUMMARY clicked' >> /tmp/ai_button_debug.log");
//         system("echo 'BUSY' > /home/elf/ai_cloud/runtime/cloud_status");
//         system("echo '--' > /home/elf/ai_cloud/runtime/cloud_latency");
//         system("nohup /usr/bin/python3 /home/elf/ai_cloud/ask_cloud.py summary > /home/elf/ai_cloud/runtime/ask_cloud_ui.log 2>&1 &");
//     }
// }

// static void ai_mode_btn_event_cb(lv_event_t *e)
// {
//     const char *mode = (const char *)lv_event_get_user_data(e);
//     ai_start_request(mode);
// }

// static lv_obj_t *ai_small_btn_create(lv_obj_t *parent,
//                                      const char *txt,
//                                      int x,
//                                      int y,
//                                      const char *mode)
// {
//     lv_obj_t *btn = lv_btn_create(parent);
//     lv_obj_set_size(btn, 100, 34);
//     lv_obj_set_pos(btn, x, y);

//     lv_obj_set_style_radius(btn, 16, 0);
//     lv_obj_set_style_bg_color(btn, lv_color_hex(0x102B45), 0);
//     lv_obj_set_style_bg_opa(btn, LV_OPA_80, 0);
//     lv_obj_set_style_border_width(btn, 1, 0);
//     lv_obj_set_style_border_color(btn, COLOR_CYAN, 0);

//     lv_obj_t *label = lv_label_create(btn);
//     lv_label_set_text(label, txt);
//     lv_obj_set_style_text_color(label, COLOR_CYAN, 0);
//     lv_obj_set_style_text_font(label, FONT_SMALL, 0);
//     lv_obj_center(label);

//     lv_obj_add_event_cb(btn, ai_mode_btn_event_cb, LV_EVENT_CLICKED, (void *)mode);

//     return btn;
// }

// static void show_ai_copilot_popup_event_cb(lv_event_t *e)
// {
//     (void)e;

//     /*
//      * e != NULL 表示用户点击按钮手动打开。
//      * 自动弹窗会传 NULL，但自动弹窗函数里会提前设置 ai_popup_auto_opened = 1。
//      */
//     if (e != NULL) {
//         ai_popup_auto_opened = 0;
//     }

//     if (ai_popup) {
//         lv_obj_del(ai_popup);
//         ai_popup = NULL;
//         ai_answer_box = NULL;
//         ai_answer_label = NULL;
//         ai_status_label = NULL;
//     }
//     ai_last_answer_cache[0] = '\0';

//     /*
//      * 半透明遮罩层
//      */
//     ai_popup = lv_obj_create(lv_layer_top());
//     lv_obj_set_size(ai_popup, 900, 560);
//     lv_obj_set_pos(ai_popup, 0, 0);
//     lv_obj_set_style_bg_color(ai_popup, lv_color_hex(0x020A12), 0);
//     lv_obj_set_style_bg_opa(ai_popup, LV_OPA_80, 0);
//     lv_obj_set_style_border_width(ai_popup, 0, 0);
//     lv_obj_clear_flag(ai_popup, LV_OBJ_FLAG_SCROLLABLE);

//     /*
//      * 主面板
//      */
//     lv_obj_t *panel = lv_obj_create(ai_popup);
//     lv_obj_set_size(panel, 790, 490);
//     lv_obj_set_pos(panel, 55, 35);
//     lv_obj_set_style_bg_color(panel, lv_color_hex(0x071A2F), 0);
//     lv_obj_set_style_bg_opa(panel, LV_OPA_90, 0);
//     lv_obj_set_style_border_width(panel, 1, 0);
//     lv_obj_set_style_border_color(panel, COLOR_CYAN, 0);
//     lv_obj_set_style_border_opa(panel, LV_OPA_70, 0);
//     lv_obj_set_style_radius(panel, 18, 0);
//     lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);

//     label_create(panel,
//                  "AI COPILOT / EDGE-CLOUD RISK ANALYSIS",
//                  26,
//                  18,
//                  COLOR_CYAN,
//                  FONT_SMALL);

//     ai_status_label = label_create(panel,
//                                    "Cloud AI: --    Latency: --",
//                                    26,
//                                    48,
//                                    COLOR_SUB_TEXT,
//                                    FONT_SMALL);

//     // ai_policy_label = label_create(panel,
//     //                            "DEFAULT_POLICY",
//     //                            520,
//     //                            48,
//     //                            COLOR_SUB_TEXT,
//     //                            FONT_SMALL);                               

//     /*
//      * 结果显示区域
//      */
//     ai_answer_box = lv_obj_create(panel);
//     lv_obj_set_size(ai_answer_box, 730, 315);
//     lv_obj_set_pos(ai_answer_box, 28, 84);
//     lv_obj_set_style_bg_color(ai_answer_box, lv_color_hex(0x020617), 0);
//     lv_obj_set_style_bg_opa(ai_answer_box, LV_OPA_80, 0);
//     lv_obj_set_style_border_width(ai_answer_box, 1, 0);
//     lv_obj_set_style_border_color(ai_answer_box, COLOR_BORDER_DIM, 0);
//     lv_obj_set_style_radius(ai_answer_box, 12, 0);
//     lv_obj_set_style_pad_all(ai_answer_box, 10, 0);

//     /* 关键：开启垂直滚动 */
//     lv_obj_add_flag(ai_answer_box, LV_OBJ_FLAG_SCROLLABLE);
//     lv_obj_set_scroll_dir(ai_answer_box, LV_DIR_VER);
//     lv_obj_set_scrollbar_mode(ai_answer_box, LV_SCROLLBAR_MODE_AUTO);

//     /* 可选：滚动条样式更明显一点 */
//     lv_obj_set_style_bg_opa(ai_answer_box, LV_OPA_80, LV_PART_MAIN);
//     lv_obj_set_style_width(ai_answer_box, 6, LV_PART_SCROLLBAR);
//     lv_obj_set_style_bg_color(ai_answer_box, COLOR_CYAN, LV_PART_SCROLLBAR);
//     lv_obj_set_style_bg_opa(ai_answer_box, LV_OPA_60, LV_PART_SCROLLBAR);

//     ai_answer_label = lv_label_create(ai_answer_box);
//     lv_label_set_text(ai_answer_label,
//                     "Press Brain / Policy / Plan to start AI Copilot analysis.");
//     lv_obj_set_pos(ai_answer_label, 0, 0);
//     lv_obj_set_width(ai_answer_label, 690);
//     lv_label_set_long_mode(ai_answer_label, LV_LABEL_LONG_WRAP);
//     lv_obj_set_style_text_color(ai_answer_label, COLOR_TEXT, 0);
//     lv_obj_set_style_text_font(ai_answer_label, &lv_font_chinese_18, 0);

//     /*
//      * 底部按钮
//      */
//     ai_small_btn_create(panel, "Brain",  28, 425, "summary");
//     ai_small_btn_create(panel, "Policy", 145, 425, "why");
//     ai_small_btn_create(panel, "Plan",   262, 425, "advice");

//     lv_obj_t *btn_close = lv_btn_create(panel);
//     lv_obj_set_size(btn_close, 100, 34);
//     lv_obj_set_pos(btn_close, 658, 425);
//     lv_obj_set_style_radius(btn_close, 16, 0);
//     lv_obj_set_style_bg_color(btn_close, lv_color_hex(0x102B45), 0);
//     lv_obj_set_style_bg_opa(btn_close, LV_OPA_80, 0);
//     lv_obj_set_style_border_width(btn_close, 1, 0);
//     lv_obj_set_style_border_color(btn_close, COLOR_RED, 0);
//     lv_obj_add_event_cb(btn_close, close_ai_popup_event_cb, LV_EVENT_CLICKED, NULL);

//     lv_obj_t *close_label = lv_label_create(btn_close);
//     lv_label_set_text(close_label, "Close");
//     lv_obj_set_style_text_color(close_label, COLOR_RED, 0);
//     lv_obj_set_style_text_font(close_label, FONT_SMALL, 0);
//     lv_obj_center(close_label);

//     /*
//      * 打开弹窗后先读一次结果。
//      */
//     ai_result_timer_cb(NULL);
// }

// static void create_records_page(lv_obj_t *scr)
// {
//     lv_obj_t *page = lv_obj_create(scr);
//     pages[PAGE_RECORDS] = page;

//     lv_obj_set_size(page, PAGE_W, PAGE_H);
//     lv_obj_set_pos(page, PAGE_X, PAGE_Y);
//     lv_obj_set_style_bg_opa(page, LV_OPA_TRANSP, 0);
//     lv_obj_set_style_border_width(page, 0, 0);
//     lv_obj_set_style_pad_all(page, 0, 0);
//     lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);

//     decorative_curve_lines(page);

//     lv_obj_t *trend = glass_panel_create(page, 0, 0, 840, 260);
//     title_create(trend, "4. RECORDS / FATIGUE SCORE TREND", 24, 14);

//     rec_chart = lv_chart_create(trend);
//     lv_obj_set_size(rec_chart, 760, 175);
//     lv_obj_set_pos(rec_chart, 40, 62);
//     lv_chart_set_type(rec_chart, LV_CHART_TYPE_LINE);
//     lv_chart_set_range(rec_chart, LV_CHART_AXIS_PRIMARY_Y, 0, 100);
//     lv_chart_set_point_count(rec_chart, 60);
//     lv_chart_set_update_mode(rec_chart, LV_CHART_UPDATE_MODE_SHIFT);

//     lv_obj_set_style_bg_color(rec_chart, lv_color_hex(0x061220), 0);
//     lv_obj_set_style_bg_opa(rec_chart, OPACITY_70, 0);
//     lv_obj_set_style_border_width(rec_chart, 0, 0);
//     lv_obj_set_style_line_color(rec_chart, lv_color_hex(0x12344A), LV_PART_MAIN);
//     lv_obj_set_style_size(rec_chart, 0, LV_PART_INDICATOR);

//     rec_score_series = lv_chart_add_series(rec_chart, COLOR_GREEN, LV_CHART_AXIS_PRIMARY_Y);

//     for (int i = 0; i < 60; i++) {
//         lv_chart_set_next_value(rec_chart, rec_score_series, 0);
//     }

//     rec_time_labels[0] = label_create(trend, "--:--:--", 35, 232, COLOR_SUB_TEXT, FONT_SMALL);
//     rec_time_labels[1] = label_create(trend, "--:--:--", 205, 232, COLOR_SUB_TEXT, FONT_SMALL);
//     rec_time_labels[2] = label_create(trend, "--:--:--", 375, 232, COLOR_SUB_TEXT, FONT_SMALL);
//     rec_time_labels[3] = label_create(trend, "--:--:--", 545, 232, COLOR_SUB_TEXT, FONT_SMALL);
//     rec_time_labels[4] = label_create(trend, "--:--:--", 715, 232, COLOR_CYAN, FONT_SMALL);

//     records_update_time_axis();

//     lv_obj_t *events = glass_panel_create(page, 0, 275, 840, 215);
//     title_create(events, "RECENT EVENTS", 24, 14);

//     /* History 按钮：查看 CSV 历史记录 */
//     lv_obj_t *btn_history = lv_btn_create(events);
//     lv_obj_set_size(btn_history, 96, 34);
//     lv_obj_set_pos(btn_history, 710, 12);
//     lv_obj_set_style_radius(btn_history, 16, 0);
//     lv_obj_set_style_bg_color(btn_history, lv_color_hex(0x071A2F), 0);
//     lv_obj_set_style_bg_opa(btn_history, LV_OPA_70, 0);
//     lv_obj_set_style_border_width(btn_history, 1, 0);
//     lv_obj_set_style_border_color(btn_history, COLOR_CYAN, 0);
//     lv_obj_set_style_border_opa(btn_history, LV_OPA_70, 0);
//     lv_obj_add_event_cb(btn_history, show_history_popup_event_cb, LV_EVENT_CLICKED, NULL);

//     lv_obj_t *history_label = lv_label_create(btn_history);
//     lv_label_set_text(history_label, "History");
//     lv_obj_set_style_text_color(history_label, COLOR_CYAN, 0);
//     lv_obj_set_style_text_font(history_label, FONT_SMALL, 0);
//     lv_obj_center(history_label);

//         /*
//      * AI Copilot 按钮：打开端云协同风险研判弹窗。
//      */
//     lv_obj_t *btn_ai = lv_btn_create(events);
//     lv_obj_set_size(btn_ai, 110, 34);
//     lv_obj_set_pos(btn_ai, 580, 14);
//     lv_obj_set_style_radius(btn_ai, 16, 0);
//     lv_obj_set_style_bg_color(btn_ai, lv_color_hex(0x102B45), 0);
//     lv_obj_set_style_bg_opa(btn_ai, LV_OPA_80, 0);
//     lv_obj_set_style_border_width(btn_ai, 1, 0);
//     lv_obj_set_style_border_color(btn_ai, COLOR_GREEN, 0);
//     lv_obj_set_style_border_opa(btn_ai, LV_OPA_70, 0);
//     lv_obj_add_event_cb(btn_ai, show_ai_copilot_popup_event_cb, LV_EVENT_CLICKED, NULL);

//     lv_obj_t *ai_label = lv_label_create(btn_ai);
//     lv_label_set_text(ai_label, "AI Copilot");
//     lv_obj_set_style_text_color(ai_label, COLOR_GREEN, 0);
//     lv_obj_set_style_text_font(ai_label, FONT_SMALL, 0);
//     lv_obj_center(ai_label);

//     /*
//      * 初始化事件文本。
//      */
//     snprintf(rec_event_texts[0], EVENT_TEXT_LEN, "--:--:--   System started");
//     snprintf(rec_event_texts[1], EVENT_TEXT_LEN, "--:--:--   Camera and sensors connected");
//     snprintf(rec_event_texts[2], EVENT_TEXT_LEN, "--:--:--   Risk engine active");
//     snprintf(rec_event_texts[3], EVENT_TEXT_LEN, "--:--:--   Waiting for detection events");
//     snprintf(rec_event_texts[4], EVENT_TEXT_LEN, "--:--:--   No abnormal alert");

//     for (int i = 0; i < MAX_EVENT_RECORDS; i++) {
//         rec_event_labels[i] = label_create(events,
//                                            rec_event_texts[i],
//                                            35,
//                                            55 + i * 28,
//                                            (i == 0) ? COLOR_GREEN : COLOR_SUB_TEXT,
//                                            FONT_SMALL);
//     }
// }

// /* =========================================================
//  * Settings page
//  * ========================================================= */
// static void power_btn_event_cb(lv_event_t *e);

// static void voice_switch_event_cb(lv_event_t *e)
// {
//     lv_obj_t *sw = lv_event_get_target(e);
//     bool enabled = lv_obj_has_state(sw, LV_STATE_CHECKED);
//     ui_update_voice_state(enabled);
// }

// static void write_text_file_simple(const char *path, const char *text)
// {
//     FILE *fp = fopen(path, "w");
//     if (!fp) {
//         return;
//     }

//     fprintf(fp, "%s\n", text);
//     fclose(fp);
// }

// static const char *vehicle_mode_to_text(int mode)
// {
//     if (mode == 1) {
//         return "DRIVING";
//     } else if (mode == 2) {
//         return "PARKED";
//     } else {
//         return "AUTO";
//     }
// }

// static int vehicle_mode_from_text(const char *text)
// {
//     if (!text) {
//         return 0;
//     }

//     if (strstr(text, "DRIVING") || strstr(text, "MOVING")) {
//         return 1;
//     }

//     if (strstr(text, "PARKED") || strstr(text, "STOP")) {
//         return 2;
//     }

//     if (strstr(text, "AUTO")) {
//         return 0;
//     }

//     return 0;
// }

// static int read_vehicle_mode_file(void)
// {
//     char buf[64] = {0};

//     /*
//      * 优先读 runtime，因为现在 Dashboard / watchdog 主要靠它同步。
//      */
//     if (read_text_file_ui("/home/elf/ai_cloud/runtime/vehicle_mode",
//                           buf,
//                           sizeof(buf)) == 0) {
//         return vehicle_mode_from_text(buf);
//     }

//     memset(buf, 0, sizeof(buf));
//     if (read_text_file_ui("/tmp/cloud_vehicle_mode",
//                           buf,
//                           sizeof(buf)) == 0) {
//         return vehicle_mode_from_text(buf);
//     }

//     memset(buf, 0, sizeof(buf));
//     if (read_text_file_ui("/tmp/vehicle_mode",
//                           buf,
//                           sizeof(buf)) == 0) {
//         return vehicle_mode_from_text(buf);
//     }

//     return 0;   /* 默认 AUTO */
// }

// static void write_vehicle_mode_file(int mode)
// {
//     const char *mode_text = vehicle_mode_to_text(mode);

//     /*
//      * 推荐优先调用统一脚本。
//      * 这样以后 Dashboard、watchdog、UI 三边都一致。
//      */
//     char cmd[256];
//     snprintf(cmd,
//              sizeof(cmd),
//              "/home/elf/ai_cloud/set_vehicle_mode.sh %s >/tmp/set_vehicle_mode_ui.log 2>&1",
//              mode_text);

//     int ret = system(cmd);

//     /*
//      * 如果脚本不存在或执行失败，直接写三个文件兜底。
//      */
//     if (ret != 0) {
//         write_text_file_simple("/tmp/vehicle_mode", mode_text);
//         write_text_file_simple("/tmp/cloud_vehicle_mode", mode_text);
//         write_text_file_simple("/home/elf/ai_cloud/runtime/vehicle_mode", mode_text);
//         write_text_file_simple("/home/elf/ai_cloud/runtime/cloud_vehicle_mode", mode_text);
//     }
// }

// static void update_vehicle_mode_label(void)
// {
//     if (!setting_vehicle_mode_label) {
//         return;
//     }

//     if (g_vehicle_mode == 1) {
//         lv_label_set_text(setting_vehicle_mode_label, "Driving");
//         lv_obj_set_style_text_color(setting_vehicle_mode_label, COLOR_GREEN, 0);
//     } else if (g_vehicle_mode == 2) {
//         lv_label_set_text(setting_vehicle_mode_label, "Parked");
//         lv_obj_set_style_text_color(setting_vehicle_mode_label, COLOR_YELLOW, 0);
//     } else {
//         lv_label_set_text(setting_vehicle_mode_label, "Auto");
//         lv_obj_set_style_text_color(setting_vehicle_mode_label, COLOR_CYAN, 0);
//     }
// }

// static void vehicle_mode_btn_event_cb(lv_event_t *e)
// {
//     (void)e;

//     g_vehicle_mode++;
//     if (g_vehicle_mode > 2) {
//         g_vehicle_mode = 0;
//     }

//     write_vehicle_mode_file(g_vehicle_mode);
//     update_vehicle_mode_label();
// }

// static float read_cpu_usage_percent(void)
// {
//     static unsigned long long last_total = 0;
//     static unsigned long long last_idle = 0;

//     FILE *fp = fopen("/proc/stat", "r");
//     if (!fp) {
//         return -1.0f;
//     }

//     char cpu[8];
//     unsigned long long user, nice, system, idle;
//     unsigned long long iowait, irq, softirq, steal;

//     int ret = fscanf(fp,
//                      "%7s %llu %llu %llu %llu %llu %llu %llu %llu",
//                      cpu,
//                      &user,
//                      &nice,
//                      &system,
//                      &idle,
//                      &iowait,
//                      &irq,
//                      &softirq,
//                      &steal);

//     fclose(fp);

//     if (ret < 8) {
//         return -1.0f;
//     }

//     unsigned long long idle_all = idle + iowait;
//     unsigned long long non_idle = user + nice + system + irq + softirq + steal;
//     unsigned long long total = idle_all + non_idle;

//     if (last_total == 0) {
//         last_total = total;
//         last_idle = idle_all;
//         return 0.0f;
//     }

//     unsigned long long total_delta = total - last_total;
//     unsigned long long idle_delta = idle_all - last_idle;

//     last_total = total;
//     last_idle = idle_all;

//     if (total_delta == 0) {
//         return 0.0f;
//     }

//     return (float)(total_delta - idle_delta) * 100.0f / (float)total_delta;
// }

// static void metrics_timer_cb(lv_timer_t *timer)
// {
//     (void)timer;

//     static uint32_t last_tick = 0;
//     static int last_camera_frame = -1;
//     static int last_preview_frame = 0;

//     uint32_t now = lv_tick_get();

//     if (metric_start_tick == 0) {
//         metric_start_tick = now;
//     }

//     if (last_tick == 0) {
//         last_tick = now;

//         if (g_camera_frame) {
//             last_camera_frame = g_camera_frame->frame_count;
//         }

//         last_preview_frame = camera_preview_frame_count;
//         return;
//     }

//     uint32_t dt = now - last_tick;
//     if (dt < 900) {
//         return;
//     }

//     float sec = (float)dt / 1000.0f;

//     float camera_fps = 0.0f;
//     float preview_fps = 0.0f;

//     if (g_camera_frame && last_camera_frame >= 0) {
//         int cur = g_camera_frame->frame_count;
//         camera_fps = (float)(cur - last_camera_frame) / sec;
//         last_camera_frame = cur;
//     }

//     preview_fps = (float)(camera_preview_frame_count - last_preview_frame) / sec;
//     last_preview_frame = camera_preview_frame_count;

//     last_tick = now;

//     char buf[64];

//     /* Runtime */
//     uint32_t runtime_s = (now - metric_start_tick) / 1000;
//     uint32_t h = runtime_s / 3600;
//     uint32_t m = (runtime_s % 3600) / 60;
//     uint32_t s = runtime_s % 60;

//     snprintf(buf, sizeof(buf), "%02u:%02u:%02u", h, m, s);
//     if (metric_runtime_label) {
//         lv_label_set_text(metric_runtime_label, buf);
//     }

//     /* Camera FPS: 采集共享内存帧率 */
//     snprintf(buf, sizeof(buf), "%.1f FPS", camera_fps);
//     if (metric_camera_fps_label) {
//         lv_label_set_text(metric_camera_fps_label, buf);
//     }

//     /*
//      * NPU FPS:
//      * 推荐显示 pose_infer_fps，因为它最接近你日志里的 rknn_run FPS。
//      * 如果你想显示双模型完整链路，就把 pose_infer_fps 改成 ai_infer_fps。
//      */
//     float npu_fps = 0.0f;
//     float infer_ms = 0.0f;

//     if (g_behavior_result) {
//         npu_fps = g_behavior_result->pose_infer_fps;
//         infer_ms = g_behavior_result->pose_infer_ms;

//         /*
//          * 如果 pose_infer_fps 没写入，就兜底用 ai_infer_fps。
//          */
//         if (npu_fps <= 0.1f) {
//             npu_fps = g_behavior_result->ai_infer_fps;
//             infer_ms = g_behavior_result->ai_infer_ms;
//         }
//     }

//     if (npu_fps > 0.1f) {
//         snprintf(buf, sizeof(buf), "%.1f FPS", npu_fps);
//     } else {
//         snprintf(buf, sizeof(buf), "-- FPS");
//     }

//     if (metric_ai_fps_label) {
//         lv_label_set_text(metric_ai_fps_label, buf);

//         if (npu_fps >= 30.0f) {
//             lv_obj_set_style_text_color(metric_ai_fps_label, COLOR_GREEN, 0);
//         } else if (npu_fps >= 15.0f) {
//             lv_obj_set_style_text_color(metric_ai_fps_label, COLOR_YELLOW, 0);
//         } else {
//             lv_obj_set_style_text_color(metric_ai_fps_label, COLOR_RED, 0);
//         }
//     }

//     if (infer_ms > 0.1f) {
//         snprintf(buf, sizeof(buf), "%.2f ms", infer_ms);
//     } else {
//         snprintf(buf, sizeof(buf), "-- ms");
//     }

//     if (metric_infer_ms_label) {
//         lv_label_set_text(metric_infer_ms_label, buf);
//     }

//     /* Preview FPS: LVGL 实际预览刷新率 */
//     snprintf(buf, sizeof(buf), "%.1f FPS", preview_fps);
//     if (metric_preview_fps_label) {
//         lv_label_set_text(metric_preview_fps_label, buf);
//     }

//     /* CPU Usage */
//     float cpu = read_cpu_usage_percent();

//     if (cpu >= 0.0f) {
//         snprintf(buf, sizeof(buf), "%.0f%%", cpu);
//     } else {
//         snprintf(buf, sizeof(buf), "--%%");
//     }

//     if (metric_cpu_label) {
//         lv_label_set_text(metric_cpu_label, buf);

//         if (cpu >= 80.0f) {
//             lv_obj_set_style_text_color(metric_cpu_label, COLOR_RED, 0);
//         } else if (cpu >= 55.0f) {
//             lv_obj_set_style_text_color(metric_cpu_label, COLOR_YELLOW, 0);
//         } else {
//             lv_obj_set_style_text_color(metric_cpu_label, COLOR_GREEN, 0);
//         }
//     }

//     if (metric_stability_label) {
//         lv_label_set_text(metric_stability_label, "Stable");
//         lv_obj_set_style_text_color(metric_stability_label, COLOR_GREEN, 0);
//     }
// }

// // static void restart_app_btn_event_cb(lv_event_t *e)
// // {
// //     lv_obj_t *btn = lv_event_get_target(e);

// //     if (btn) {
// //         lv_obj_t *label = lv_obj_get_child(btn, 3);  /* 你的 title_label 通常是第 3 个 child */
// //         if (label) {
// //             lv_label_set_text(label, "RESTARTING");
// //         }
// //         lv_obj_set_style_bg_color(btn, lv_color_hex(0x1A4A6A), 0);
// //         lv_obj_invalidate(btn);
// //     }

// //     /*
// //      * 关键：
// //      * 不要在当前服务 cgroup 里直接 systemctl restart 自己。
// //      * 用 systemd-run 创建一个临时独立任务，延迟 1 秒后重启 smart_cockpit。
// //      */
// //     (void)system(
// //         "systemd-run --unit=smart-cockpit-restart --collect "
// //         "/bin/bash -lc 'sleep 1; systemctl restart smart_cockpit.service' &"
// //     );
// // }

// // static void shutdown_system_btn_event_cb(lv_event_t *e)
// // {
// //     lv_obj_t *btn = lv_event_get_target(e);

// //     if (btn) {
// //         lv_obj_t *label = lv_obj_get_child(btn, 3);
// //         if (label) {
// //             lv_label_set_text(label, "POWERING OFF");
// //         }
// //         lv_obj_set_style_bg_color(btn, lv_color_hex(0x5A1018), 0);
// //         lv_obj_invalidate(btn);
// //     }

// //     /*
// //      * 同理，关机也放到独立 systemd-run 临时任务里。
// //      * 否则 stop 当前服务时可能先把 UI 自己杀掉，poweroff 没执行到。
// //      */
// //     (void)system(
// //         "systemd-run --unit=smart-cockpit-poweroff --collect "
// //         "/bin/bash -lc 'sleep 1; poweroff' &"
// //     );
// // }

// static void restart_app_btn_event_cb(lv_event_t *e)
// {
//     lv_obj_t *btn = lv_event_get_target(e);

//     if (btn) {
//         lv_obj_set_style_bg_color(btn, lv_color_hex(0x1A4A6A), 0);
//         lv_obj_invalidate(btn);
//     }

//     /*
//      * Use systemd-run to restart from outside current UI process.
//      */
//     (void)system(
//         "systemd-run --unit=smart-cockpit-restart --collect "
//         "/bin/bash -lc 'sleep 1; systemctl restart smart_cockpit.service' &"
//     );
// }

// static void power_main_btn_event_cb(lv_event_t *e)
// {
//     lv_event_code_t code = lv_event_get_code(e);
//     lv_obj_t *btn = lv_event_get_target(e);

//     if (code == LV_EVENT_PRESSED) {
//         power_press_tick = lv_tick_get();
//         power_shutdown_dialog_shown = 0;

//         if (btn) {
//             lv_obj_set_style_bg_color(btn, lv_color_hex(0x0B2E45), 0);
//             lv_obj_set_style_shadow_opa(btn, LV_OPA_60, 0);
//             lv_obj_invalidate(btn);
//         }
//     }
//     else if (code == LV_EVENT_PRESSING) {
//         uint32_t held = lv_tick_elaps(power_press_tick);

//         if (btn) {
//             if (held >= POWER_SHUTDOWN_HOLD_MS) {
//                 lv_obj_set_style_border_color(btn, COLOR_RED, 0);
//                 lv_obj_set_style_shadow_color(btn, COLOR_RED, 0);
//                 lv_obj_set_style_bg_color(btn, lv_color_hex(0x4A1018), 0);
//             } else if (held >= POWER_RESTART_HOLD_MS) {
//                 lv_obj_set_style_border_color(btn, COLOR_CYAN, 0);
//                 lv_obj_set_style_shadow_color(btn, COLOR_CYAN, 0);
//                 lv_obj_set_style_bg_color(btn, lv_color_hex(0x123A5A), 0);
//             }

//             lv_obj_invalidate(btn);
//         }

//         /*
//          * 长按达到关机阈值后，自动弹出确认框。
//          * 用 power_shutdown_dialog_shown 防止一直重复创建弹窗。
//          */
//         if (held >= POWER_SHUTDOWN_HOLD_MS && !power_shutdown_dialog_shown) {
//             power_shutdown_dialog_shown = 1;
//             show_shutdown_confirm_dialog();
//         }
//     }
//     else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
//         uint32_t held = lv_tick_elaps(power_press_tick);

//         if (btn) {
//             lv_obj_set_style_bg_color(btn, lv_color_hex(0x081A2D), 0);
//             lv_obj_set_style_border_color(btn, COLOR_CYAN, 0);
//             lv_obj_set_style_shadow_color(btn, COLOR_CYAN, 0);
//             lv_obj_set_style_shadow_opa(btn, LV_OPA_40, 0);
//             lv_obj_invalidate(btn);
//         }

//         /*
//          * 如果已经弹出关机确认框，松手时不要再执行重启。
//          */
//         if (!power_shutdown_dialog_shown && held >= POWER_RESTART_HOLD_MS) {
//             int ret = system(
//                 "systemd-run --unit=smart-cockpit-restart-$(date +%s) --collect "
//                 "/bin/bash -lc 'sleep 0.3; systemctl restart --no-block smart_cockpit.service' &"
//             );
//             (void)ret;
//         }

//         power_press_tick = 0;
//     }
// }

// static lv_obj_t *power_tile_create(lv_obj_t *parent,
//                                    int x, int y,
//                                    int w, int h,
//                                    const char *title,
//                                    const char *sub,
//                                    lv_color_t main_color,
//                                    lv_event_cb_t cb)
// {
//     (void)sub;

//     lv_obj_t *tile = lv_btn_create(parent);
//     lv_obj_set_size(tile, w, h);
//     lv_obj_set_pos(tile, x, y);
//     lv_obj_clear_flag(tile, LV_OBJ_FLAG_SCROLLABLE);

//     /* 整体卡片 */
//     lv_obj_set_style_radius(tile, 22, 0);
//     lv_obj_set_style_bg_color(tile, lv_color_hex(0x081A2D), 0);
//     lv_obj_set_style_bg_opa(tile, LV_OPA_60, 0);

//     lv_obj_set_style_border_width(tile, 2, 0);
//     lv_obj_set_style_border_color(tile, main_color, 0);
//     lv_obj_set_style_border_opa(tile, LV_OPA_80, 0);

//     lv_obj_set_style_shadow_width(tile, 24, 0);
//     lv_obj_set_style_shadow_color(tile, main_color, 0);
//     lv_obj_set_style_shadow_opa(tile, LV_OPA_40, 0);
//     lv_obj_set_style_shadow_spread(tile, 1, 0);

//     lv_obj_set_style_pad_all(tile, 0, 0);

//     /* 按下反馈 */
//     lv_obj_set_style_bg_color(tile, lv_color_hex(0x10304A), LV_STATE_PRESSED);
//     lv_obj_set_style_bg_opa(tile, LV_OPA_80, LV_STATE_PRESSED);
//     lv_obj_set_style_shadow_opa(tile, LV_OPA_50, LV_STATE_PRESSED);

//     if (cb) {
//         lv_obj_add_event_cb(tile, cb, LV_EVENT_ALL, NULL);
//     }

//     /* 只保留电源图标，不要圆圈，不要椭圆底 */
//     lv_obj_t *icon = lv_label_create(tile);
//     lv_label_set_text(icon, LV_SYMBOL_POWER);
//     lv_obj_set_style_text_color(icon, main_color, 0);
//     lv_obj_set_style_text_font(icon, &lv_font_montserrat_28, 0);
//     lv_obj_align(icon, LV_ALIGN_TOP_MID, 0, 16);
//     lv_obj_clear_flag(icon, LV_OBJ_FLAG_CLICKABLE);

//     /* 给图标加一点文字阴影，增强发光感 */
//     lv_obj_set_style_text_opa(icon, LV_OPA_COVER, 0);

//     /* POWER 字样，缩小 */
//     lv_obj_t *label = lv_label_create(tile);
//     lv_label_set_text(label, title ? title : "POWER");
//     lv_obj_set_style_text_color(label, main_color, 0);
//     lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);
//     lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, -14);
//     lv_obj_clear_flag(label, LV_OBJ_FLAG_CLICKABLE);

//     return tile;
// }

// static void shutdown_confirm_cancel_cb(lv_event_t *e)
// {
//     (void)e;

//     if (shutdown_confirm_mask) {
//         lv_obj_del(shutdown_confirm_mask);
//         shutdown_confirm_mask = NULL;
//     }

//     shutdown_confirm_box = NULL;
//     power_shutdown_dialog_shown = 0;
// }

// static void shutdown_confirm_yes_cb(lv_event_t *e)
// {
//     (void)e;

//     if (shutdown_confirm_mask) {
//         lv_obj_del(shutdown_confirm_mask);
//         shutdown_confirm_mask = NULL;
//     }

//     shutdown_confirm_box = NULL;
//     power_shutdown_dialog_shown = 0;

//     int ret = system(
//         "systemd-run --unit=smart-cockpit-poweroff-$(date +%s) --collect "
//         "/bin/bash -lc 'sleep 1; poweroff' &"
//     );
//     (void)ret;
// }

// static void show_shutdown_confirm_dialog(void)
// {
//     if (shutdown_confirm_mask) {
//         return;   /* 已经弹出时不重复创建 */
//     }

//     /* 半透明遮罩层 */
//     shutdown_confirm_mask = lv_obj_create(lv_scr_act());
//     lv_obj_remove_style_all(shutdown_confirm_mask);
//     lv_obj_set_size(shutdown_confirm_mask, LV_HOR_RES, LV_VER_RES);
//     lv_obj_set_style_bg_color(shutdown_confirm_mask, lv_color_hex(0x000000), 0);
//     lv_obj_set_style_bg_opa(shutdown_confirm_mask, LV_OPA_40, 0);
//     lv_obj_clear_flag(shutdown_confirm_mask, LV_OBJ_FLAG_SCROLLABLE);
//     lv_obj_add_flag(shutdown_confirm_mask, LV_OBJ_FLAG_CLICKABLE);

//     /* 弹窗主体 */
//     lv_obj_t *box = lv_obj_create(shutdown_confirm_mask);
//     shutdown_confirm_box = box;
//     lv_obj_set_size(box, 430, 220);
//     lv_obj_center(box);

//     lv_obj_set_style_radius(box, 26, 0);
//     lv_obj_set_style_bg_color(box, lv_color_hex(0x101C3A), 0);
//     lv_obj_set_style_bg_opa(box, LV_OPA_90, 0);

//     lv_obj_set_style_border_width(box, 3, 0);
//     lv_obj_set_style_border_color(box, lv_color_hex(0xFF4FA3), 0);
//     lv_obj_set_style_border_opa(box, LV_OPA_90, 0);

//     lv_obj_set_style_shadow_width(box, 24, 0);
//     lv_obj_set_style_shadow_color(box, lv_color_hex(0xFF4FA3), 0);
//     lv_obj_set_style_shadow_opa(box, LV_OPA_40, 0);

//     lv_obj_set_style_pad_all(box, 0, 0);
//     lv_obj_clear_flag(box, LV_OBJ_FLAG_SCROLLABLE);

//     /* 标题 */
//     lv_obj_t *title = lv_label_create(box);
//     lv_label_set_text(title, "CONFIRM SHUTDOWN");
//     lv_obj_set_style_text_color(title, lv_color_hex(0xFF6FB7), 0);
//     lv_obj_set_style_text_font(title, FONT_NORMAL, 0);
//     lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 28);

//     /* 提示文字 */
//     lv_obj_t *msg = lv_label_create(box);
//     lv_label_set_text(msg, "Power off the whole device?");
//     lv_obj_set_style_text_color(msg, COLOR_CYAN, 0);
//     lv_obj_set_style_text_font(msg, FONT_SMALL, 0);
//     lv_obj_align(msg, LV_ALIGN_TOP_MID, 0, 82);

//     /* Cancel 按钮 */
//     lv_obj_t *btn_cancel = lv_btn_create(box);
//     lv_obj_set_size(btn_cancel, 120, 42);
//     lv_obj_set_pos(btn_cancel, 65, 145);

//     lv_obj_set_style_radius(btn_cancel, 14, 0);
//     lv_obj_set_style_bg_color(btn_cancel, lv_color_hex(0x1E5D87), 0);
//     lv_obj_set_style_bg_opa(btn_cancel, LV_OPA_80, 0);
//     lv_obj_set_style_border_width(btn_cancel, 2, 0);
//     lv_obj_set_style_border_color(btn_cancel, COLOR_CYAN, 0);
//     lv_obj_set_style_shadow_width(btn_cancel, 14, 0);
//     lv_obj_set_style_shadow_color(btn_cancel, COLOR_CYAN, 0);
//     lv_obj_set_style_shadow_opa(btn_cancel, LV_OPA_20, 0);

//     lv_obj_t *cancel_label = lv_label_create(btn_cancel);
//     lv_label_set_text(cancel_label, "Cancel");
//     lv_obj_set_style_text_color(cancel_label, COLOR_CYAN, 0);
//     lv_obj_set_style_text_font(cancel_label, FONT_SMALL, 0);
//     lv_obj_center(cancel_label);

//     lv_obj_add_event_cb(btn_cancel, shutdown_confirm_cancel_cb, LV_EVENT_CLICKED, NULL);

//     /* Shutdown 按钮 */
//     lv_obj_t *btn_yes = lv_btn_create(box);
//     lv_obj_set_size(btn_yes, 120, 42);
//     lv_obj_set_pos(btn_yes, 245, 145);

//     lv_obj_set_style_radius(btn_yes, 14, 0);
//     lv_obj_set_style_bg_color(btn_yes, lv_color_hex(0x6E254A), 0);
//     lv_obj_set_style_bg_opa(btn_yes, LV_OPA_90, 0);
//     lv_obj_set_style_border_width(btn_yes, 2, 0);
//     lv_obj_set_style_border_color(btn_yes, lv_color_hex(0xFF5AAE), 0);
//     lv_obj_set_style_shadow_width(btn_yes, 14, 0);
//     lv_obj_set_style_shadow_color(btn_yes, lv_color_hex(0xFF5AAE), 0);
//     lv_obj_set_style_shadow_opa(btn_yes, LV_OPA_20, 0);

//     lv_obj_t *yes_label = lv_label_create(btn_yes);
//     lv_label_set_text(yes_label, "Shutdown");
//     lv_obj_set_style_text_color(yes_label, lv_color_hex(0xFF8CC8), 0);
//     lv_obj_set_style_text_font(yes_label, FONT_SMALL, 0);
//     lv_obj_center(yes_label);

//     lv_obj_add_event_cb(btn_yes, shutdown_confirm_yes_cb, LV_EVENT_CLICKED, NULL);
// }

// static void brightness_slider_event_cb(lv_event_t *e)
// {
//     lv_obj_t *slider = lv_event_get_target(e);
//     int value = lv_slider_get_value(slider);   /* 10 - 100 */

//     const char *brightness_path = "/sys/class/backlight/backlight-dsi0/brightness";
//     const char *max_path = "/sys/class/backlight/backlight-dsi0/max_brightness";

//     FILE *fp = fopen(max_path, "r");
//     if (!fp) {
//         return;
//     }

//     int max_brightness = 0;
//     if (fscanf(fp, "%d", &max_brightness) != 1) {
//         fclose(fp);
//         return;
//     }
//     fclose(fp);

//     if (max_brightness <= 0) {
//         return;
//     }

//     /*
//      * 最低不允许到 0，防止屏幕黑掉不好恢复。
//      */
//     int min_value = max_brightness / 10;
//     if (min_value < 1) {
//         min_value = 1;
//     }

//     int hw_value = max_brightness * value / 100;
//     if (hw_value < min_value) {
//         hw_value = min_value;
//     }

//     fp = fopen(brightness_path, "w");
//     if (!fp) {
//         return;
//     }

//     fprintf(fp, "%d\n", hw_value);
//     fclose(fp);
// }

// static void create_settings_page(lv_obj_t *scr)
// {
//     lv_obj_t *page = lv_obj_create(scr);
//     pages[PAGE_SETTINGS] = page;

//     lv_obj_set_size(page, PAGE_W, PAGE_H);
//     lv_obj_set_pos(page, PAGE_X, PAGE_Y);
//     lv_obj_set_style_bg_opa(page, LV_OPA_TRANSP, 0);
//     lv_obj_set_style_border_width(page, 0, 0);
//     lv_obj_set_style_pad_all(page, 0, 0);
//     lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);

//     decorative_curve_lines(page);

//     lv_obj_t *settings = glass_panel_create(page, 0, 0, 500, 440);
//     title_create(settings, "5. SETTINGS / SYSTEM SETTINGS", 24, 18);

//     label_create(settings, "Voice Broadcast", 45, 80, COLOR_SUB_TEXT, FONT_NORMAL);
//     setting_voice_state = label_create(settings, "On", 310, 80, COLOR_GREEN, FONT_NORMAL);

//     sw_voice = lv_switch_create(settings);
//     lv_obj_set_pos(sw_voice, 405, 76);
//     lv_obj_add_state(sw_voice, LV_STATE_CHECKED);
//     lv_obj_add_event_cb(sw_voice, voice_switch_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

//     label_create(settings, "Warning Threshold", 45, 130, COLOR_SUB_TEXT, FONT_NORMAL);
//     label_create(settings, "75", 405, 130, COLOR_CYAN, FONT_NORMAL);

//     label_create(settings, "Danger Threshold", 45, 180, COLOR_SUB_TEXT, FONT_NORMAL);
//     label_create(settings, "90", 405, 180, COLOR_CYAN, FONT_NORMAL);

//     label_create(settings, "Fan Default Mode", 45, 230, COLOR_SUB_TEXT, FONT_NORMAL);
//     label_create(settings, "Auto", 370, 230, COLOR_GREEN, FONT_NORMAL);

//     /* 新增：Vehicle Mode */
//     label_create(settings, "Vehicle Mode", 45, 280, COLOR_SUB_TEXT, FONT_NORMAL);

//     btn_vehicle_mode = lv_btn_create(settings);
//     lv_obj_set_size(btn_vehicle_mode, 120, 34);
//     lv_obj_set_pos(btn_vehicle_mode, 345, 270);
//     lv_obj_set_style_radius(btn_vehicle_mode, 16, 0);
//     lv_obj_set_style_bg_color(btn_vehicle_mode, lv_color_hex(0x071A2F), 0);
//     lv_obj_set_style_bg_opa(btn_vehicle_mode, LV_OPA_60, 0);
//     lv_obj_set_style_border_width(btn_vehicle_mode, 1, 0);
//     lv_obj_set_style_border_color(btn_vehicle_mode, COLOR_CYAN, 0);
//     lv_obj_set_style_border_opa(btn_vehicle_mode, LV_OPA_70, 0);
//     lv_obj_add_event_cb(btn_vehicle_mode, vehicle_mode_btn_event_cb, LV_EVENT_CLICKED, NULL);

//     setting_vehicle_mode_label = lv_label_create(btn_vehicle_mode);
//     lv_label_set_text(setting_vehicle_mode_label, "Auto");
//     lv_obj_set_style_text_color(setting_vehicle_mode_label, COLOR_CYAN, 0);
//     lv_obj_set_style_text_font(setting_vehicle_mode_label, FONT_SMALL, 0);
//     lv_obj_center(setting_vehicle_mode_label);

//     /*
//     * 不要开机强制写 AUTO。
//     * 现在 start_demo.sh 可能已经把车辆状态设为 DRIVING。
//     * UI 启动时应该读取当前真实状态，而不是覆盖它。
//     */
//     g_vehicle_mode = read_vehicle_mode_file();
//     update_vehicle_mode_label();

//     label_create(settings, "Screen Brightness", 45, 335, COLOR_SUB_TEXT, FONT_NORMAL);

//     lv_obj_t *bright = lv_slider_create(settings);
//     lv_obj_set_size(bright, 260, 12);
//     lv_obj_set_pos(bright, 200, 342);

//     /* 最低 10%，防止误滑黑屏 */
//     lv_slider_set_range(bright, 10, 100);
//     lv_slider_set_value(bright, 80, LV_ANIM_OFF);

//     lv_obj_add_event_cb(bright, brightness_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

//     lv_obj_t *control = glass_panel_create(page, 535, 0, 305, 440);
//     title_create(control, "SYSTEM METRICS", 24, 14);

//     label_create(control, "Runtime", 35, 55, COLOR_SUB_TEXT, FONT_SMALL);
//     metric_runtime_label = label_create(control, "00:00:00", 165, 55, COLOR_CYAN, FONT_SMALL);

//     label_create(control, "Camera FPS", 35, 85, COLOR_SUB_TEXT, FONT_SMALL);
//     metric_camera_fps_label = label_create(control, "-- FPS", 165, 85, COLOR_GREEN, FONT_SMALL);

//     label_create(control, "NPU FPS", 35, 115, COLOR_SUB_TEXT, FONT_SMALL);
//     metric_ai_fps_label = label_create(control, "-- FPS", 165, 115, COLOR_GREEN, FONT_SMALL);

//     label_create(control, "Infer Time", 35, 145, COLOR_SUB_TEXT, FONT_SMALL);
//     metric_infer_ms_label = label_create(control, "-- ms", 165, 145, COLOR_CYAN, FONT_SMALL);

//     label_create(control, "Preview FPS", 35, 175, COLOR_SUB_TEXT, FONT_SMALL);
//     metric_preview_fps_label = label_create(control, "-- FPS", 165, 175, COLOR_CYAN, FONT_SMALL);

//     label_create(control, "CPU Usage", 35, 205, COLOR_SUB_TEXT, FONT_SMALL);
//     metric_cpu_label = label_create(control, "--%", 165, 205, COLOR_GREEN, FONT_SMALL);

//     label_create(control, "Stability", 35, 235, COLOR_SUB_TEXT, FONT_SMALL);
//     metric_stability_label = label_create(control, "Stable", 165, 235, COLOR_GREEN, FONT_SMALL);

//     /* 分割发光线 */
//     lv_obj_t *split_line = lv_obj_create(control);
//     lv_obj_set_size(split_line, 230, 2);
//     lv_obj_set_pos(split_line, 38, 270);
//     lv_obj_set_style_bg_color(split_line, COLOR_CYAN, 0);
//     lv_obj_set_style_bg_opa(split_line, LV_OPA_50, 0);
//     lv_obj_set_style_border_width(split_line, 0, 0);
//     lv_obj_set_style_radius(split_line, 2, 0);

//     /* ===================== SYSTEM CONTROL ===================== */
//     label_create(control, "SYSTEM CONTROL", 35, 286, COLOR_CYAN, FONT_SMALL);

//     /*
//      * 单个电源控制大方块：
//      * 短按：只反馈
//      * 长按 3s：Restart App
//      * 长按 7s：弹出关机确认
//      */
//     int power_w = 200;
//     int power_h = 96;
//     int power_x = (305 - power_w) / 2;
//     int power_y = 324;

//     lv_obj_t *power_tile = power_tile_create(control,
//                                              power_x, power_y,
//                                              power_w, power_h,
//                                              "POWER",
//                                              "Hold",
//                                              COLOR_CYAN,
//                                              NULL);

//     lv_obj_add_event_cb(power_tile, power_main_btn_event_cb, LV_EVENT_ALL, NULL);
// }

// /* =========================================================
//  * Create UI
//  * ========================================================= */
// void ui_fatigue_create(void)
// {
//     lv_obj_t *scr = lv_scr_act();

//     lv_obj_set_style_bg_color(scr, COLOR_BG, 0);
//     lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

//     create_header(scr);
//     create_sidebar(scr);

//     create_home_page(scr);
//     create_detection_page(scr);
//     create_environment_page(scr);
//     create_records_page(scr);
//     create_settings_page(scr);

//     metric_start_tick = lv_tick_get();
//     lv_timer_create(metrics_timer_cb, 1000, NULL);

//     /*
//      * AI Copilot 结果刷新定时器：
//      * 每 1 秒读取 /home/elf/ai_cloud/runtime/llm_answer /home/elf/ai_cloud/runtime/cloud_status /home/elf/ai_cloud/runtime/cloud_latency。
//      */
//     lv_timer_create(ai_result_timer_cb, 1000, NULL);

//     lv_timer_create(danger_flash_timer_cb, 180, NULL);

//     lv_timer_create(ai_auto_popup_timer_cb, 300, NULL);

//     page_show(PAGE_HOME);
// }

// /* Current version has shutdown in Settings page */
// void ui_add_power_button(void)
// {
// }

// /* =========================================================
//  * Update functions
//  * ========================================================= */
// // void ui_update_fatigue_score(int score)
// // {
// //     score = clamp_int(score, 0, 100);

// //     char buf[16];
// //     snprintf(buf, sizeof(buf), "%d", score);

// //     if (home_score_label) {
// //         lv_label_set_text(home_score_label, buf);
// //     }

// //     if (home_score_bar) {
// //         lv_bar_set_value(home_score_bar, score, LV_ANIM_OFF);
// //     }

// //     if (home_state_arc) {
// //         lv_arc_set_value(home_state_arc, score);
// //     }

// //     if (rec_chart && rec_score_series) {
// //         lv_chart_set_next_value(rec_chart, rec_score_series, score);
// //         lv_chart_refresh(rec_chart);
// //     }

// //     if (score <= 30) {
// //         ui_update_state("NORMAL", COLOR_GREEN);
// //     } else if (score <= 60) {
// //         ui_update_state("MILD RISK", COLOR_YELLOW);
// //     } else if (score <= 80) {
// //         ui_update_state("DROWSY", COLOR_ORANGE);
// //     } else {
// //         ui_update_state("DANGEROUS", COLOR_RED);
// //     }
// // }

// void ui_update_fatigue_score(int score)
// {
//     static int display_score = 0;
//     static int last_chart_score = 0;
//     static uint32_t last_chart_tick = 0;

//     score = clamp_int(score, 0, 100);

//     /*
//      * UI显示分数做平滑处理：
//      * 每次最多变化 2 分，避免进度条抖动。
//      */
//     if (display_score < score) {
//         display_score += 2;
//         if (display_score > score) display_score = score;
//     } else if (display_score > score) {
//         display_score -= 2;
//         if (display_score < score) display_score = score;
//     }

//     char buf[16];
//     snprintf(buf, sizeof(buf), "%d", display_score);

//     if (home_score_label) {
//         lv_label_set_text(home_score_label, buf);
//     }

//     /*
//     * Detect 页面同步显示 Risk Score。
//     * 这样演示时不用切回 Home 页面看状态分。
//     */
//     if (detect_risk_score_label) {
//         lv_label_set_text(detect_risk_score_label, buf);

//         if (display_score >= 80) {
//             lv_obj_set_style_text_color(detect_risk_score_label, COLOR_RED, 0);
//         } else if (display_score >= 50) {
//             lv_obj_set_style_text_color(detect_risk_score_label, COLOR_YELLOW, 0);
//         } else {
//             lv_obj_set_style_text_color(detect_risk_score_label, COLOR_GREEN, 0);
//         }
//     }

//     /*
//      * 关键：这里用 LV_ANIM_OFF，不要每 500ms 重新开动画。
//      */
//     if (home_score_bar) {
//         lv_bar_set_value(home_score_bar, display_score, LV_ANIM_OFF);
//     }

//     if (home_state_arc) {
//         lv_arc_set_value(home_state_arc, display_score);
//     }

//     /*
//      * 曲线图不要每 500ms 刷一次。
//      * 1秒记录一个点就够了，否则容易增加重绘压力。
//      */
//     uint32_t now = lv_tick_get();

//     if (rec_chart && rec_score_series && now - last_chart_tick >= 1000) {
//     last_chart_tick = now;
//     last_chart_score = display_score;

//     lv_chart_set_next_value(rec_chart, rec_score_series, last_chart_score);
//     lv_chart_refresh(rec_chart);

//     records_update_time_axis();
// }

//     /*
//      * 状态判断仍然用原始 score，响应更及时。
//      * 如果你想状态也更稳，可以后面再加迟滞判断。
//      */
// int fatigue_level = 0;

// if (score <= 30) {
//     fatigue_level = 0;
//     ui_update_state("NORMAL", COLOR_GREEN);
// } else if (score <= 60) {
//     fatigue_level = 1;
//     ui_update_state("MILD RISK", COLOR_YELLOW);
// } else if (score <= 80) {
//     fatigue_level = 2;
//     ui_update_state("DROWSY", COLOR_ORANGE);
// } else {
//     fatigue_level = 3;
//     ui_update_state("DANGEROUS", COLOR_RED);
// }

// /*
//  * 只在疲劳等级变化时记录，避免每 500ms 刷屏。
//  */
// if (last_fatigue_level != fatigue_level) {
//     last_fatigue_level = fatigue_level;

//     if (fatigue_level == 0) {
//         records_add_event("Fatigue risk normal", COLOR_GREEN);
//     } else if (fatigue_level == 1) {
//         records_add_event("Mild fatigue risk warning", COLOR_YELLOW);
//     } else if (fatigue_level == 2) {
//         records_add_event("Drowsy risk warning", COLOR_ORANGE);
//     } else {
//         records_add_event("Dangerous fatigue risk", COLOR_RED);
//     }
// }

// }

// void ui_update_state(const char *state_text, lv_color_t color)
// {
//     if (home_state_label) {
//         lv_label_set_text(home_state_label, state_text);
//         lv_obj_set_style_text_color(home_state_label, color, 0);
//     }

//     if (det_driver_state_label) {
//         lv_label_set_text(det_driver_state_label, state_text);
//         lv_obj_set_style_text_color(det_driver_state_label, color, 0);
//     }

//     if (home_state_icon) {
//         if (strcmp(state_text, "NORMAL") == 0) {
//             lv_label_set_text(home_state_icon, ":)");
//         } else if (strcmp(state_text, "NO DRIVER") == 0) {
//             lv_label_set_text(home_state_icon, "--");
//         } else if (strcmp(state_text, "MILD RISK") == 0) {
//             lv_label_set_text(home_state_icon, ":|");
//         } else if (strcmp(state_text, "DROWSY") == 0) {
//             lv_label_set_text(home_state_icon, "Zz");
//         } else {
//             lv_label_set_text(home_state_icon, "!!");
//         }

//         lv_obj_set_style_text_color(home_state_icon, color, 0);
//     }

//     if (home_state_arc) {
//         lv_obj_set_style_arc_color(home_state_arc, color, LV_PART_INDICATOR);
//     }

//     if (home_state_desc) {
//         if (strcmp(state_text, "NORMAL") == 0) {
//             lv_label_set_text(home_state_desc, "Driver status is good. Keep focused.");
//         } else if (strcmp(state_text, "MILD RISK") == 0) {
//             lv_label_set_text(home_state_desc, "Attention is decreasing. Please focus.");
//         } else if (strcmp(state_text, "DROWSY") == 0) {
//             lv_label_set_text(home_state_desc, "Drowsy risk detected. Rest is recommended.");
//         } else {
//             lv_label_set_text(home_state_desc, "Dangerous state. Please stop driving.");
//         }
//     }

//     if (home_alert_text) {
//         if (strcmp(state_text, "NORMAL") == 0) {
//             lv_label_set_text(home_alert_text, "No abnormal alert");
//             lv_obj_set_style_text_color(home_alert_text, COLOR_GREEN, 0);
//         } else if (strcmp(state_text, "MILD RISK") == 0) {
//             lv_label_set_text(home_alert_text, "Please pay attention");
//             lv_obj_set_style_text_color(home_alert_text, COLOR_YELLOW, 0);
//         } else if (strcmp(state_text, "DROWSY") == 0) {
//             lv_label_set_text(home_alert_text, "Rest is recommended");
//             lv_obj_set_style_text_color(home_alert_text, COLOR_ORANGE, 0);
//         } else {
//             lv_label_set_text(home_alert_text, "Danger! Stop driving");
//             lv_obj_set_style_text_color(home_alert_text, COLOR_RED, 0);
//         }
//     }

//     if (det_risk_text) {
//         char buf[64];
//         snprintf(buf, sizeof(buf), "Current Risk:");
//         lv_label_set_text(det_risk_text, buf);
//         lv_obj_set_style_text_color(det_risk_text, COLOR_SUB_TEXT, 0);
//     }

//     if (det_explain_text) {
//         if (strcmp(state_text, "NORMAL") == 0) {
//             lv_label_set_text(det_explain_text, "No abnormal behavior.");
//         } else {
//             lv_label_set_text(det_explain_text, "Risk rising.");
//         }
//     }
// }

// void ui_update_eye_state(const char *eye_state)
// {
//     if (!eye_state) return;

//     if (det_eye_state) {
//         lv_label_set_text(det_eye_state, eye_state);

//         if (strcmp(eye_state, "Normal") == 0 || strcmp(eye_state, "NORMAL") == 0) {
//             set_label_color(det_eye_state, COLOR_GREEN);
//         } else if (strcmp(eye_state, "Closed") == 0) {
//             set_label_color(det_eye_state, COLOR_RED);
//         } else {
//             set_label_color(det_eye_state, COLOR_YELLOW);
//         }
//     }
// }

// void ui_update_perclos(int perclos)
// {
//     char buf[16];
//     perclos = clamp_int(perclos, 0, 100);
//     snprintf(buf, sizeof(buf), "%d%%", perclos);

//     if (det_perclos) {
//         lv_label_set_text(det_perclos, buf);
//     }
// }

// void ui_update_blink_freq(int blink_freq)
// {
//     char buf[32];
//     snprintf(buf, sizeof(buf), "%d/min", blink_freq);

//     if (det_blink) {
//         lv_label_set_text(det_blink, buf);
//     }
// }

// void ui_update_yawn_count(int yawn_count)
// {
//     // char buf[16];
//     // snprintf(buf, sizeof(buf), "%d", yawn_count);

//     // if (det_yawn) {
//     //     lv_label_set_text(det_yawn, buf);
//     // }
//     (void)yawn_count;
// }

// void ui_update_phone_state(int detected)
// {
//     pill_update(home_phone_pill, detected);
//     pill_update(det_phone_pill, detected);

//     if (last_phone_state != detected) {
//         last_phone_state = detected;

//         if (detected) {
//             records_add_event("Phone use signal detected", COLOR_YELLOW);
//         } else {
//             records_add_event("Phone signal cleared", COLOR_GREEN);
//         }
//     }
// }

// void ui_update_smoking_state(int detected)
// {
//     /*
//      * 注意：后端 smoking 字段现在兼容表示 mouth_risk。
//      * UI 上不要再叫 Smoking，统一显示为 Mouth Risk。
//      */
//     pill_update(home_smoke_pill, detected);
//     pill_update(det_smoke_pill, detected);

//     if (last_smoking_state != detected) {
//         last_smoking_state = detected;

//         if (detected) {
//             records_add_event("Mouth risk detected", COLOR_YELLOW);
//         } else {
//             records_add_event("Mouth risk cleared", COLOR_GREEN);
//         }
//     }
// }

// static void wheel_pill_update(lv_obj_t *label, int hands_off)
// {
//     if (!label) return;

//     lv_obj_t *pill = lv_obj_get_parent(label);
//     if (!pill) return;

//     /*
//      * hands_off:
//      * -1 = No Driver / Unknown
//      *  0 = Wheel On
//      *  1 = Wheel Off
//      */
//     if (hands_off < 0) {
//         lv_label_set_text(label, "--");
//         lv_obj_set_style_text_color(label, COLOR_SUB_TEXT, 0);
//         lv_obj_set_style_border_color(pill, COLOR_BORDER_DIM, 0);
//         lv_obj_set_style_bg_color(pill, lv_color_hex(0x101826), 0);
//     } else if (hands_off) {
//         lv_label_set_text(label, "Off");
//         lv_obj_set_style_text_color(label, COLOR_RED, 0);
//         lv_obj_set_style_border_color(pill, COLOR_RED, 0);
//         lv_obj_set_style_bg_color(pill, lv_color_hex(0x361224), 0);
//     } else {
//         lv_label_set_text(label, "On");
//         lv_obj_set_style_text_color(label, COLOR_GREEN, 0);
//         lv_obj_set_style_border_color(pill, COLOR_GREEN, 0);
//         lv_obj_set_style_bg_color(pill, lv_color_hex(0x092C25), 0);
//     }
// }

// void ui_update_wheel_state(int hands_off)
// {
//     /*
//      * hands_off:
//      * -1 = No Driver / Unknown
//      *  0 = Wheel On
//      *  1 = Wheel Off
//      */

//     if (home_wheel_pill) {
//         wheel_pill_update(home_wheel_pill, hands_off);
//     }

//     if (det_yawn) {
//         if (hands_off < 0) {
//             lv_label_set_text(det_yawn, "--");
//             lv_obj_set_style_text_color(det_yawn, COLOR_SUB_TEXT, 0);
//         } else if (hands_off) {
//             lv_label_set_text(det_yawn, "Off");
//             lv_obj_set_style_text_color(det_yawn, COLOR_RED, 0);
//         } else {
//             lv_label_set_text(det_yawn, "On");
//             lv_obj_set_style_text_color(det_yawn, COLOR_GREEN, 0);
//         }
//     }
// }

// void ui_update_vehicle_state(int moving)
// {
//     if (!det_vehicle_state) {
//         return;
//     }

//     if (moving > 0) {
//         lv_label_set_text(det_vehicle_state, "MOVING");
//         lv_obj_set_style_text_color(det_vehicle_state, COLOR_GREEN, 0);
//     } else if (moving == 0) {
//         lv_label_set_text(det_vehicle_state, "PARKED");
//         lv_obj_set_style_text_color(det_vehicle_state, COLOR_YELLOW, 0);
//     } else {
//         lv_label_set_text(det_vehicle_state, "--");
//         lv_obj_set_style_text_color(det_vehicle_state, COLOR_SUB_TEXT, 0);
//     }
// }

// void ui_update_eye_risk(float eye_risk)
// {
//     /*
//      * eye_risk:
//      * < 0.0f = No Driver / Unknown
//      * 0.0f ~ 1.0f = valid eye risk
//      */
//     if (eye_risk < 0.0f) {
//         if (det_perclos) {
//             lv_label_set_text(det_perclos, "--");
//             lv_obj_set_style_text_color(det_perclos, COLOR_SUB_TEXT, 0);
//         }

//         if (det_eye_state) {
//             lv_label_set_text(det_eye_state, "--");
//             lv_obj_set_style_text_color(det_eye_state, COLOR_SUB_TEXT, 0);
//         }

//         return;
//     }

//     if (eye_risk > 1.0f) eye_risk = 1.0f;

//     int percent = (int)(eye_risk * 100.0f + 0.5f);

//     char buf[16];
//     snprintf(buf, sizeof(buf), "%d%%", percent);

//     if (det_perclos) {
//         lv_label_set_text(det_perclos, buf);

//         if (percent < 30) {
//             lv_obj_set_style_text_color(det_perclos, COLOR_GREEN, 0);
//         } else if (percent < 60) {
//             lv_obj_set_style_text_color(det_perclos, COLOR_YELLOW, 0);
//         } else {
//             lv_obj_set_style_text_color(det_perclos, COLOR_RED, 0);
//         }
//     }

//     if (det_eye_state) {
//         if (percent < 30) {
//             lv_label_set_text(det_eye_state, "Normal");
//             lv_obj_set_style_text_color(det_eye_state, COLOR_GREEN, 0);
//         } else if (percent < 60) {
//             lv_label_set_text(det_eye_state, "Watch");
//             lv_obj_set_style_text_color(det_eye_state, COLOR_YELLOW, 0);
//         } else {
//             lv_label_set_text(det_eye_state, "Risk");
//             lv_obj_set_style_text_color(det_eye_state, COLOR_RED, 0);
//         }
//     }
// }

// void ui_update_yaw_state(int yaw_state, float yaw_score)
// {
//     (void)yaw_score;

//     if (!det_blink) {
//         return;
//     }

//     /*
//      * yaw_state:
//      * -1 = No Driver / Unknown
//      *  0 = Normal
//      *  1 = Left / Right depending on camera mirror
//      *  2 = Left / Right depending on camera mirror
//      */
//     if (yaw_state < 0) {
//         lv_label_set_text(det_blink, "--");
//         lv_obj_set_style_text_color(det_blink, COLOR_SUB_TEXT, 0);
//         return;
//     }

//     /*
//      * 摄像头面对驾驶员时，图像左右和驾驶员真实左右是镜像关系。
//      * behavior_engine 输出的是图像坐标方向，
//      * UI 这里显示成驾驶员真实方向，所以左右需要互换。
//      */
//     if (yaw_state == 1) {
//         lv_label_set_text(det_blink, "Right");
//         lv_obj_set_style_text_color(det_blink, COLOR_YELLOW, 0);
//     } else if (yaw_state == 2) {
//         lv_label_set_text(det_blink, "Left");
//         lv_obj_set_style_text_color(det_blink, COLOR_YELLOW, 0);
//     } else {
//         lv_label_set_text(det_blink, "Normal");
//         lv_obj_set_style_text_color(det_blink, COLOR_CYAN, 0);
//     }
// }

// void ui_update_hands_state(int hands_off)
// {
//     if (!det_yawn) {
//         return;
//     }

//     if (hands_off) {
//         lv_label_set_text(det_yawn, "Off");
//         lv_obj_set_style_text_color(det_yawn, COLOR_RED, 0);
//     } else {
//         lv_label_set_text(det_yawn, "On");
//         lv_obj_set_style_text_color(det_yawn, COLOR_GREEN, 0);
//     }
// }

// void ui_update_head_state(int detected)
// {
//     pill_update(home_head_pill, detected);
//     pill_update(det_head_pill, detected);

//     if (last_head_state != detected) {
//         last_head_state = detected;

//         if (detected) {
//             records_add_event("Head down risk detected", COLOR_ORANGE);
//         } else {
//             records_add_event("Head position recovered", COLOR_GREEN);
//         }
//     }
// }

// void ui_update_env(float temp, int humi)
// {
//     char buf[32];

//     snprintf(buf, sizeof(buf), "%.1f°C", (double)temp);

//     if (env_temp_label) {
//         lv_label_set_text(env_temp_label, buf);
//     }

//     snprintf(buf, sizeof(buf), "%d%%", humi);

//     if (env_humi_label) {
//         lv_label_set_text(env_humi_label, buf);
//     }

//     if (temp >= 18.0f && temp <= 28.0f && humi >= 40 && humi <= 70) {
//         if (env_comfort_label) {
//             lv_label_set_text(env_comfort_label, "Good");
//             set_label_color(env_comfort_label, COLOR_GREEN);
//         }

//         if (home_comfort) {
//             lv_label_set_text(home_comfort, "Good");
//             set_label_color(home_comfort, COLOR_GREEN);
//         }
//     } else if (temp > 28.0f || humi > 75) {
//         if (env_comfort_label) {
//             lv_label_set_text(env_comfort_label, "Hot");
//             set_label_color(env_comfort_label, COLOR_YELLOW);
//         }

//         if (home_comfort) {
//             lv_label_set_text(home_comfort, "Hot");
//             set_label_color(home_comfort, COLOR_YELLOW);
//         }
//     } else {
//         if (env_comfort_label) {
//             lv_label_set_text(env_comfort_label, "Poor");
//             set_label_color(env_comfort_label, COLOR_ORANGE);
//         }

//         if (home_comfort) {
//             lv_label_set_text(home_comfort, "Poor");
//             set_label_color(home_comfort, COLOR_ORANGE);
//         }
//     }
// }

// void ui_update_fan_speed(int speed)
// {
//     speed = clamp_int(speed, 0, 100);

//     char buf[16];
//     snprintf(buf, sizeof(buf), "%d%%", speed);

//     if (env_fan_speed) {
//         lv_label_set_text(env_fan_speed, buf);
//     }

//     if (home_fan_speed) {
//         lv_label_set_text(home_fan_speed, buf);
//     }

//     if (env_fan_arc) {
//         lv_arc_set_value(env_fan_arc, speed);
//     }

//     if (!slider_fan) return;

//     if (g_fan_ctrl && g_fan_ctrl->mode == 2) {
//         return;
//     }

//     g_updating_slider = true;
//     lv_slider_set_value(slider_fan, speed, LV_ANIM_OFF);
//     g_updating_slider = false;
// }

// void ui_update_fan_mode(const char *mode_text)
// {
//     if (!mode_text) return;

//     if (env_fan_mode) {
//         lv_label_set_text(env_fan_mode, mode_text);
//     }

//     if (home_fan_mode) {
//         lv_label_set_text(home_fan_mode, mode_text);
//     }

//     lv_color_t color = COLOR_GREEN;

//     if (strcmp(mode_text, "Manual") == 0) {
//         color = COLOR_CYAN;
//     } else if (strcmp(mode_text, "Off") == 0) {
//         color = COLOR_SUB_TEXT;
//     }

//     set_label_color(env_fan_mode, color);
//     set_label_color(home_fan_mode, color);
// }

// void ui_update_voice_state(bool enabled)
// {
//     if (setting_voice_state) {
//         if (enabled) {
//             lv_label_set_text(setting_voice_state, "On");
//             lv_obj_set_style_text_color(setting_voice_state, COLOR_GREEN, 0);
//         } else {
//             lv_label_set_text(setting_voice_state, "Off");
//             lv_obj_set_style_text_color(setting_voice_state, COLOR_RED, 0);
//         }
//     }

//     /*
//      * Voice Broadcast 开关状态写入临时文件。
//      * main.c 的语音预警逻辑会读取这个文件。
//      *
//      * 1 = 开启主动语音预警
//      * 0 = 关闭主动语音预警
//      */
//     FILE *fp = fopen("/tmp/voice_broadcast_enable", "w");
//     if (fp) {
//         fprintf(fp, "%d\n", enabled ? 1 : 0);
//         fclose(fp);
//     }
// }

// void ui_update_time(const char *time_text, const char *date_text)
// {
//     if (label_time && time_text) {
//         lv_label_set_text(label_time, time_text);
//     }

//     if (label_date && date_text) {
//         lv_label_set_text(label_date, date_text);
//     }
// }

// void ui_update_chip_temp(float chip_temp)
// {
//     char buf[16];
//     snprintf(buf, sizeof(buf), "%.1f°C", (double)chip_temp);

//     if (env_chip_temp) {
//         lv_label_set_text(env_chip_temp, buf);
//     }

//     if (home_chip_temp) {
//         lv_label_set_text(home_chip_temp, buf);
//     }

//     lv_color_t color = COLOR_GREEN;

//     if (chip_temp >= 60.0f) {
//         color = COLOR_RED;
//     } else if (chip_temp >= 50.0f) {
//         color = COLOR_ORANGE;
//     }

//     set_label_color(env_chip_temp, color);
//     set_label_color(home_chip_temp, color);
// }

// void ui_show_alarm(const char *msg)
// {
//     static const char *btns[] = {"OK", ""};

//     lv_obj_t *mbox = lv_msgbox_create(NULL, "Alert", msg, btns, true);
//     lv_obj_center(mbox);
//     lv_obj_set_style_bg_color(mbox, COLOR_PANEL, 0);
//     lv_obj_set_style_text_color(mbox, COLOR_TEXT, 0);
//     lv_obj_set_style_border_color(mbox, COLOR_RED, 0);
//     lv_obj_set_style_border_width(mbox, 2, 0);
// }

// /* =========================================================
//  * Shutdown
//  * ========================================================= */
// static void power_btn_confirm_cb(lv_event_t *e)
// {
//     lv_obj_t *mbox = lv_event_get_current_target(e);
//     uint32_t btn = lv_msgbox_get_active_btn(mbox);

//     if (btn == 1) {
//         int ret;

//         ret = system("sudo killall gst_producer behavior_engine env_monitor fan_control 2>/dev/null");
//         (void)ret;

//         ret = system("sudo sh -c \"echo 1 > /sys/class/pwm/pwmchip0/pwm0/enable\" 2>/dev/null");
//         (void)ret;

//         ret = system("sudo sh -c \"echo 1000000 > /sys/class/pwm/pwmchip0/pwm0/duty_cycle\" 2>/dev/null");
//         (void)ret;

//         exit(0);
//     }

//     lv_msgbox_close(mbox);
// }

// static void power_btn_event_cb(lv_event_t *e)
// {
//     (void)e;

//     static const char *btns[] = {"Cancel", "Exit", ""};

//     lv_obj_t *mbox = lv_msgbox_create(NULL,
//                                       "Shutdown",
//                                       "Stop all processes and exit?",
//                                       btns,
//                                       true);
//     lv_obj_center(mbox);
//     lv_obj_add_event_cb(mbox, power_btn_confirm_cb, LV_EVENT_VALUE_CHANGED, NULL);
// }





// /* =========================================================
//  * Camera preview update
//  * Source: SharedFrame RGB888
//  * Target: LVGL native color buffer
//  * ========================================================= */

// static void camera_preview_init_buffer(void)
// {
//     if (camera_preview_buf) {
//         return;
//     }

//     camera_preview_buf = (lv_color_t *)malloc(CAM_PREVIEW_W * CAM_PREVIEW_H * sizeof(lv_color_t));
//     if (!camera_preview_buf) {
//         printf("[UI] camera_preview_buf malloc failed\n");
//         return;
//     }

//     memset(camera_preview_buf, 0, CAM_PREVIEW_W * CAM_PREVIEW_H * sizeof(lv_color_t));

//     memset(&camera_img_dsc, 0, sizeof(camera_img_dsc));
//     camera_img_dsc.header.always_zero = 0;
//     camera_img_dsc.header.w = CAM_PREVIEW_W;
//     camera_img_dsc.header.h = CAM_PREVIEW_H;
//     camera_img_dsc.header.cf = LV_IMG_CF_TRUE_COLOR;
//     camera_img_dsc.data_size = CAM_PREVIEW_W * CAM_PREVIEW_H * sizeof(lv_color_t);
//     camera_img_dsc.data = (const uint8_t *)camera_preview_buf;

//     if (camera_img) {
//         lv_img_set_src(camera_img, &camera_img_dsc);
//     }
// }

// static void camera_compute_crop_rect(int src_w, int src_h)
// {
//     if (src_w <= 0 || src_h <= 0) {
//         src_w = CAM_WIDTH;
//         src_h = CAM_HEIGHT;
//     }

//     float ratio = camera_view_crop_ratio[g_camera_view_mode];

//     int crop_w = (int)((float)src_w * ratio);
//     int crop_h = (int)((float)src_h * ratio);

//     /*
//      * 保持和预览框一样的 16:9 比例。
//      * 你的源图 1280x720 和预览 555x312 都接近 16:9。
//      */
//     float preview_aspect = (float)CAM_PREVIEW_W / (float)CAM_PREVIEW_H;
//     float crop_aspect = (float)crop_w / (float)crop_h;

//     if (crop_aspect > preview_aspect) {
//         crop_w = (int)((float)crop_h * preview_aspect);
//     } else {
//         crop_h = (int)((float)crop_w / preview_aspect);
//     }

//     if (crop_w > src_w) crop_w = src_w;
//     if (crop_h > src_h) crop_h = src_h;

//     int max_x = src_w - crop_w;
//     int max_y = src_h - crop_h;

//     if (max_x < 0) max_x = 0;
//     if (max_y < 0) max_y = 0;

//     int crop_x = max_x / 2;

//     float y_bias = camera_view_y_bias[g_camera_view_mode];
//     int crop_y = (int)((float)max_y * y_bias);

//     if (crop_y < 0) crop_y = 0;
//     if (crop_y > max_y) crop_y = max_y;

//     g_crop_x = crop_x;
//     g_crop_y = crop_y;
//     g_crop_w = crop_w;
//     g_crop_h = crop_h;
// }

// static void camera_rgb888_to_lvgl_nearest(const unsigned char *src_rgb,
//                                           int src_w,
//                                           int src_h)
// {
//     if (!src_rgb || !camera_preview_buf) {
//         return;
//     }

//     if (src_w <= 0 || src_h <= 0) {
//         return;
//     }

//     camera_compute_crop_rect(src_w, src_h);

//     /*
//      * 从裁剪区域 g_crop_x/g_crop_y/g_crop_w/g_crop_h
//      * 缩放到固定预览框 CAM_PREVIEW_W x CAM_PREVIEW_H。
//      */
//     for (int y = 0; y < CAM_PREVIEW_H; y++) {
//         int sy = g_crop_y + (y * g_crop_h) / CAM_PREVIEW_H;

//         if (sy < 0) sy = 0;
//         if (sy >= src_h) sy = src_h - 1;

//         for (int x = 0; x < CAM_PREVIEW_W; x++) {
//             int sx = g_crop_x + (x * g_crop_w) / CAM_PREVIEW_W;

//             if (sx < 0) sx = 0;
//             if (sx >= src_w) sx = src_w - 1;

//             int src_idx = (sy * src_w + sx) * 3;

//             unsigned char r = src_rgb[src_idx + 0];
//             unsigned char g = src_rgb[src_idx + 1];
//             unsigned char b = src_rgb[src_idx + 2];

//             camera_preview_buf[y * CAM_PREVIEW_W + x] = lv_color_make(r, g, b);
//         }
//     }
// }

// void ui_update_camera_preview(void)
// {
//     if (!g_camera_frame || !camera_img) {
//         return;
//     }

//     /*
//      * 防止共享内存还没写入有效数据。
//      */
//     if (g_camera_frame->width <= 0 ||
//         g_camera_frame->height <= 0 ||
//         g_camera_frame->width > CAM_WIDTH ||
//         g_camera_frame->height > CAM_HEIGHT) {
//         return;
//     }

//     /*
//      * 没有新帧就不刷新，降低 UI 重绘压力。
//      */
//     if (g_camera_frame->frame_count == last_camera_frame_count) {
//         return;
//     }

//     last_camera_frame_count = g_camera_frame->frame_count;

//     camera_preview_init_buffer();
//     if (!camera_preview_buf) {
//         return;
//     }

//     camera_rgb888_to_lvgl_nearest(g_camera_frame->rgb_data,
//                                   g_camera_frame->width,
//                                   g_camera_frame->height);

//     if (camera_placeholder_1) {
//         lv_obj_add_flag(camera_placeholder_1, LV_OBJ_FLAG_HIDDEN);
//     }

//     if (camera_placeholder_2) {
//         lv_obj_add_flag(camera_placeholder_2, LV_OBJ_FLAG_HIDDEN);
//     }

//     /*
//      * 让 LVGL 重新绘制这张图片。
//      */
//     lv_img_cache_invalidate_src(&camera_img_dsc);
//     lv_obj_invalidate(camera_img);

//     /* 统计 UI 实际显示帧数，用于 Preview FPS */
//     camera_preview_frame_count++;

//     ui_update_detection_overlay();

// }

// static void nav_child_text_color(lv_obj_t *obj, lv_color_t color)
// {
//     if (!obj) return;

//     uint32_t cnt = lv_obj_get_child_cnt(obj);

//     for (uint32_t i = 0; i < cnt; i++) {
//         lv_obj_t *child = lv_obj_get_child(obj, i);

//         if (child) {
//             lv_obj_set_style_text_color(child, color, 0);
//             nav_child_text_color(child, color);
//         }
//     }
// }

// static void nav_set_active_manual(int active_page)
// {
//     for (int i = 0; i < PAGE_COUNT; i++) {
//         if (!nav_btns[i]) continue;

//         /*
//          * 清掉 LVGL 默认状态，避免红色 checked/focused 残留。
//          */
//         lv_obj_clear_state(nav_btns[i], LV_STATE_CHECKED);
//         lv_obj_clear_state(nav_btns[i], LV_STATE_FOCUSED);
//         lv_obj_clear_state(nav_btns[i], LV_STATE_PRESSED);

//         if (i == active_page) {
//             /*
//              * 当前页面：蓝色高亮按钮。
//              * 这里用你的系统蓝色/青色风格。
//              */
//             lv_obj_set_style_bg_color(nav_btns[i], lv_color_hex(0x0B7CFF), 0);
//             lv_obj_set_style_bg_opa(nav_btns[i], LV_OPA_70, 0);

//             lv_obj_set_style_border_width(nav_btns[i], 2, 0);
//             lv_obj_set_style_border_color(nav_btns[i], COLOR_CYAN, 0);
//             lv_obj_set_style_border_opa(nav_btns[i], LV_OPA_90, 0);

//             lv_obj_set_style_shadow_width(nav_btns[i], 18, 0);
//             lv_obj_set_style_shadow_color(nav_btns[i], COLOR_CYAN, 0);
//             lv_obj_set_style_shadow_opa(nav_btns[i], LV_OPA_30, 0);

//             nav_child_text_color(nav_btns[i], COLOR_CYAN);
//         } else {
//             /*
//              * 非当前页面：恢复普通深色按钮。
//              */
//             lv_obj_set_style_bg_color(nav_btns[i], lv_color_hex(0x071A2F), 0);
//             lv_obj_set_style_bg_opa(nav_btns[i], LV_OPA_30, 0);

//             lv_obj_set_style_border_width(nav_btns[i], 1, 0);
//             lv_obj_set_style_border_color(nav_btns[i], COLOR_BORDER_DIM, 0);
//             lv_obj_set_style_border_opa(nav_btns[i], LV_OPA_60, 0);

//             lv_obj_set_style_shadow_width(nav_btns[i], 0, 0);
//             lv_obj_set_style_shadow_opa(nav_btns[i], LV_OPA_TRANSP, 0);

//             nav_child_text_color(nav_btns[i], COLOR_CYAN);
//         }
//     }
// }

// static void ui_show_page_by_id(int page_id)
// {
//     if (page_id < 0 || page_id >= PAGE_COUNT) {
//         return;
//     }

//     /*
//      * 1. 切换页面显示
//      */
//     for (int i = 0; i < PAGE_COUNT; i++) {
//         if (pages[i]) {
//             lv_obj_add_flag(pages[i], LV_OBJ_FLAG_HIDDEN);
//         }
//     }

//     if (pages[page_id]) {
//         lv_obj_clear_flag(pages[page_id], LV_OBJ_FLAG_HIDDEN);
//     }

//     /*
//      * 2. 手动同步左侧导航蓝色高亮
//      * 不使用 LV_STATE_CHECKED，避免红色按钮出现。
//      */
//     nav_set_active_manual(page_id);
// }

// void ui_handle_voice_command(const char *cmd)
// {
//     if (!cmd) return;

//     /*
//      * Page control
//      */
//     if (strcmp(cmd, "HOME") == 0) {
//         ui_show_page_by_id(PAGE_HOME);
//         return;
//     } else if (strcmp(cmd, "DETECT") == 0) {
//         ui_show_page_by_id(PAGE_DETECTION);
//         return;
//     } else if (strcmp(cmd, "ENV") == 0) {
//         ui_show_page_by_id(PAGE_ENVIRONMENT);
//         return;
//     } else if (strcmp(cmd, "RECORD") == 0) {
//         ui_show_page_by_id(PAGE_RECORDS);
//         return;
//     } else if (strcmp(cmd, "SETTING") == 0) {
//         ui_show_page_by_id(PAGE_SETTINGS);
//         return;
//     }

//     /*
//      * Fan control
//      *
//      * FanCtrl mode:
//      * 0 = Auto
//      * 2 = Manual
//      * 3 = Off
//      */
//     if (strcmp(cmd, "FAN_AUTO") == 0) {
//         if (g_fan_ctrl) {
//             g_fan_ctrl->mode = 0;
//             g_fan_ctrl->timestamp = (uint64_t)time(NULL);
//         }

//         ui_update_fan_mode("Auto");

//         if (slider_fan) {
//             g_updating_slider = true;
//             lv_slider_set_value(slider_fan, 0, LV_ANIM_OFF);
//             g_updating_slider = false;

//             lv_obj_clear_flag(slider_fan, LV_OBJ_FLAG_CLICKABLE);
//             lv_obj_add_state(slider_fan, LV_STATE_DISABLED);
//         }

//         return;
//     }

//     if (strcmp(cmd, "FAN_ON") == 0) {
//         if (g_fan_ctrl) {
//             g_fan_ctrl->mode = 2;
//             g_fan_ctrl->manual_speed = 70;
//             g_fan_ctrl->timestamp = (uint64_t)time(NULL);
//         }

//         ui_update_fan_mode("Manual");
//         ui_update_fan_speed(70);

//         if (slider_fan) {
//             g_updating_slider = true;
//             lv_slider_set_value(slider_fan, 70, LV_ANIM_OFF);
//             g_updating_slider = false;

//             lv_obj_add_flag(slider_fan, LV_OBJ_FLAG_CLICKABLE);
//             lv_obj_clear_state(slider_fan, LV_STATE_DISABLED);
//         }

//         return;
//     }

//     if (strcmp(cmd, "FAN_MAX") == 0) {
//         if (g_fan_ctrl) {
//             g_fan_ctrl->mode = 2;
//             g_fan_ctrl->manual_speed = 100;
//             g_fan_ctrl->timestamp = (uint64_t)time(NULL);
//         }

//         ui_update_fan_mode("Manual");
//         ui_update_fan_speed(100);

//         if (slider_fan) {
//             g_updating_slider = true;
//             lv_slider_set_value(slider_fan, 100, LV_ANIM_OFF);
//             g_updating_slider = false;

//             lv_obj_add_flag(slider_fan, LV_OBJ_FLAG_CLICKABLE);
//             lv_obj_clear_state(slider_fan, LV_STATE_DISABLED);
//         }

//         return;
//     }

//     if (strcmp(cmd, "FAN_OFF") == 0) {
//         if (g_fan_ctrl) {
//             g_fan_ctrl->mode = 3;
//             g_fan_ctrl->manual_speed = 0;
//             g_fan_ctrl->timestamp = (uint64_t)time(NULL);
//         }

//         ui_update_fan_mode("Off");
//         ui_update_fan_speed(0);

//         if (slider_fan) {
//             g_updating_slider = true;
//             lv_slider_set_value(slider_fan, 0, LV_ANIM_OFF);
//             g_updating_slider = false;

//             lv_obj_clear_flag(slider_fan, LV_OBJ_FLAG_CLICKABLE);
//             lv_obj_add_state(slider_fan, LV_STATE_DISABLED);
//         }

//         return;
//     }
// }


#include "ui_fatigue.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

//LV_FONT_DECLARE(lv_font_chinese_18);

/* =========================================================
 * LVGL 8.2 compatible opacity
 * ========================================================= */
#define OPACITY_20      ((lv_opa_t)51)
#define OPACITY_30      ((lv_opa_t)76)
#define OPACITY_35      ((lv_opa_t)90)
#define OPACITY_45      ((lv_opa_t)115)
#define OPACITY_55      ((lv_opa_t)140)
#define OPACITY_70      ((lv_opa_t)178)

/* =========================================================
 * Colors
 * ========================================================= */
#define COLOR_BG        lv_color_hex(0x030914)
#define COLOR_HEADER    lv_color_hex(0x050B15)
#define COLOR_SIDEBAR   lv_color_hex(0x06101E)

#define COLOR_PANEL     lv_color_hex(0x0A1728)
#define COLOR_PANEL_2   lv_color_hex(0x0E2238)
#define COLOR_PANEL_3   lv_color_hex(0x061220)

#define COLOR_BORDER    lv_color_hex(0x0C6D9E)
#define COLOR_BORDER_DIM lv_color_hex(0x0A314A)

#define COLOR_TEXT      lv_color_hex(0xE8EEF8)
#define COLOR_SUB_TEXT  lv_color_hex(0x9AA8C1)

#define COLOR_BLUE      lv_color_hex(0x168BFF)
#define COLOR_CYAN      lv_color_hex(0x00E5FF)
#define COLOR_GREEN     lv_color_hex(0x20F58A)
#define COLOR_YELLOW    lv_color_hex(0xFFC928)
#define COLOR_ORANGE    lv_color_hex(0xFF8C00)
#define COLOR_RED       lv_color_hex(0xFF365F)

/* =========================================================
 * Fonts
 * ========================================================= */
#define FONT_SMALL      (&lv_font_montserrat_14)
#define FONT_NORMAL     (&lv_font_montserrat_16)
#define FONT_MEDIUM     (&lv_font_montserrat_20)
#define FONT_LARGE      (&lv_font_montserrat_28)
#define FONT_NUMBER     (&lv_font_montserrat_38)

/* =========================================================
 * Layout
 * ========================================================= */
#define HEADER_H        62
#define SIDEBAR_W       150

#define PAGE_X          160
#define PAGE_Y          74
#define PAGE_W          850
#define PAGE_H          510

/* =========================================================
 * Page enum
 * ========================================================= */
typedef enum {
    PAGE_HOME = 0,
    PAGE_DETECTION,
    PAGE_ENVIRONMENT,
    PAGE_RECORDS,
    PAGE_SETTINGS,
    PAGE_COUNT
} UiPage;

/* =========================================================
 * Global objects
 * ========================================================= */
static lv_obj_t *pages[PAGE_COUNT];
static lv_obj_t *nav_btns[PAGE_COUNT];

static lv_obj_t *label_time;
static lv_obj_t *label_date;

static FanCtrl *g_fan_ctrl = NULL;
static bool g_updating_slider = false;

static lv_obj_t *danger_flash_layer = NULL;
static int danger_flash_count = 0;

static int last_auto_popup_level = 0;
static uint32_t last_auto_popup_tick = 0;

static int normal_behavior_count = 0;
static int fan_boost_active = 0;
static int ai_popup_auto_opened = 0;

/* =========================================================
 * Camera preview
 * SharedFrame: 1280x720 RGB888 -> LVGL image
 * ========================================================= */
#define CAM_PREVIEW_W   610
#define CAM_PREVIEW_H   343
#define CAM_IMG_Y_OFF   9

/* =========================================================
 * Camera view mode
 * 外部预览框固定，内部图像做裁剪/缩放
 * ========================================================= */
typedef enum {
    CAMERA_VIEW_WIDE = 0,
    CAMERA_VIEW_NORMAL,
    CAMERA_VIEW_FOCUS,
    CAMERA_VIEW_COUNT
} CameraViewMode;

static CameraViewMode g_camera_view_mode = CAMERA_VIEW_WIDE;

/*
 * Wide   = 完整画面，保持当前效果
 * Normal = 轻微裁剪放大
 * Focus  = 明显裁剪放大，突出脸部/上半身
 */
static const float camera_view_crop_ratio[CAMERA_VIEW_COUNT] = {
    1.00f,   /* Wide */
    0.88f,   /* Normal */
    0.72f    /* Focus */
};

/*
 * y_bias 控制裁剪区域上下位置：
 * 0.50 = 居中
 * 0.38 = 稍微偏上，更适合驾驶员头肩区域
 */
static const float camera_view_y_bias[CAMERA_VIEW_COUNT] = {
    0.50f,   /* Wide */
    0.42f,   /* Normal */
    0.36f    /* Focus */
};

static lv_obj_t *view_mode_btns[CAMERA_VIEW_COUNT];
static lv_obj_t *view_mode_label = NULL;

/* 当前裁剪区域，Overlay 映射也用它 */
static int g_crop_x = 0;
static int g_crop_y = 0;
static int g_crop_w = 0;
static int g_crop_h = 0;

/* 前置声明：ui_update_detection_overlay() 会提前调用它 */
static void camera_compute_crop_rect(int src_w, int src_h);

static void show_ai_copilot_popup_event_cb(lv_event_t *e);
static void ai_result_timer_cb(lv_timer_t *timer);

static SharedFrame *g_camera_frame = NULL;
static BehaviorResult *g_behavior_result = NULL;

static lv_obj_t *camera_img = NULL;
static lv_obj_t *camera_preview_panel = NULL;
static lv_obj_t *camera_overlay = NULL;

static lv_obj_t *camera_placeholder_1 = NULL;
static lv_obj_t *camera_placeholder_2 = NULL;

static lv_img_dsc_t camera_img_dsc;
static lv_color_t *camera_preview_buf = NULL;

static int last_camera_frame_count = -1;



/* Overlay 开关 */
static bool g_overlay_enabled = true;
static lv_obj_t *overlay_switch = NULL;
static lv_obj_t *overlay_state_label = NULL;


/* 最多 3 张人脸，每张脸 1 个框 + 5 个关键点 */
#define MAX_UI_FACES 3
#define FACE_POINTS  5

static lv_obj_t *face_boxes[MAX_UI_FACES];
static lv_obj_t *face_points[MAX_UI_FACES][FACE_POINTS];

/* 姿态关键点：YOLOv8-Pose / COCO 17 points */
#define POSE_POINTS 17
static lv_obj_t *pose_points[POSE_POINTS];

/* 姿态骨架线：COCO 17 points skeleton */
#define SKELETON_LINES 16

static lv_obj_t *pose_lines[SKELETON_LINES];

static const int skeleton_pairs[SKELETON_LINES][2] = {
    {0, 1},   /* nose - left eye */
    {0, 2},   /* nose - right eye */
    {1, 3},   /* left eye - left ear */
    {2, 4},   /* right eye - right ear */

    {5, 6},   /* left shoulder - right shoulder */
    {5, 7},   /* left shoulder - left elbow */
    {7, 9},   /* left elbow - left wrist */
    {6, 8},   /* right shoulder - right elbow */
    {8, 10},  /* right elbow - right wrist */

    {5, 11},  /* left shoulder - left hip */
    {6, 12},  /* right shoulder - right hip */
    {11, 12}, /* left hip - right hip */

    {11, 13}, /* left hip - left knee */
    {13, 15}, /* left knee - left ankle */
    {12, 14}, /* right hip - right knee */
    {14, 16}  /* right knee - right ankle */
};

/* Home */
static lv_obj_t *home_state_arc;
static lv_obj_t *home_state_icon;
static lv_obj_t *home_state_label;
static lv_obj_t *home_state_desc;
static lv_obj_t *home_score_label;
static lv_obj_t *home_score_bar;
static lv_obj_t *home_head_pill;
static lv_obj_t *home_phone_pill;
static lv_obj_t *home_smoke_pill;
static lv_obj_t *home_fan_mode;
static lv_obj_t *home_fan_speed;
static lv_obj_t *home_comfort;
static lv_obj_t *home_chip_temp;
static lv_obj_t *home_alert_text;
static lv_obj_t *home_wheel_pill = NULL;

/* Detection */
static lv_obj_t *det_eye_state;
static lv_obj_t *det_perclos;
static lv_obj_t *det_blink;
static lv_obj_t *det_yawn;
static lv_obj_t *det_head_pill;
static lv_obj_t *det_phone_pill;
static lv_obj_t *det_smoke_pill;
static lv_obj_t *det_risk_text;
static lv_obj_t *det_explain_text;
static lv_obj_t *detect_risk_score_label = NULL;
static lv_obj_t *detect_risk_score_text_label = NULL;
static lv_obj_t *det_driver_state_label = NULL;
static lv_obj_t *det_vehicle_state = NULL;

/* Environment */
static lv_obj_t *env_temp_label;
static lv_obj_t *env_humi_label;
static lv_obj_t *env_comfort_label;
static lv_obj_t *env_fan_mode;
static lv_obj_t *env_fan_speed;
static lv_obj_t *env_chip_temp;
static lv_obj_t *env_fan_arc;
static lv_obj_t *slider_fan;

/* Records */
static lv_obj_t *rec_chart;
static lv_chart_series_t *rec_score_series;


#define MAX_EVENT_RECORDS 5
#define EVENT_TEXT_LEN    96

static lv_obj_t *rec_event_labels[MAX_EVENT_RECORDS];
static char rec_event_texts[MAX_EVENT_RECORDS][EVENT_TEXT_LEN];
static int rec_event_count = 0;

#define REC_TIME_LABELS 5
static lv_obj_t *rec_time_labels[REC_TIME_LABELS];

#define EVENT_LOG_FILE "/home/elf/rknn_model_zoo/rknn_model_zoo-2.1.0/examples/yolov8_pose/cpp/logs/events.csv"

#define HISTORY_MAX_LINES 20
#define HISTORY_LINE_LEN  256

static lv_obj_t *history_popup = NULL;
static lv_obj_t *history_line_labels[HISTORY_MAX_LINES];

/* =========================================================
 * AI Copilot popup
 * ========================================================= */
static lv_obj_t *ai_popup = NULL;
static lv_obj_t *ai_answer_box = NULL;
static lv_obj_t *ai_answer_label = NULL;
static lv_obj_t *ai_status_label = NULL;
static char ai_last_answer_cache[4096] = {0};
//static lv_obj_t *ai_policy_label = NULL;

/* 用于防止同一个状态每次刷新都重复记录 */
static int last_head_state = -1;
static int last_phone_state = -1;
static int last_smoking_state = -1;
static int last_fatigue_level = -1;

/* Settings */
static lv_obj_t *setting_voice_state;
static lv_obj_t *sw_voice;

//背光
static void brightness_slider_event_cb(lv_event_t *e);

/* System Metrics */
static lv_obj_t *metric_runtime_label;
static lv_obj_t *metric_camera_fps_label;
static lv_obj_t *metric_ai_fps_label;
static lv_obj_t *metric_infer_ms_label;
static lv_obj_t *metric_preview_fps_label;
static lv_obj_t *metric_cpu_label;
static lv_obj_t *metric_stability_label;

static uint32_t metric_start_tick = 0;
static int power_shutdown_dialog_shown = 0;
static int camera_preview_frame_count = 0;

/* Shutdown confirm dialog */
static lv_obj_t *shutdown_confirm_mask = NULL;
static lv_obj_t *shutdown_confirm_box  = NULL;

static lv_obj_t *setting_vehicle_mode_label = NULL;
static lv_obj_t *btn_vehicle_mode = NULL;
static int g_vehicle_mode = 0;   /* 0=AUTO, 1=DRIVING, 2=PARKED */

#define POWER_RESTART_HOLD_MS   1800
#define POWER_SHUTDOWN_HOLD_MS  5500

static uint32_t power_press_tick = 0;

static void power_main_btn_event_cb(lv_event_t *e);
static void show_shutdown_confirm_dialog(void);
static void shutdown_confirm_yes_cb(lv_event_t *e);
static void shutdown_confirm_cancel_cb(lv_event_t *e);

/* =========================================================
 * Utility
 * ========================================================= */
void ui_set_fan_ctrl(FanCtrl *ctrl)
{
    g_fan_ctrl = ctrl;
}

void ui_set_camera_frame(SharedFrame *frame)
{
    g_camera_frame = frame;
}

void ui_set_behavior_result(BehaviorResult *result)
{
    g_behavior_result = result;
}

static int clamp_int(int value, int min, int max)
{
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

static void set_label_color(lv_obj_t *label, lv_color_t color)
{
    if (label) {
        lv_obj_set_style_text_color(label, color, 0);
    }
}

static lv_obj_t *label_create(lv_obj_t *parent,
                              const char *txt,
                              int x,
                              int y,
                              lv_color_t color,
                              const lv_font_t *font)
{
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, txt);
    lv_obj_set_pos(label, x, y);
    lv_obj_set_style_text_color(label, color, 0);
    lv_obj_set_style_text_font(label, font, 0);
    return label;
}

static lv_obj_t *glass_panel_create(lv_obj_t *parent, int x, int y, int w, int h)
{
    lv_obj_t *obj = lv_obj_create(parent);
    lv_obj_set_size(obj, w, h);
    lv_obj_set_pos(obj, x, y);

    lv_obj_set_style_bg_color(obj, COLOR_PANEL, 0);
    lv_obj_set_style_bg_opa(obj, OPACITY_70, 0);
    lv_obj_set_style_radius(obj, 24, 0);

    lv_obj_set_style_border_width(obj, 1, 0);
    lv_obj_set_style_border_color(obj, COLOR_BORDER_DIM, 0);

    lv_obj_set_style_shadow_width(obj, 18, 0);
    lv_obj_set_style_shadow_color(obj, lv_color_hex(0x003A66), 0);
    lv_obj_set_style_shadow_opa(obj, OPACITY_35, 0);

    lv_obj_set_style_pad_all(obj, 0, 0);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);

    return obj;
}

static lv_obj_t *soft_panel_create(lv_obj_t *parent, int x, int y, int w, int h)
{
    lv_obj_t *obj = lv_obj_create(parent);
    lv_obj_set_size(obj, w, h);
    lv_obj_set_pos(obj, x, y);

    lv_obj_set_style_bg_color(obj, COLOR_PANEL_2, 0);
    lv_obj_set_style_bg_opa(obj, OPACITY_55, 0);
    lv_obj_set_style_radius(obj, 18, 0);

    lv_obj_set_style_border_width(obj, 1, 0);
    lv_obj_set_style_border_color(obj, COLOR_BORDER_DIM, 0);

    lv_obj_set_style_pad_all(obj, 0, 0);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);

    return obj;
}

static void title_create(lv_obj_t *parent, const char *title, int x, int y)
{
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, title);
    lv_obj_set_pos(label, x, y);
    lv_obj_set_style_text_color(label, COLOR_TEXT, 0);
    lv_obj_set_style_text_font(label, FONT_MEDIUM, 0);
}

static void glow_line_create(lv_obj_t *parent, int x, int y, int w)
{
    lv_obj_t *line = lv_obj_create(parent);
    lv_obj_set_size(line, w, 2);
    lv_obj_set_pos(line, x, y);
    lv_obj_set_style_bg_color(line, COLOR_CYAN, 0);
    lv_obj_set_style_bg_opa(line, OPACITY_55, 0);
    lv_obj_set_style_border_width(line, 0, 0);
    lv_obj_set_style_shadow_width(line, 12, 0);
    lv_obj_set_style_shadow_color(line, COLOR_CYAN, 0);
    lv_obj_set_style_shadow_opa(line, OPACITY_45, 0);
}

static void decorative_curve_lines(lv_obj_t *parent)
{
    /*
     * Pure LVGL cannot draw the same irregular curved glass outlines
     * as concept art, but several thin translucent lines can reduce
     * the "boxy" feeling.
     */
    // glow_line_create(parent, 20, 20, 180);
    // glow_line_create(parent, 650, 20, 180);
    // glow_line_create(parent, 40, PAGE_H - 25, 220);
    // glow_line_create(parent, 580, PAGE_H - 25, 240);
    (void)parent;
}

static lv_obj_t *pill_create(lv_obj_t *parent, int x, int y, int w, const char *txt)
{
    lv_obj_t *pill = lv_obj_create(parent);
    lv_obj_set_size(pill, w, 32);
    lv_obj_set_pos(pill, x, y);
    lv_obj_set_style_radius(pill, 16, 0);
    lv_obj_set_style_bg_color(pill, lv_color_hex(0x092C25), 0);
    lv_obj_set_style_bg_opa(pill, OPACITY_70, 0);
    lv_obj_set_style_border_width(pill, 1, 0);
    lv_obj_set_style_border_color(pill, COLOR_GREEN, 0);
    lv_obj_clear_flag(pill, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *label = lv_label_create(pill);
    lv_label_set_text(label, txt);
    lv_obj_set_style_text_color(label, COLOR_GREEN, 0);
    lv_obj_set_style_text_font(label, FONT_SMALL, 0);
    lv_obj_center(label);

    return label;
}

static void pill_update(lv_obj_t *label, int detected)
{
    if (!label) return;

    lv_obj_t *pill = lv_obj_get_parent(label);
    if (!pill) return;

    /*
     * detected:
     * -1 = No Driver / Unknown
     *  0 = Normal
     *  1 = Detected
     */
    if (detected < 0) {
        lv_label_set_text(label, "--");
        lv_obj_set_style_text_color(label, COLOR_SUB_TEXT, 0);
        lv_obj_set_style_border_color(pill, COLOR_BORDER_DIM, 0);
        lv_obj_set_style_bg_color(pill, lv_color_hex(0x101826), 0);
    } else if (detected) {
        lv_label_set_text(label, "Detected");
        lv_obj_set_style_text_color(label, COLOR_RED, 0);
        lv_obj_set_style_border_color(pill, COLOR_RED, 0);
        lv_obj_set_style_bg_color(pill, lv_color_hex(0x361224), 0);
    } else {
        lv_label_set_text(label, "Normal");
        lv_obj_set_style_text_color(label, COLOR_GREEN, 0);
        lv_obj_set_style_border_color(pill, COLOR_GREEN, 0);
        lv_obj_set_style_bg_color(pill, lv_color_hex(0x092C25), 0);
    }
}

/* =========================================================
 * Navigation helpers
 * ========================================================= */
static void page_show(UiPage page);

static void nav_event_cb(lv_event_t *e)
{
    UiPage page = (UiPage)(uintptr_t)lv_event_get_user_data(e);
    page_show(page);
}

/* =========================================================
 * Header
 * ========================================================= */
static void create_header(lv_obj_t *scr)
{
    lv_obj_t *top = lv_obj_create(scr);
    lv_obj_set_size(top, SCREEN_W, HEADER_H);
    lv_obj_set_pos(top, 0, 0);
    lv_obj_set_style_bg_color(top, COLOR_HEADER, 0);
    lv_obj_set_style_bg_opa(top, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(top, 0, 0);
    lv_obj_set_style_pad_all(top, 0, 0);
    lv_obj_clear_flag(top, LV_OBJ_FLAG_SCROLLABLE);

    /* 左侧硬件状态 */
    label_create(top, "Camera OK", 20, 18, COLOR_GREEN, FONT_SMALL);
    label_create(top, "Mic OK", 140, 18, COLOR_CYAN, FONT_SMALL);
    label_create(top, "AHT20 OK", 235, 18, COLOR_YELLOW, FONT_SMALL);

    /* 中间标题 */
    lv_obj_t *title = lv_label_create(top);
    lv_label_set_text(title, "DRIVER WELLNESS & ENVIRONMENT SYSTEM");
    lv_obj_set_style_text_color(title, COLOR_TEXT, 0);
    lv_obj_set_style_text_font(title, FONT_SMALL, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 8);

    glow_line_create(top, 445, 46, 120);

    /* 右侧时间日期，避免和标题重叠 */
    label_time = label_create(top, "--:--:--", 840, 10, COLOR_CYAN, FONT_MEDIUM);
    label_date = label_create(top, "----/--/--", 840, 38, COLOR_SUB_TEXT, FONT_SMALL);
}

/* =========================================================
 * Sidebar navigation
 * ========================================================= */
static lv_obj_t *side_btn_create(lv_obj_t *parent,
                                 const char *icon,
                                 const char *text,
                                 int y,
                                 UiPage page)
{
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_size(btn, 128, 54);
    lv_obj_set_pos(btn, 11, y);

    lv_obj_set_style_radius(btn, 16, 0);
    lv_obj_set_style_bg_color(btn, COLOR_PANEL_3, 0);
    lv_obj_set_style_bg_opa(btn, OPACITY_70, 0);
    lv_obj_set_style_border_width(btn, 1, 0);
    lv_obj_set_style_border_color(btn, COLOR_BORDER_DIM, 0);

    lv_obj_t *label_icon = lv_label_create(btn);
    lv_label_set_text(label_icon, icon);
    lv_obj_set_style_text_color(label_icon, COLOR_CYAN, 0);
    lv_obj_set_style_text_font(label_icon, FONT_NORMAL, 0);
    lv_obj_set_pos(label_icon, 14, 17);

    lv_obj_t *label_text = lv_label_create(btn);
    lv_label_set_text(label_text, text);
    lv_obj_set_style_text_color(label_text, COLOR_TEXT, 0);
    lv_obj_set_style_text_font(label_text, FONT_SMALL, 0);
    lv_obj_set_pos(label_text, 42, 17);

    lv_obj_add_event_cb(btn, nav_event_cb, LV_EVENT_CLICKED, (void *)(uintptr_t)page);
    nav_btns[page] = btn;

    return btn;
}

static void create_sidebar(lv_obj_t *scr)
{
    lv_obj_t *bar = lv_obj_create(scr);
    lv_obj_set_size(bar, SIDEBAR_W, SCREEN_H - HEADER_H);
    lv_obj_set_pos(bar, 0, HEADER_H);
    lv_obj_set_style_bg_color(bar, COLOR_SIDEBAR, 0);
    lv_obj_set_style_bg_opa(bar, OPACITY_70, 0);
    lv_obj_set_style_border_width(bar, 0, 0);
    lv_obj_set_style_pad_all(bar, 0, 0);
    lv_obj_clear_flag(bar, LV_OBJ_FLAG_SCROLLABLE);

    side_btn_create(bar, LV_SYMBOL_HOME,     "Home",    35,  PAGE_HOME);
    side_btn_create(bar, LV_SYMBOL_EYE_OPEN, "Detect",  105, PAGE_DETECTION);
    side_btn_create(bar, LV_SYMBOL_REFRESH,  "Env",     175, PAGE_ENVIRONMENT);
    side_btn_create(bar, LV_SYMBOL_LIST,     "Record",  245, PAGE_RECORDS);
    side_btn_create(bar, LV_SYMBOL_SETTINGS, "Setting", 315, PAGE_SETTINGS);

    label_create(bar, "System", 28, 430, COLOR_SUB_TEXT, FONT_SMALL);
    label_create(bar, "OK", 28, 455, COLOR_GREEN, FONT_NORMAL);
    label_create(bar, "All normal", 28, 480, COLOR_SUB_TEXT, FONT_SMALL);
}

static void nav_set_active(UiPage page)
{
    for (int i = 0; i < PAGE_COUNT; i++) {
        if (!nav_btns[i]) continue;

        if (i == (int)page) {
            lv_obj_set_style_bg_color(nav_btns[i], COLOR_BLUE, 0);
            lv_obj_set_style_border_color(nav_btns[i], COLOR_CYAN, 0);
            lv_obj_set_style_shadow_width(nav_btns[i], 16, 0);
            lv_obj_set_style_shadow_color(nav_btns[i], COLOR_CYAN, 0);
            lv_obj_set_style_shadow_opa(nav_btns[i], OPACITY_55, 0);
        } else {
            lv_obj_set_style_bg_color(nav_btns[i], COLOR_PANEL_3, 0);
            lv_obj_set_style_border_color(nav_btns[i], COLOR_BORDER_DIM, 0);
            lv_obj_set_style_shadow_width(nav_btns[i], 0, 0);
        }
    }
}

static void page_show(UiPage page)
{
    for (int i = 0; i < PAGE_COUNT; i++) {
        if (!pages[i]) continue;

        if (i == (int)page) {
            lv_obj_clear_flag(pages[i], LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(pages[i], LV_OBJ_FLAG_HIDDEN);
        }
    }

    nav_set_active(page);
}

/* =========================================================
 * Home page
 * ========================================================= */
static void create_home_page(lv_obj_t *scr)
{
    lv_obj_t *page = lv_obj_create(scr);
    pages[PAGE_HOME] = page;

    lv_obj_set_size(page, PAGE_W, PAGE_H);
    lv_obj_set_pos(page, PAGE_X, PAGE_Y);
    lv_obj_set_style_bg_opa(page, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(page, 0, 0);
    lv_obj_set_style_pad_all(page, 0, 0);
    lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);

    decorative_curve_lines(page);

    lv_obj_t *main = glass_panel_create(page, 0, 0, 465, 440);
    title_create(main, "1. HOME", 28, 18);

    home_state_arc = lv_arc_create(main);
    lv_obj_set_size(home_state_arc, 295, 295);
    lv_obj_set_pos(home_state_arc, 82, 58);
    lv_arc_set_range(home_state_arc, 0, 100);
    lv_arc_set_value(home_state_arc, 20);
    lv_arc_set_bg_angles(home_state_arc, 0, 360);
    lv_obj_remove_style(home_state_arc, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(home_state_arc, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_arc_width(home_state_arc, 16, LV_PART_MAIN);
    lv_obj_set_style_arc_width(home_state_arc, 16, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(home_state_arc, lv_color_hex(0x0D2D44), LV_PART_MAIN);
    lv_obj_set_style_arc_color(home_state_arc, COLOR_GREEN, LV_PART_INDICATOR);

    home_state_icon = label_create(main, ":)", 206, 142, COLOR_GREEN, FONT_LARGE);
    home_state_label = label_create(main, "NORMAL", 155, 220, COLOR_GREEN, FONT_LARGE);
    label_create(main, "Drive Safely", 188, 260, COLOR_GREEN, FONT_SMALL);
    // home_state_desc = label_create(main, "Driver status is good. Keep focused.", 92, 342, COLOR_SUB_TEXT, FONT_SMALL);
    // home_alert_text = label_create(main, "No abnormal alert", 170, 372, COLOR_GREEN, FONT_SMALL);
    home_state_desc = label_create(main,
                                "Driver status is good. Keep focused.",
                                40, 355,
                                COLOR_SUB_TEXT,
                                FONT_NORMAL);
    lv_obj_set_width(home_state_desc, 385);
    lv_obj_set_style_text_align(home_state_desc, LV_TEXT_ALIGN_CENTER, 0);

    home_alert_text = label_create(main,
                                "No abnormal alert",
                                40, 390,
                                COLOR_GREEN,
                                FONT_NORMAL);
    lv_obj_set_width(home_alert_text, 385);
    lv_obj_set_style_text_align(home_alert_text, LV_TEXT_ALIGN_CENTER, 0);

    lv_obj_t *score = glass_panel_create(page, 490, 0, 350, 130);
    title_create(score, "FATIGUE SCORE", 24, 14);

    home_score_label = label_create(score, "0", 105, 40, COLOR_GREEN, FONT_NUMBER);
    label_create(score, "Normal", 118, 84, COLOR_GREEN, FONT_SMALL);

    home_score_bar = lv_bar_create(score);
    lv_obj_set_size(home_score_bar, 250, 10);
    lv_obj_set_pos(home_score_bar, 48, 104);
    lv_bar_set_range(home_score_bar, 0, 100);
    lv_bar_set_value(home_score_bar, 0, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(home_score_bar, lv_color_hex(0x20364A), LV_PART_MAIN);
    lv_obj_set_style_bg_color(home_score_bar, COLOR_GREEN, LV_PART_INDICATOR);
    lv_obj_set_style_radius(home_score_bar, 5, LV_PART_MAIN);
    lv_obj_set_style_radius(home_score_bar, 5, LV_PART_INDICATOR);

    lv_obj_t *summary = glass_panel_create(page, 490, 150, 350, 125);
    title_create(summary, "DETECTION SUMMARY", 24, 14);

    label_create(summary, "Head", 28, 58, COLOR_SUB_TEXT, FONT_SMALL);
    home_head_pill = pill_create(summary, 80, 50, 80, "Normal");
    label_create(summary, "Phone", 180, 58, COLOR_SUB_TEXT, FONT_SMALL);
    home_phone_pill = pill_create(summary, 245, 50, 80, "Normal");
    label_create(summary, "Mouth", 28, 98, COLOR_SUB_TEXT, FONT_SMALL);
    home_smoke_pill = pill_create(summary, 80, 90, 80, "Normal");
    label_create(summary, "Wheel", 180, 98, COLOR_SUB_TEXT, FONT_SMALL);
    home_wheel_pill = pill_create(summary, 245, 90, 80, "On");

    lv_obj_t *sys = glass_panel_create(page, 490, 295, 350, 145);
    title_create(sys, "SYSTEM SUMMARY", 24, 14);

    label_create(sys, "Fan", 28, 60, COLOR_SUB_TEXT, FONT_SMALL);
    home_fan_mode = label_create(sys, "Auto", 28, 84, COLOR_GREEN, FONT_NORMAL);

    label_create(sys, "Speed", 118, 60, COLOR_SUB_TEXT, FONT_SMALL);
    home_fan_speed = label_create(sys, "0%", 118, 84, COLOR_CYAN, FONT_NORMAL);

    label_create(sys, "Comfort", 215, 60, COLOR_SUB_TEXT, FONT_SMALL);
    home_comfort = label_create(sys, "Good", 215, 84, COLOR_GREEN, FONT_NORMAL);

    label_create(sys, "Chip", 28, 112, COLOR_SUB_TEXT, FONT_SMALL);
    home_chip_temp = label_create(sys, "--C", 78, 112, COLOR_YELLOW, FONT_SMALL);
}

/* =========================================================
 * Detection page
 * ========================================================= */
static int clamp_coord(int v, int min_v, int max_v)
{
    if (v < min_v) return min_v;
    if (v > max_v) return max_v;
    return v;
}

static void hide_all_overlay_objects(void)
{
    for (int i = 0; i < MAX_UI_FACES; i++) {
        if (face_boxes[i]) {
            lv_obj_add_flag(face_boxes[i], LV_OBJ_FLAG_HIDDEN);
        }

        for (int j = 0; j < FACE_POINTS; j++) {
            if (face_points[i][j]) {
                lv_obj_add_flag(face_points[i][j], LV_OBJ_FLAG_HIDDEN);
            }
        }
    }

    for (int i = 0; i < POSE_POINTS; i++) {
        if (pose_points[i]) {
            lv_obj_add_flag(pose_points[i], LV_OBJ_FLAG_HIDDEN);
        }
    }

    for (int i = 0; i < SKELETON_LINES; i++) {
        if (pose_lines[i]) {
            lv_obj_add_flag(pose_lines[i], LV_OBJ_FLAG_HIDDEN);
        }
    }
}

//骨架线刷新函数
static int pose_valid[POSE_POINTS];
static lv_point_t pose_line_points[SKELETON_LINES][2];

static void update_pose_skeleton_lines(void)
{
    for (int i = 0; i < SKELETON_LINES; i++) {
        int p1 = skeleton_pairs[i][0];
        int p2 = skeleton_pairs[i][1];

        if (!pose_lines[i]) {
            continue;
        }

        if (!pose_valid[p1] || !pose_valid[p2]) {
            lv_obj_add_flag(pose_lines[i], LV_OBJ_FLAG_HIDDEN);
            continue;
        }

        int x1 = lv_obj_get_x(pose_points[p1]) + 3;
        int y1 = lv_obj_get_y(pose_points[p1]) + 3;
        int x2 = lv_obj_get_x(pose_points[p2]) + 3;
        int y2 = lv_obj_get_y(pose_points[p2]) + 3;

        pose_line_points[i][0].x = x1;
        pose_line_points[i][0].y = y1;
        pose_line_points[i][1].x = x2;
        pose_line_points[i][1].y = y2;

        lv_line_set_points(pose_lines[i], pose_line_points[i], 2);
        lv_obj_clear_flag(pose_lines[i], LV_OBJ_FLAG_HIDDEN);
    }
}

void ui_update_detection_overlay(void)
{
    if (!camera_overlay || !g_behavior_result || !g_camera_frame) {
        return;
    }

    if (!g_overlay_enabled) {
        hide_all_overlay_objects();
        return;
    }

    int src_w = g_camera_frame->width;
    int src_h = g_camera_frame->height;

    if (src_w <= 0 || src_h <= 0) {
        src_w = CAM_WIDTH;
        src_h = CAM_HEIGHT;
    }

    camera_compute_crop_rect(src_w, src_h);

    float scale_x = (float)CAM_PREVIEW_W / (float)g_crop_w;
    float scale_y = (float)CAM_PREVIEW_H / (float)g_crop_h;

    hide_all_overlay_objects();

    /* ===================== 人脸框 + 5点关键点 ===================== */
    if (g_behavior_result->face_detected && g_behavior_result->face_count > 0) {
        int count = g_behavior_result->face_count;
        if (count > MAX_UI_FACES) {
            count = MAX_UI_FACES;
        }

        for (int i = 0; i < count; i++) {
            FaceInfo *face = &g_behavior_result->faces[i];

            int x1 = (int)((face->left   - g_crop_x) * scale_x);
            int y1 = (int)((face->top    - g_crop_y) * scale_y);
            int x2 = (int)((face->right  - g_crop_x) * scale_x);
            int y2 = (int)((face->bottom - g_crop_y) * scale_y);

            x1 = clamp_coord(x1, 0, CAM_PREVIEW_W - 1);
            y1 = clamp_coord(y1, 0, CAM_PREVIEW_H - 1);
            x2 = clamp_coord(x2, 0, CAM_PREVIEW_W - 1);
            y2 = clamp_coord(y2, 0, CAM_PREVIEW_H - 1);

            int w = x2 - x1;
            int h = y2 - y1;

            if (w >= 5 && h >= 5 && face_boxes[i]) {
                lv_obj_clear_flag(face_boxes[i], LV_OBJ_FLAG_HIDDEN);
                lv_obj_set_pos(face_boxes[i], x1, y1);
                lv_obj_set_size(face_boxes[i], w, h);
            }

            for (int j = 0; j < FACE_POINTS; j++) {
                if (!face_points[i][j]) continue;

                int px = (int)((face->points[j][0] - g_crop_x) * scale_x);
                int py = (int)((face->points[j][1] - g_crop_y) * scale_y);

                px = clamp_coord(px, 0, CAM_PREVIEW_W - 1);
                py = clamp_coord(py, 0, CAM_PREVIEW_H - 1);

                lv_obj_clear_flag(face_points[i][j], LV_OBJ_FLAG_HIDDEN);
                lv_obj_set_pos(face_points[i][j], px - 3, py - 3);

                if (j == 0 || j == 1) {
                    lv_obj_set_style_bg_color(face_points[i][j], COLOR_CYAN, 0);
                } else if (j == 2) {
                    lv_obj_set_style_bg_color(face_points[i][j], COLOR_YELLOW, 0);
                } else {
                    lv_obj_set_style_bg_color(face_points[i][j], COLOR_GREEN, 0);
                }
            }
        }
    }

/* ===================== 姿态 17 个关键点 + 骨架线 ===================== */
for (int i = 0; i < POSE_POINTS; i++) {
    pose_valid[i] = 0;
}

if (g_behavior_result->pose_detected) {
    for (int i = 0; i < POSE_POINTS; i++) {
        if (!pose_points[i]) continue;

        float raw_x = g_behavior_result->pose_keypoints[i][0];
        float raw_y = g_behavior_result->pose_keypoints[i][1];

        /*
         * 无效点过滤：
         * 模型未检测到时，可能是 0,0。
         */
        if (raw_x <= 1.0f && raw_y <= 1.0f) {
            continue;
        }

        if (raw_x < g_crop_x || raw_x > g_crop_x + g_crop_w ||
            raw_y < g_crop_y || raw_y > g_crop_y + g_crop_h) {
            continue;
        }

        int px = (int)((raw_x - g_crop_x) * scale_x);
        int py = (int)((raw_y - g_crop_y) * scale_y);

        px = clamp_coord(px, 0, CAM_PREVIEW_W - 1);
        py = clamp_coord(py, 0, CAM_PREVIEW_H - 1);

        lv_obj_clear_flag(pose_points[i], LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(pose_points[i], px - 3, py - 3);

        pose_valid[i] = 1;

        if (i <= 4) {
            lv_obj_set_style_bg_color(pose_points[i], COLOR_CYAN, 0);
        } else if (i <= 10) {
            lv_obj_set_style_bg_color(pose_points[i], COLOR_GREEN, 0);
        } else {
            lv_obj_set_style_bg_color(pose_points[i], COLOR_ORANGE, 0);
        }
    }

    update_pose_skeleton_lines();
}

}

static void overlay_switch_event_cb(lv_event_t *e)
{
    lv_obj_t *sw = lv_event_get_target(e);
    g_overlay_enabled = lv_obj_has_state(sw, LV_STATE_CHECKED);

    if (overlay_state_label) {
        lv_label_set_text(overlay_state_label, g_overlay_enabled ? "On" : "Off");
        lv_obj_set_style_text_color(overlay_state_label,
                                    g_overlay_enabled ? COLOR_GREEN : COLOR_SUB_TEXT,
                                    0);
    }

    ui_update_detection_overlay();
}

static void update_view_mode_buttons(void)
{
    for (int i = 0; i < CAMERA_VIEW_COUNT; i++) {
        if (!view_mode_btns[i]) continue;

        if (i == (int)g_camera_view_mode) {
            lv_obj_set_style_bg_color(view_mode_btns[i], COLOR_BLUE, 0);
            lv_obj_set_style_border_color(view_mode_btns[i], COLOR_CYAN, 0);
            lv_obj_set_style_shadow_width(view_mode_btns[i], 12, 0);
            lv_obj_set_style_shadow_color(view_mode_btns[i], COLOR_CYAN, 0);
            lv_obj_set_style_shadow_opa(view_mode_btns[i], OPACITY_45, 0);
        } else {
            lv_obj_set_style_bg_color(view_mode_btns[i], COLOR_PANEL_2, 0);
            lv_obj_set_style_border_color(view_mode_btns[i], COLOR_BORDER_DIM, 0);
            lv_obj_set_style_shadow_width(view_mode_btns[i], 0, 0);
        }
    }

    if (view_mode_label) {
        if (g_camera_view_mode == CAMERA_VIEW_WIDE) {
            lv_label_set_text(view_mode_label, "Wide");
        } else if (g_camera_view_mode == CAMERA_VIEW_NORMAL) {
            lv_label_set_text(view_mode_label, "Normal");
        } else {
            lv_label_set_text(view_mode_label, "Focus");
        }
    }
}

static void view_mode_event_cb(lv_event_t *e)
{
    CameraViewMode mode = (CameraViewMode)(uintptr_t)lv_event_get_user_data(e);

    if (mode < 0 || mode >= CAMERA_VIEW_COUNT) {
        return;
    }

    g_camera_view_mode = mode;
    update_view_mode_buttons();

    /*
     * 强制下一帧刷新图像，否则如果 frame_count 没变，
     * 画面可能不会立刻切换。
     */
    last_camera_frame_count = -1;

    ui_update_camera_preview();
    ui_update_detection_overlay();
}

static lv_obj_t *view_mode_btn_create(lv_obj_t *parent,
                                      const char *txt,
                                      int x,
                                      int y,
                                      CameraViewMode mode)
{
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_size(btn, 72, 30);
    lv_obj_set_pos(btn, x, y);

    lv_obj_set_style_radius(btn, 14, 0);
    lv_obj_set_style_bg_color(btn, COLOR_PANEL_2, 0);
    lv_obj_set_style_bg_opa(btn, OPACITY_70, 0);
    lv_obj_set_style_border_width(btn, 1, 0);
    lv_obj_set_style_border_color(btn, COLOR_BORDER_DIM, 0);

    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, txt);
    lv_obj_set_style_text_font(label, FONT_SMALL, 0);
    lv_obj_set_style_text_color(label, COLOR_TEXT, 0);
    lv_obj_center(label);

    lv_obj_add_event_cb(btn,
                        view_mode_event_cb,
                        LV_EVENT_CLICKED,
                        (void *)(uintptr_t)mode);

    view_mode_btns[mode] = btn;

    return btn;
}

static void create_overlay_objects(lv_obj_t *parent)
{
    camera_overlay = lv_obj_create(parent);
    lv_obj_set_size(camera_overlay, CAM_PREVIEW_W, CAM_PREVIEW_H);
    lv_obj_set_pos(camera_overlay, 0, CAM_IMG_Y_OFF);

    lv_obj_set_style_bg_opa(camera_overlay, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(camera_overlay, 0, 0);
    lv_obj_set_style_pad_all(camera_overlay, 0, 0);
    lv_obj_clear_flag(camera_overlay, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(camera_overlay, LV_OBJ_FLAG_CLICKABLE);

    /* 人脸框 + 5点 */
    for (int i = 0; i < MAX_UI_FACES; i++) {
        face_boxes[i] = lv_obj_create(camera_overlay);
        lv_obj_set_size(face_boxes[i], 10, 10);
        lv_obj_set_pos(face_boxes[i], 0, 0);
        lv_obj_set_style_bg_opa(face_boxes[i], LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(face_boxes[i], 2, 0);
        lv_obj_set_style_border_color(face_boxes[i], COLOR_CYAN, 0);
        lv_obj_set_style_radius(face_boxes[i], 4, 0);
        lv_obj_clear_flag(face_boxes[i], LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(face_boxes[i], LV_OBJ_FLAG_HIDDEN);

        for (int j = 0; j < FACE_POINTS; j++) {
            face_points[i][j] = lv_obj_create(camera_overlay);
            lv_obj_set_size(face_points[i][j], 7, 7);
            lv_obj_set_pos(face_points[i][j], 0, 0);
            lv_obj_set_style_radius(face_points[i][j], 4, 0);
            lv_obj_set_style_bg_color(face_points[i][j], COLOR_GREEN, 0);
            lv_obj_set_style_bg_opa(face_points[i][j], LV_OPA_COVER, 0);
            lv_obj_set_style_border_width(face_points[i][j], 0, 0);
            lv_obj_clear_flag(face_points[i][j], LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_add_flag(face_points[i][j], LV_OBJ_FLAG_HIDDEN);
        }
    }

    /*
     * 先创建骨架线，再创建姿态点。
     * 这样点会显示在线上面，更清楚。
     */
    for (int i = 0; i < SKELETON_LINES; i++) {
        pose_lines[i] = lv_line_create(camera_overlay);

        static lv_point_t dummy_points[2] = {
            {0, 0},
            {1, 1}
        };

        lv_line_set_points(pose_lines[i], dummy_points, 2);
        lv_obj_set_style_line_width(pose_lines[i], 2, 0);
        lv_obj_set_style_line_color(pose_lines[i], COLOR_CYAN, 0);
        lv_obj_set_style_line_opa(pose_lines[i], LV_OPA_70, 0);
        lv_obj_add_flag(pose_lines[i], LV_OBJ_FLAG_HIDDEN);
    }

    /* 创建姿态 17 个关键点 */
    for (int i = 0; i < POSE_POINTS; i++) {
        pose_points[i] = lv_obj_create(camera_overlay);
        lv_obj_set_size(pose_points[i], 6, 6);
        lv_obj_set_pos(pose_points[i], 0, 0);
        lv_obj_set_style_radius(pose_points[i], 3, 0);
        lv_obj_set_style_bg_color(pose_points[i], COLOR_ORANGE, 0);
        lv_obj_set_style_bg_opa(pose_points[i], LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(pose_points[i], 0, 0);
        lv_obj_clear_flag(pose_points[i], LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(pose_points[i], LV_OBJ_FLAG_HIDDEN);
    }
}







static void create_detection_page(lv_obj_t *scr)
{
    lv_obj_t *page = lv_obj_create(scr);
    pages[PAGE_DETECTION] = page;

    lv_obj_set_size(page, PAGE_W, PAGE_H);
    lv_obj_set_pos(page, PAGE_X, PAGE_Y);
    lv_obj_set_style_bg_opa(page, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(page, 0, 0);
    lv_obj_set_style_pad_all(page, 0, 0);
    lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);

    decorative_curve_lines(page);

    /* 左侧大摄像头区域 */
    lv_obj_t *camera = glass_panel_create(page, 0, 0, 620, 440);
    title_create(camera, "2. DETECTION  /  LIVE DRIVER MONITORING", 28, 18);

    lv_obj_t *preview = lv_obj_create(camera);
    camera_preview_panel = preview;

    /*
     * 原来是 555 x 330。
     * 这里小幅放大到 575 x 345，不大改布局，比较安全。
     */
    lv_obj_set_size(preview, 610, 355);
    lv_obj_set_pos(preview, 5, 50);
    lv_obj_set_style_radius(preview, 20, 0);
    lv_obj_set_style_bg_color(preview, lv_color_hex(0x071423), 0);
    lv_obj_set_style_bg_opa(preview, OPACITY_70, 0);
    lv_obj_set_style_border_width(preview, 1, 0);
    lv_obj_set_style_border_color(preview, COLOR_BORDER_DIM, 0);
    lv_obj_set_style_pad_all(preview, 0, 0);
    lv_obj_clear_flag(preview, LV_OBJ_FLAG_SCROLLABLE);

    /*
     * 真实摄像头图像显示对象。
     * 注意：
     * 如果 CAM_PREVIEW_W / CAM_PREVIEW_H 仍然是 555 / 312，
     * 那图像本身不会明显变大，只是外框变大。
     * 如果你想图像也变大，后面再统一改 CAM_PREVIEW_W/H。
     */
    camera_img = lv_img_create(preview);
    lv_obj_set_size(camera_img, CAM_PREVIEW_W, CAM_PREVIEW_H);

    /*
     * 让图像在新的 preview 里居中一些。
     * 如果你的 CAM_IMG_Y_OFF 已经适配，可以继续用原来的。
     */
    //lv_obj_set_pos(camera_img, (600 - CAM_PREVIEW_W) / 2, CAM_IMG_Y_OFF);
    lv_obj_set_pos(camera_img, 0, 11);

    /*
     * Overlay 对象放在图像上层。
     */
    create_overlay_objects(preview);

    /* 占位文字：有画面后隐藏 */
    camera_placeholder_1 = label_create(preview,
                                        "CAMERA PREVIEW AREA",
                                        190,
                                        135,
                                        COLOR_CYAN,
                                        FONT_MEDIUM);

    camera_placeholder_2 = label_create(preview,
                                        "Waiting for SharedFrame...",
                                        190,
                                        175,
                                        COLOR_SUB_TEXT,
                                        FONT_SMALL);

    /* 四角 HUD 装饰线：跟随新 preview 尺寸调整 */
    glow_line_create(preview, 40, 35, 80);
    glow_line_create(preview, 490, 35, 80);
    glow_line_create(preview, 40, 325, 80);
    glow_line_create(preview, 490, 325, 80);

    /* Camera view mode：整体下移，利用底部空间 */
    label_create(camera, "View", 36, 420, COLOR_SUB_TEXT, FONT_SMALL);
    view_mode_label = label_create(camera, "Wide", 80, 420, COLOR_GREEN, FONT_SMALL);

    view_mode_btn_create(camera, "Wide",   150, 413, CAMERA_VIEW_WIDE);
    view_mode_btn_create(camera, "Normal", 240, 413, CAMERA_VIEW_NORMAL);
    view_mode_btn_create(camera, "Focus",  340, 413, CAMERA_VIEW_FOCUS);

    update_view_mode_buttons();

    /* 右上：驾驶状态 */
    lv_obj_t *state = glass_panel_create(page, 630, 0, 210, 200);
    title_create(state, "DRIVER STATE", 20, 14);

    det_risk_text = label_create(state, "Current Risk:", 28, 50, COLOR_SUB_TEXT, FONT_SMALL);
    //label_create(state, "Normal", 28, 72, COLOR_GREEN, FONT_LARGE);
    det_driver_state_label = label_create(state, "Normal", 28, 72, COLOR_GREEN, FONT_NORMAL);

    /*
     * Detect 页状态分：
     * 演示时不用再切回 Home 页面看总分。
     */
    detect_risk_score_text_label = label_create(state, "Risk Score", 28, 108, COLOR_SUB_TEXT, FONT_SMALL);
    detect_risk_score_label = label_create(state, "0", 145, 106, COLOR_GREEN, FONT_NORMAL);

    /* 新增：车辆运动状态 */
    label_create(state, "Vehicle", 28, 134, COLOR_SUB_TEXT, FONT_SMALL);
    det_vehicle_state = label_create(state, "PARKED", 105, 134, COLOR_YELLOW, FONT_SMALL);

    det_explain_text = label_create(state, "No abnormal behavior.", 28, 154, COLOR_SUB_TEXT, FONT_SMALL);

    /* Overlay 开关 */
    label_create(state, "Overlay", 28, 176, COLOR_SUB_TEXT, FONT_SMALL);

    overlay_state_label = label_create(state, "On", 92, 176, COLOR_GREEN, FONT_SMALL);

    overlay_switch = lv_switch_create(state);
    lv_obj_set_size(overlay_switch, 46, 24);
    lv_obj_set_pos(overlay_switch, 145, 170);
    lv_obj_add_state(overlay_switch, LV_STATE_CHECKED);
    lv_obj_add_event_cb(overlay_switch, overlay_switch_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    /* 右下：行为信号 */
    lv_obj_t *behavior = glass_panel_create(page, 630, 220, 210, 220);
    title_create(behavior, "BEHAVIOR SIGNAL", 20, 14);

    label_create(behavior, "Eyes", 25, 52, COLOR_SUB_TEXT, FONT_SMALL);
    det_eye_state = label_create(behavior, "Normal", 105, 52, COLOR_GREEN, FONT_SMALL);

    label_create(behavior, "EyeRisk", 25, 79, COLOR_SUB_TEXT, FONT_SMALL);
    det_perclos = label_create(behavior, "0%", 105, 79, COLOR_CYAN, FONT_SMALL);

    label_create(behavior, "Yaw", 25, 106, COLOR_SUB_TEXT, FONT_SMALL);
    det_blink = label_create(behavior, "Normal", 105, 106, COLOR_CYAN, FONT_SMALL);

    label_create(behavior, "Wheel", 25, 133, COLOR_SUB_TEXT, FONT_SMALL);
    det_yawn = label_create(behavior, "On", 105, 133, COLOR_GREEN, FONT_SMALL);

    /*
     * 底部三个状态胶囊。
     * Head / Phone / Mouth 三项保持现在的布局。
     */
    det_head_pill  = pill_create(behavior, 25, 163, 85, "Normal");
    det_phone_pill = pill_create(behavior, 120, 163, 85, "Normal");
    det_smoke_pill = pill_create(behavior, 72, 193, 85, "Normal");
}

/* =========================================================
 * Environment page
 * ========================================================= */
static void fan_slider_event_cb(lv_event_t *e)
{
    if (g_updating_slider) return;
    if (!g_fan_ctrl || g_fan_ctrl->mode != 2) return;

    lv_obj_t *slider = lv_event_get_target(e);
    int value = lv_slider_get_value(slider);

    g_fan_ctrl->mode = 2;
    g_fan_ctrl->manual_speed = value;

    ui_update_fan_speed(value);
}

static void fan_mode_btn_event_cb(lv_event_t *e)
{
    const char *mode = (const char*)lv_event_get_user_data(e);

    if (!g_fan_ctrl || !mode) return;

    if (strcmp(mode, "auto") == 0) {
        g_fan_ctrl->mode = 0;
        ui_update_fan_mode("Auto");

        if (slider_fan) {
            lv_obj_clear_flag(slider_fan, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_add_state(slider_fan, LV_STATE_DISABLED);
        }
    } else if (strcmp(mode, "manual") == 0) {
        g_fan_ctrl->mode = 2;
        ui_update_fan_mode("Manual");

        if (slider_fan) {
            lv_obj_add_flag(slider_fan, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_clear_state(slider_fan, LV_STATE_DISABLED);

            g_updating_slider = true;
            lv_slider_set_value(slider_fan, g_fan_ctrl->manual_speed, LV_ANIM_OFF);
            g_updating_slider = false;
        }

        ui_update_fan_speed(g_fan_ctrl->manual_speed);
    } else if (strcmp(mode, "off") == 0) {
        g_fan_ctrl->mode = 3;
        ui_update_fan_mode("Off");

        if (slider_fan) {
            lv_obj_clear_flag(slider_fan, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_add_state(slider_fan, LV_STATE_DISABLED);
        }
    }
}

static lv_obj_t *mode_btn_create(lv_obj_t *parent,
                                 const char *txt,
                                 int x,
                                 int y,
                                 const char *mode)
{
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_size(btn, 110, 38);
    lv_obj_set_pos(btn, x, y);
    lv_obj_set_style_radius(btn, 12, 0);
    lv_obj_set_style_bg_color(btn, COLOR_PANEL_2, 0);
    lv_obj_set_style_border_width(btn, 1, 0);
    lv_obj_set_style_border_color(btn, COLOR_BORDER_DIM, 0);

    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, txt);
    lv_obj_center(label);

    lv_obj_add_event_cb(btn, fan_mode_btn_event_cb, LV_EVENT_CLICKED, (void*)mode);

    return btn;
}

static void create_environment_page(lv_obj_t *scr)
{
    lv_obj_t *page = lv_obj_create(scr);
    pages[PAGE_ENVIRONMENT] = page;

    lv_obj_set_size(page, PAGE_W, PAGE_H);
    lv_obj_set_pos(page, PAGE_X, PAGE_Y);
    lv_obj_set_style_bg_opa(page, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(page, 0, 0);
    lv_obj_set_style_pad_all(page, 0, 0);
    lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);

    decorative_curve_lines(page);

    lv_obj_t *fan = glass_panel_create(page, 0, 0, 455, 440);
    title_create(fan, "3. ENVIRONMENT / FAN CONTROL", 24, 18);

    env_fan_arc = lv_arc_create(fan);
    lv_obj_set_size(env_fan_arc, 230, 230);
    lv_obj_set_pos(env_fan_arc, 70, 70);
    lv_arc_set_range(env_fan_arc, 0, 100);
    lv_arc_set_value(env_fan_arc, 40);
    lv_obj_remove_style(env_fan_arc, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(env_fan_arc, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_arc_width(env_fan_arc, 16, LV_PART_MAIN);
    lv_obj_set_style_arc_width(env_fan_arc, 16, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(env_fan_arc, lv_color_hex(0x0D2D44), LV_PART_MAIN);
    lv_obj_set_style_arc_color(env_fan_arc, COLOR_CYAN, LV_PART_INDICATOR);

    label_create(fan, "FAN", 155, 160, COLOR_CYAN, FONT_LARGE);
    env_fan_speed = label_create(fan, "0%", 155, 220, COLOR_CYAN, FONT_NUMBER);

    mode_btn_create(fan, "Auto", 45, 330, "auto");
    mode_btn_create(fan, "Manual", 170, 330, "manual");
    mode_btn_create(fan, "Off", 295, 330, "off");

    slider_fan = lv_slider_create(fan);
    lv_obj_set_size(slider_fan, 350, 14);
    lv_obj_set_pos(slider_fan, 52, 390);
    lv_slider_set_range(slider_fan, 0, 100);
    lv_slider_set_value(slider_fan, 40, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(slider_fan, lv_color_hex(0x20364A), LV_PART_MAIN);
    lv_obj_set_style_bg_color(slider_fan, COLOR_CYAN, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(slider_fan, COLOR_TEXT, LV_PART_KNOB);
    lv_obj_add_event_cb(slider_fan, fan_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_clear_flag(slider_fan, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_state(slider_fan, LV_STATE_DISABLED);

    lv_obj_t *info = glass_panel_create(page, 490, 0, 350, 200);
    title_create(info, "CABIN ENVIRONMENT", 24, 14);

    label_create(info, "Temperature", 35, 70, COLOR_SUB_TEXT, FONT_NORMAL);
    env_temp_label = label_create(info, "--C", 35, 105, COLOR_CYAN, FONT_LARGE);

    label_create(info, "Humidity", 205, 70, COLOR_SUB_TEXT, FONT_NORMAL);
    env_humi_label = label_create(info, "--%", 205, 105, COLOR_CYAN, FONT_LARGE);

    lv_obj_t *thermal = glass_panel_create(page, 490, 225, 350, 215);
    title_create(thermal, "THERMAL STATUS", 24, 14);

    label_create(thermal, "Mode", 35, 70, COLOR_SUB_TEXT, FONT_NORMAL);
    env_fan_mode = label_create(thermal, "Auto", 140, 70, COLOR_GREEN, FONT_NORMAL);

    label_create(thermal, "Comfort", 35, 110, COLOR_SUB_TEXT, FONT_NORMAL);
    env_comfort_label = label_create(thermal, "Good", 140, 110, COLOR_GREEN, FONT_NORMAL);

    label_create(thermal, "Chip Temp", 35, 150, COLOR_SUB_TEXT, FONT_NORMAL);
    env_chip_temp = label_create(thermal, "--C", 140, 150, COLOR_YELLOW, FONT_NORMAL);
}

/* =========================================================
 * Records page
 * ========================================================= */
static const char *ui_get_time_text(void)
{
    if (label_time) {
        return lv_label_get_text(label_time);
    }

    return "--:--:--";
}

static void records_refresh_ui(void)
{
    for (int i = 0; i < MAX_EVENT_RECORDS; i++) {
        if (rec_event_labels[i]) {
            lv_label_set_text(rec_event_labels[i], rec_event_texts[i]);
        }
    }
}

static void records_add_event(const char *event_text, lv_color_t color)
{
    if (!event_text) {
        return;
    }

    /*
     * 最新事件放在第 0 行，旧事件往下移动。
     */
    for (int i = MAX_EVENT_RECORDS - 1; i > 0; i--) {
        strncpy(rec_event_texts[i], rec_event_texts[i - 1], EVENT_TEXT_LEN - 1);
        rec_event_texts[i][EVENT_TEXT_LEN - 1] = '\0';
    }

    snprintf(rec_event_texts[0],
             EVENT_TEXT_LEN,
             "%s   %s",
             ui_get_time_text(),
             event_text);

    if (rec_event_count < MAX_EVENT_RECORDS) {
        rec_event_count++;
    }

    records_refresh_ui();

    /*
     * 最新一条根据事件类型变色，其他保持普通颜色。
     */
    for (int i = 0; i < MAX_EVENT_RECORDS; i++) {
        if (!rec_event_labels[i]) continue;

        if (i == 0) {
            lv_obj_set_style_text_color(rec_event_labels[i], color, 0);
        } else {
            lv_obj_set_style_text_color(rec_event_labels[i], COLOR_SUB_TEXT, 0);
        }
    }
}

static void records_update_time_axis(void)
{
    if (!rec_time_labels[0]) {
        return;
    }

    time_t now = time(NULL);

    /*
     * 这 5 个时间点对应：
     * 60秒前、45秒前、30秒前、15秒前、当前
     */
    int offsets[REC_TIME_LABELS] = {
        -60, -45, -30, -15, 0
    };

    char buf[REC_TIME_LABELS][16];

    for (int i = 0; i < REC_TIME_LABELS; i++) {
        time_t t = now + offsets[i];
        struct tm *tm_info = localtime(&t);

        if (tm_info) {
            snprintf(buf[i],
                     sizeof(buf[i]),
                     "%02d:%02d:%02d",
                     tm_info->tm_hour,
                     tm_info->tm_min,
                     tm_info->tm_sec);
        } else {
            snprintf(buf[i], sizeof(buf[i]), "--:--:--");
        }

        lv_label_set_text(rec_time_labels[i], buf[i]);
    }
}

static const char *history_event_pretty_name(const char *event)
{
    if (!event) return "UNKNOWN";

    if (strcmp(event, "HEAD_DOWN") == 0) {
        return "Head Down";
    } else if (strcmp(event, "PHONE_CALL") == 0) {
        return "Phone Call";
    } else if (strcmp(event, "MOUTH_RISK") == 0) {
        return "Mouth Risk";
    } else if (strcmp(event, "WHEEL_OFF") == 0) {
        return "Wheel Off";
    } else if (strcmp(event, "DANGEROUS") == 0) {
        return "Dangerous";
    }

    return event;
}

static void format_history_csv_line(const char *csv_line, char *out, size_t out_size)
{
    if (!csv_line || !out || out_size == 0) {
        return;
    }

    char time_str[32] = {0};
    char event[32] = {0};
    char vehicle_mode[32] = {0};

    int score = 0;
    int head_down = 0;
    int phone_call = 0;
    int mouth_risk = 0;
    int hands_off = 0;
    int vehicle_moving = 0;
    int voice_broadcast = 0;

    int ret = sscanf(csv_line,
                     "%31[^,],%31[^,],%d,%d,%d,%d,%d,%31[^,],%d,%d",
                     time_str,
                     event,
                     &score,
                     &head_down,
                     &phone_call,
                     &mouth_risk,
                     &hands_off,
                     vehicle_mode,
                     &vehicle_moving,
                     &voice_broadcast);

    if (ret < 10) {
        snprintf(out, out_size, "%s", csv_line);
        out[out_size - 1] = '\0';

        size_t len = strlen(out);
        if (len > 0 && (out[len - 1] == '\n' || out[len - 1] == '\r')) {
            out[len - 1] = '\0';
        }
        return;
    }

    /*
     * time_str 格式一般是：
     * 2026-06-03 23:04:46
     * UI 里只显示 HH:MM:SS，更清爽。
     */
    const char *clock_str = time_str;
    if (strlen(time_str) >= 19) {
        clock_str = time_str + 11;
    }

    snprintf(out,
             out_size,
             "%s  %-10s  S:%3d  %-7s  %s  Voice:%s",
             clock_str,
             history_event_pretty_name(event),
             score,
             vehicle_mode,
             vehicle_moving ? "MOVING" : "PARKED",
             voice_broadcast ? "ON" : "OFF");
}

static int load_recent_history_lines(char lines[HISTORY_MAX_LINES][HISTORY_LINE_LEN])
{
    FILE *fp = fopen(EVENT_LOG_FILE, "r");
    if (!fp) {
        return 0;
    }

    char ring[HISTORY_MAX_LINES][HISTORY_LINE_LEN];
    int total = 0;

    char buf[HISTORY_LINE_LEN];

    while (fgets(buf, sizeof(buf), fp)) {
        /*
         * 跳过表头。
         */
        if (strncmp(buf, "time,event", 10) == 0) {
            continue;
        }

        /*
         * 跳过空行。
         */
        if (strlen(buf) < 5) {
            continue;
        }

        int idx = total % HISTORY_MAX_LINES;
        snprintf(ring[idx], HISTORY_LINE_LEN, "%s", buf);
        total++;
    }

    fclose(fp);

    int count = total < HISTORY_MAX_LINES ? total : HISTORY_MAX_LINES;

    /*
     * 最新的放在最上面。
     */
    for (int i = 0; i < count; i++) {
        int src_index = (total - 1 - i) % HISTORY_MAX_LINES;
        if (src_index < 0) {
            src_index += HISTORY_MAX_LINES;
        }

        format_history_csv_line(ring[src_index], lines[i], HISTORY_LINE_LEN);
    }

    return count;
}

static void close_history_popup_event_cb(lv_event_t *e)
{
    (void)e;

    if (history_popup) {
        lv_obj_del(history_popup);
        history_popup = NULL;
    }
}

static void show_history_popup_event_cb(lv_event_t *e)
{
    (void)e;

    if (history_popup) {
        lv_obj_del(history_popup);
        history_popup = NULL;
    }

    /*
     * 半透明遮罩层。
     */
    history_popup = lv_obj_create(lv_layer_top());
    lv_obj_set_size(history_popup, 900, 560);
    lv_obj_set_pos(history_popup, 0, 0);
    lv_obj_set_style_bg_color(history_popup, lv_color_hex(0x020A12), 0);
    lv_obj_set_style_bg_opa(history_popup, LV_OPA_80, 0);
    lv_obj_set_style_border_width(history_popup, 0, 0);
    lv_obj_clear_flag(history_popup, LV_OBJ_FLAG_SCROLLABLE);

    /*
     * 中间历史面板。
     */
    lv_obj_t *panel = lv_obj_create(history_popup);
    lv_obj_set_size(panel, 760, 470);
    lv_obj_set_pos(panel, 70, 45);
    lv_obj_set_style_bg_color(panel, lv_color_hex(0x071A2F), 0);
    lv_obj_set_style_bg_opa(panel, LV_OPA_90, 0);
    lv_obj_set_style_border_width(panel, 1, 0);
    lv_obj_set_style_border_color(panel, COLOR_CYAN, 0);
    lv_obj_set_style_border_opa(panel, LV_OPA_60, 0);
    lv_obj_set_style_radius(panel, 18, 0);
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);

    label_create(panel, "EVENT HISTORY / CSV RECENT RECORDS", 26, 18, COLOR_CYAN, FONT_SMALL);

    /*
     * Close 按钮。
     */
    lv_obj_t *btn_close = lv_btn_create(panel);
    lv_obj_set_size(btn_close, 88, 34);
    lv_obj_set_pos(btn_close, 640, 14);
    lv_obj_set_style_radius(btn_close, 16, 0);
    lv_obj_set_style_bg_color(btn_close, lv_color_hex(0x102B45), 0);
    lv_obj_set_style_bg_opa(btn_close, LV_OPA_80, 0);
    lv_obj_set_style_border_width(btn_close, 1, 0);
    lv_obj_set_style_border_color(btn_close, COLOR_CYAN, 0);
    lv_obj_add_event_cb(btn_close, close_history_popup_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *close_label = lv_label_create(btn_close);
    lv_label_set_text(close_label, "Close");
    lv_obj_set_style_text_color(close_label, COLOR_CYAN, 0);
    lv_obj_set_style_text_font(close_label, FONT_SMALL, 0);
    lv_obj_center(close_label);

    char lines[HISTORY_MAX_LINES][HISTORY_LINE_LEN];
    int count = load_recent_history_lines(lines);

    if (count <= 0) {
        label_create(panel,
                     "No CSV history found. Trigger an alert first.",
                     35,
                     78,
                     COLOR_SUB_TEXT,
                     FONT_SMALL);
        return;
    }

    /*
     * 显示最近 20 条。最新在最上面。
     */
    for (int i = 0; i < count; i++) {
        history_line_labels[i] = label_create(panel,
                                              lines[i],
                                              35,
                                              70 + i * 19,
                                              (i == 0) ? COLOR_GREEN : COLOR_SUB_TEXT,
                                              FONT_SMALL);
    }
}

/* =========================================================
 * AI Copilot helpers
 * ========================================================= */
static int read_text_file_ui(const char *path, char *buf, int size)
{
    FILE *fp = fopen(path, "r");
    if (!fp) {
        return -1;
    }

    int n = fread(buf, 1, size - 1, fp);
    fclose(fp);

    if (n < 0) {
        return -1;
    }

    buf[n] = '\0';
    return 0;
}

static void danger_flash_timer_cb(lv_timer_t *timer)
{
    (void)timer;

    char alert[64] = {0};
    char fan_action[64] = {0};

    if (read_text_file_ui("/home/elf/ai_cloud/runtime/ui_alert",
                          alert,
                          sizeof(alert)) == 0) {
        if (strstr(alert, "RED_FLASH") != NULL && danger_flash_count == 0) {
            danger_flash_count = 8;

            FILE *fp = fopen("/home/elf/ai_cloud/runtime/ui_alert", "w");
            if (fp) {
                fprintf(fp, "NONE");
                fclose(fp);
            }
        }
    }

    if (danger_flash_count > 0) {
        if (!danger_flash_layer) {
            danger_flash_layer = lv_obj_create(lv_layer_top());
            lv_obj_set_size(danger_flash_layer, SCREEN_W, SCREEN_H);
            lv_obj_set_pos(danger_flash_layer, 0, 0);
            lv_obj_set_style_bg_color(danger_flash_layer, COLOR_RED, 0);
            lv_obj_set_style_bg_opa(danger_flash_layer, LV_OPA_40, 0);
            lv_obj_set_style_border_width(danger_flash_layer, 0, 0);
            lv_obj_clear_flag(danger_flash_layer, LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_clear_flag(danger_flash_layer, LV_OBJ_FLAG_CLICKABLE);

            lv_obj_t *label = lv_label_create(danger_flash_layer);
            lv_label_set_text(label, "HIGH RISK WARNING");
            lv_obj_set_style_text_color(label, COLOR_TEXT, 0);
            lv_obj_set_style_text_font(label, FONT_LARGE, 0);
            lv_obj_center(label);
        }

        if (danger_flash_count % 2 == 0) {
            lv_obj_set_style_bg_opa(danger_flash_layer, LV_OPA_60, 0);
        } else {
            lv_obj_set_style_bg_opa(danger_flash_layer, LV_OPA_20, 0);
        }

        danger_flash_count--;

        if (danger_flash_count == 0 && danger_flash_layer) {
            lv_obj_del(danger_flash_layer);
            danger_flash_layer = NULL;
        }
    }

    if (read_text_file_ui("/home/elf/ai_cloud/runtime/fan_action",
                          fan_action,
                          sizeof(fan_action)) == 0) {
    if (strstr(fan_action, "FAN_BOOST") != NULL) {
        fan_boost_active = 1;
        normal_behavior_count = 0;

        if (g_fan_ctrl) {
            g_fan_ctrl->mode = 2;
            g_fan_ctrl->manual_speed = 100;
            ui_update_fan_mode("AI Boost");
            ui_update_fan_speed(100);
        }

        /*
        * fan_action 是一次性命令，UI 消费后可以清成 NONE。
        * fan_state 是真实风扇状态，必须保持 BOOST，给 Dashboard 同步。
        */
        FILE *fp_state = fopen("/home/elf/ai_cloud/runtime/fan_state", "w");
        if (fp_state) {
            fprintf(fp_state, "BOOST");
            fclose(fp_state);
        }

        FILE *fp = fopen("/home/elf/ai_cloud/runtime/fan_action", "w");
        if (fp) {
            fprintf(fp, "NONE");
            fclose(fp);
        }
    }
    }

    /*
 * 如果AI风扇强干预已经开启，并且风险恢复正常一段时间，
 * 自动把风扇恢复到正常模式。
 */
if (fan_boost_active) {
    char level_buf[32] = {0};
    char score_buf[32] = {0};

    int level = 0;
    int score = 0;

    if (read_text_file_ui("/home/elf/ai_cloud/runtime/risk_level",
                          level_buf,
                          sizeof(level_buf)) == 0) {
        level = atoi(level_buf);
    }

    if (read_text_file_ui("/home/elf/ai_cloud/runtime/cognitive_risk_score",
                          score_buf,
                          sizeof(score_buf)) == 0) {
        score = atoi(score_buf);
    }

    /*
     * 这里的timer周期如果是180ms，
     * normal_behavior_count > 40 大概是7秒。
     * 你也可以改成60，大概10秒。
     */
    if (level == 0 && score < 4) {
        normal_behavior_count++;
    } else {
        normal_behavior_count = 0;
    }

    if (normal_behavior_count > 40) {
        fan_boost_active = 0;
        normal_behavior_count = 0;

        if (g_fan_ctrl) {
            g_fan_ctrl->mode = 0;
            g_fan_ctrl->manual_speed = 40;
            ui_update_fan_mode("Auto");
            ui_update_fan_speed(40);
        }

        FILE *fp_action = fopen("/home/elf/ai_cloud/runtime/fan_action", "w");
        if (fp_action) {
            fprintf(fp_action, "NORMAL");
            fclose(fp_action);
        }

        FILE *fp_state = fopen("/home/elf/ai_cloud/runtime/fan_state", "w");
        if (fp_state) {
            fprintf(fp_state, "NORMAL");
            fclose(fp_state);
        }

        FILE *fp = fopen("/home/elf/ai_cloud/runtime/ai_action_status", "w");
        if (fp) {
            fprintf(fp,
                    "Driver behavior recovered | Fan:NORMAL | Feedback:RECOVERED");
            fclose(fp);
        }
    }
}

}

static void ai_auto_popup_timer_cb(lv_timer_t *timer)
{
    (void)timer;

    char level_buf[32] = {0};

    if (read_text_file_ui("/home/elf/ai_cloud/runtime/risk_level",
                          level_buf,
                          sizeof(level_buf)) != 0) {
        return;
    }

    int level = atoi(level_buf);

    /*
     * Level 1 / Level 2 不自动弹窗。
     * 只有 Level 3 才自动弹出 AI 认知干预窗口。
     */
    if (level < 3) {
        return;
    }

    uint32_t now = lv_tick_get();

    /*
     * 关键修正：
     * last_auto_popup_tick == 0 表示从未自动弹过，
     * 第一次 Level 3 必须允许弹出。
     *
     * 已经弹过后，120秒内不再自动弹。
     */
    if (last_auto_popup_tick != 0 &&
        (now - last_auto_popup_tick) < 120000) {
        return;
    }

    last_auto_popup_level = level;
    last_auto_popup_tick = now;

    /*
     * 标记：这是系统自动弹出的 Level 3 AI 干预窗口。
     * 只有这种弹窗关闭后，才允许恢复风扇。
     */
    ai_popup_auto_opened = 1;

    if (!ai_popup) {
        show_ai_copilot_popup_event_cb(NULL);
    } else {
        ai_result_timer_cb(NULL);
    }
}

static void close_ai_popup_event_cb(lv_event_t *e)
{
    (void)e;

    /*
     * 关闭弹窗后进入2分钟冷却。
     * 不管手动还是自动，都避免马上又弹。
     */
    last_auto_popup_tick = lv_tick_get();

    /*
     * 关键：
     * 只有系统自动 Level 3 弹出的 AI 干预窗口，
     * 关闭时才认为驾驶员确认风险，并恢复风扇。
     *
     * 用户手动打开 AI Copilot 查看分析，关闭时不改变风扇状态。
     */
    if (ai_popup_auto_opened) {
        fan_boost_active = 0;
        normal_behavior_count = 0;

        if (g_fan_ctrl) {
            g_fan_ctrl->mode = 0;
            g_fan_ctrl->manual_speed = 40;
            ui_update_fan_mode("Auto");
            ui_update_fan_speed(40);
        }

        FILE *fp = fopen("/home/elf/ai_cloud/runtime/fan_action", "w");
        if (fp) {
            fprintf(fp, "NORMAL");
            fclose(fp);
        }

        /*
         * 关键补充：
         * 关闭自动 Level 3 弹窗后，真实风扇已经恢复为普通 Auto/Normal，
         * 必须同步写 fan_state=NORMAL，否则 Dashboard 会认为风扇还在 BOOST。
         */
        FILE *fp_state = fopen("/home/elf/ai_cloud/runtime/fan_state", "w");
        if (fp_state) {
            fprintf(fp_state, "NORMAL");
            fclose(fp_state);
        }

        FILE *fp2 = fopen("/home/elf/ai_cloud/runtime/ai_action_status", "w");
        if (fp2) {
            fprintf(fp2,
                    "Auto AI intervention acknowledged | Fan:NORMAL | Feedback:RECOVERED");
            fclose(fp2);
        }
    }

    ai_popup_auto_opened = 0;

    if (ai_popup) {
        lv_obj_del(ai_popup);
        ai_popup = NULL;
        ai_answer_box = NULL;
        ai_answer_label = NULL;
        ai_status_label = NULL;
    }
}

static void ai_result_timer_cb(lv_timer_t *timer)
{
    (void)timer;

    if (!ai_popup || !ai_answer_label) {
        return;
    }

    char answer[2048] = {0};
    char status[64] = {0};
    char latency[64] = {0};
    char policy_status[64] = {0};

    char action_status[256] = {0};
    char risk_level[32] = {0};
    char cognitive_score[32] = {0};

    char display[4300] = {0};

    if (read_text_file_ui("/home/elf/ai_cloud/runtime/llm_answer",
                          answer,
                          sizeof(answer)) != 0) {
        return;
    }

    read_text_file_ui("/home/elf/ai_cloud/runtime/cloud_status",
                      status,
                      sizeof(status));

    read_text_file_ui("/home/elf/ai_cloud/runtime/cloud_latency",
                      latency,
                      sizeof(latency));

    read_text_file_ui("/home/elf/ai_cloud/runtime/ai_policy_status",
                      policy_status,
                      sizeof(policy_status));

    read_text_file_ui("/home/elf/ai_cloud/runtime/ai_action_status",
                  action_status,
                  sizeof(action_status));

    read_text_file_ui("/home/elf/ai_cloud/runtime/risk_level",
                    risk_level,
                    sizeof(risk_level));

    read_text_file_ui("/home/elf/ai_cloud/runtime/cognitive_risk_score",
                    cognitive_score,
                    sizeof(cognitive_score));

    if (status[0] == '\0') {
        snprintf(status, sizeof(status), "--");
    }

    if (latency[0] == '\0') {
        snprintf(latency, sizeof(latency), "--");
    }

    if (policy_status[0] == '\0') {
        snprintf(policy_status, sizeof(policy_status), "DEFAULT_POLICY");
    }

    if (ai_status_label) {
        char header[256];

        snprintf(header,
                 sizeof(header),
                 "Cloud AI: %s    Latency: %s    Policy: %s",
                 status,
                 latency,
                 policy_status);

        lv_label_set_text(ai_status_label, header);

        if (strcmp(policy_status, "AI_POLICY_ACTIVE") == 0) {
            lv_obj_set_style_text_color(ai_status_label, COLOR_GREEN, 0);
        } else if (strcmp(status, "OFFLINE") == 0) {
            lv_obj_set_style_text_color(ai_status_label, COLOR_YELLOW, 0);
        } else if (strcmp(status, "BUSY") == 0) {
            lv_obj_set_style_text_color(ai_status_label, COLOR_CYAN, 0);
        } else if (strcmp(status, "ONLINE") == 0) {
            lv_obj_set_style_text_color(ai_status_label, COLOR_GREEN, 0);
        } else {
            lv_obj_set_style_text_color(ai_status_label, COLOR_SUB_TEXT, 0);
        }
    }

    // snprintf(display, sizeof(display), "%s", answer);

    // /*
    // * 只有内容变化时才刷新 label。
    // * 否则用户正在滑动时，定时器不要打断滚动位置。
    // */
    // if (strcmp(display, ai_last_answer_cache) != 0) {
    //     snprintf(ai_last_answer_cache, sizeof(ai_last_answer_cache), "%s", display);

    //     lv_label_set_text(ai_answer_label, display);

    //     if (ai_answer_box) {
    //         lv_obj_scroll_to_y(ai_answer_box, 0, LV_ANIM_OFF);
    //     }
    // }
    snprintf(display,
            sizeof(display),
            "Cognitive Risk Level: %s\n"
            "Cognitive Score: %s\n"
            "%s\n\n"
            "%s",
            risk_level[0] ? risk_level : "--",
            cognitive_score[0] ? cognitive_score : "--",
            action_status[0] ? action_status : "Action: --",
            answer);

    lv_label_set_text(ai_answer_label, display);
    lv_obj_set_style_text_color(ai_answer_label, lv_color_hex(0xD8F3FF), 0);
    lv_obj_set_style_text_font(ai_answer_label, FONT_SMALL, 0);
}

static void ai_start_request(const char *mode)
{
    if (!mode) {
        mode = "summary";
    }

    system("date '+AI button clicked at %H:%M:%S' >> /tmp/ai_button_debug.log");

    system("rm -f /home/elf/ai_cloud/llm_busy");

    if (strcmp(mode, "why") == 0) {
        system("echo 'Loading AI POLICY GENERATOR...' > /home/elf/ai_cloud/runtime/llm_answer");
        system("echo 'WHY clicked' >> /tmp/ai_button_debug.log");
        system("echo 'BUSY' > /home/elf/ai_cloud/runtime/cloud_status");
        system("echo '--' > /home/elf/ai_cloud/runtime/cloud_latency");
        system("nohup /usr/bin/python3 /home/elf/ai_cloud/ask_cloud.py why > /home/elf/ai_cloud/runtime/ask_cloud_ui.log 2>&1 &");
    }
    else if (strcmp(mode, "advice") == 0) {
        system("echo 'Loading SCENARIO PLAN...' > /home/elf/ai_cloud/runtime/llm_answer");
        system("echo 'ADVICE clicked' >> /tmp/ai_button_debug.log");
        system("echo 'BUSY' > /home/elf/ai_cloud/runtime/cloud_status");
        system("echo '--' > /home/elf/ai_cloud/runtime/cloud_latency");
        system("nohup /usr/bin/python3 /home/elf/ai_cloud/ask_cloud.py advice > /home/elf/ai_cloud/runtime/ask_cloud_ui.log 2>&1 &");
    }
    else {
        system("echo 'Loading AI RISK BRAIN...' > /home/elf/ai_cloud/runtime/llm_answer");
        system("echo 'SUMMARY clicked' >> /tmp/ai_button_debug.log");
        system("echo 'BUSY' > /home/elf/ai_cloud/runtime/cloud_status");
        system("echo '--' > /home/elf/ai_cloud/runtime/cloud_latency");
        system("nohup /usr/bin/python3 /home/elf/ai_cloud/ask_cloud.py summary > /home/elf/ai_cloud/runtime/ask_cloud_ui.log 2>&1 &");
    }
}

static void ai_mode_btn_event_cb(lv_event_t *e)
{
    const char *mode = (const char *)lv_event_get_user_data(e);
    ai_start_request(mode);
}

static lv_obj_t *ai_small_btn_create(lv_obj_t *parent,
                                     const char *txt,
                                     int x,
                                     int y,
                                     const char *mode)
{
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_size(btn, 100, 34);
    lv_obj_set_pos(btn, x, y);

    lv_obj_set_style_radius(btn, 16, 0);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x102B45), 0);
    lv_obj_set_style_bg_opa(btn, LV_OPA_80, 0);
    lv_obj_set_style_border_width(btn, 1, 0);
    lv_obj_set_style_border_color(btn, COLOR_CYAN, 0);

    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, txt);
    lv_obj_set_style_text_color(label, COLOR_CYAN, 0);
    lv_obj_set_style_text_font(label, FONT_SMALL, 0);
    lv_obj_center(label);

    lv_obj_add_event_cb(btn, ai_mode_btn_event_cb, LV_EVENT_CLICKED, (void *)mode);

    return btn;
}

static void show_ai_copilot_popup_event_cb(lv_event_t *e)
{
    (void)e;

    /*
     * e != NULL 表示用户点击按钮手动打开。
     * 自动弹窗会传 NULL，但自动弹窗函数里会提前设置 ai_popup_auto_opened = 1。
     */
    if (e != NULL) {
        ai_popup_auto_opened = 0;
    }

    if (ai_popup) {
        lv_obj_del(ai_popup);
        ai_popup = NULL;
        ai_answer_box = NULL;
        ai_answer_label = NULL;
        ai_status_label = NULL;
    }
    ai_last_answer_cache[0] = '\0';

    /*
     * 半透明遮罩层
     */
    ai_popup = lv_obj_create(lv_layer_top());
    lv_obj_set_size(ai_popup, 900, 560);
    lv_obj_set_pos(ai_popup, 0, 0);
    lv_obj_set_style_bg_color(ai_popup, lv_color_hex(0x020A12), 0);
    lv_obj_set_style_bg_opa(ai_popup, LV_OPA_80, 0);
    lv_obj_set_style_border_width(ai_popup, 0, 0);
    lv_obj_clear_flag(ai_popup, LV_OBJ_FLAG_SCROLLABLE);

    /*
     * 主面板
     */
    lv_obj_t *panel = lv_obj_create(ai_popup);
    lv_obj_set_size(panel, 790, 490);
    lv_obj_set_pos(panel, 55, 35);
    lv_obj_set_style_bg_color(panel, lv_color_hex(0x071A2F), 0);
    lv_obj_set_style_bg_opa(panel, LV_OPA_90, 0);
    lv_obj_set_style_border_width(panel, 1, 0);
    lv_obj_set_style_border_color(panel, COLOR_CYAN, 0);
    lv_obj_set_style_border_opa(panel, LV_OPA_70, 0);
    lv_obj_set_style_radius(panel, 18, 0);
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);

    label_create(panel,
                 "AI COPILOT / EDGE-CLOUD RISK ANALYSIS",
                 26,
                 18,
                 COLOR_CYAN,
                 FONT_SMALL);

    ai_status_label = label_create(panel,
                                   "Cloud AI: --    Latency: --",
                                   26,
                                   48,
                                   COLOR_SUB_TEXT,
                                   FONT_SMALL);

    // ai_policy_label = label_create(panel,
    //                            "DEFAULT_POLICY",
    //                            520,
    //                            48,
    //                            COLOR_SUB_TEXT,
    //                            FONT_SMALL);                               

    /*
     * 结果显示区域
     */
    ai_answer_box = lv_obj_create(panel);
    lv_obj_set_size(ai_answer_box, 730, 315);
    lv_obj_set_pos(ai_answer_box, 28, 84);
    lv_obj_set_style_bg_color(ai_answer_box, lv_color_hex(0x020617), 0);
    lv_obj_set_style_bg_opa(ai_answer_box, LV_OPA_80, 0);
    lv_obj_set_style_border_width(ai_answer_box, 1, 0);
    lv_obj_set_style_border_color(ai_answer_box, COLOR_BORDER_DIM, 0);
    lv_obj_set_style_radius(ai_answer_box, 12, 0);
    lv_obj_set_style_pad_all(ai_answer_box, 10, 0);

    /* 关键：开启垂直滚动 */
    lv_obj_add_flag(ai_answer_box, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(ai_answer_box, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(ai_answer_box, LV_SCROLLBAR_MODE_AUTO);

    /* 可选：滚动条样式更明显一点 */
    lv_obj_set_style_bg_opa(ai_answer_box, LV_OPA_80, LV_PART_MAIN);
    lv_obj_set_style_width(ai_answer_box, 6, LV_PART_SCROLLBAR);
    lv_obj_set_style_bg_color(ai_answer_box, COLOR_CYAN, LV_PART_SCROLLBAR);
    lv_obj_set_style_bg_opa(ai_answer_box, LV_OPA_60, LV_PART_SCROLLBAR);

    ai_answer_label = lv_label_create(ai_answer_box);
    lv_label_set_text(ai_answer_label,
                    "Press Brain / Policy / Plan to start AI Copilot analysis.");
    lv_obj_set_pos(ai_answer_label, 0, 0);
    lv_obj_set_width(ai_answer_label, 690);
    lv_label_set_long_mode(ai_answer_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_color(ai_answer_label, COLOR_TEXT, 0);
    lv_obj_set_style_text_font(ai_answer_label, &lv_font_chinese_18, 0);

    /*
     * 底部按钮
     */
    ai_small_btn_create(panel, "Brain",  28, 425, "summary");
    ai_small_btn_create(panel, "Policy", 145, 425, "why");
    ai_small_btn_create(panel, "Plan",   262, 425, "advice");

    lv_obj_t *btn_close = lv_btn_create(panel);
    lv_obj_set_size(btn_close, 100, 34);
    lv_obj_set_pos(btn_close, 658, 425);
    lv_obj_set_style_radius(btn_close, 16, 0);
    lv_obj_set_style_bg_color(btn_close, lv_color_hex(0x102B45), 0);
    lv_obj_set_style_bg_opa(btn_close, LV_OPA_80, 0);
    lv_obj_set_style_border_width(btn_close, 1, 0);
    lv_obj_set_style_border_color(btn_close, COLOR_RED, 0);
    lv_obj_add_event_cb(btn_close, close_ai_popup_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *close_label = lv_label_create(btn_close);
    lv_label_set_text(close_label, "Close");
    lv_obj_set_style_text_color(close_label, COLOR_RED, 0);
    lv_obj_set_style_text_font(close_label, FONT_SMALL, 0);
    lv_obj_center(close_label);

    /*
     * 打开弹窗后先读一次结果。
     */
    ai_result_timer_cb(NULL);
}

static void create_records_page(lv_obj_t *scr)
{
    lv_obj_t *page = lv_obj_create(scr);
    pages[PAGE_RECORDS] = page;

    lv_obj_set_size(page, PAGE_W, PAGE_H);
    lv_obj_set_pos(page, PAGE_X, PAGE_Y);
    lv_obj_set_style_bg_opa(page, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(page, 0, 0);
    lv_obj_set_style_pad_all(page, 0, 0);
    lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);

    decorative_curve_lines(page);

    lv_obj_t *trend = glass_panel_create(page, 0, 0, 840, 260);
    title_create(trend, "4. RECORDS / FATIGUE SCORE TREND", 24, 14);

    rec_chart = lv_chart_create(trend);
    lv_obj_set_size(rec_chart, 760, 175);
    lv_obj_set_pos(rec_chart, 40, 62);
    lv_chart_set_type(rec_chart, LV_CHART_TYPE_LINE);
    lv_chart_set_range(rec_chart, LV_CHART_AXIS_PRIMARY_Y, 0, 100);
    lv_chart_set_point_count(rec_chart, 60);
    lv_chart_set_update_mode(rec_chart, LV_CHART_UPDATE_MODE_SHIFT);

    lv_obj_set_style_bg_color(rec_chart, lv_color_hex(0x061220), 0);
    lv_obj_set_style_bg_opa(rec_chart, OPACITY_70, 0);
    lv_obj_set_style_border_width(rec_chart, 0, 0);
    lv_obj_set_style_line_color(rec_chart, lv_color_hex(0x12344A), LV_PART_MAIN);
    lv_obj_set_style_size(rec_chart, 0, LV_PART_INDICATOR);

    rec_score_series = lv_chart_add_series(rec_chart, COLOR_GREEN, LV_CHART_AXIS_PRIMARY_Y);

    for (int i = 0; i < 60; i++) {
        lv_chart_set_next_value(rec_chart, rec_score_series, 0);
    }

    rec_time_labels[0] = label_create(trend, "--:--:--", 35, 232, COLOR_SUB_TEXT, FONT_SMALL);
    rec_time_labels[1] = label_create(trend, "--:--:--", 205, 232, COLOR_SUB_TEXT, FONT_SMALL);
    rec_time_labels[2] = label_create(trend, "--:--:--", 375, 232, COLOR_SUB_TEXT, FONT_SMALL);
    rec_time_labels[3] = label_create(trend, "--:--:--", 545, 232, COLOR_SUB_TEXT, FONT_SMALL);
    rec_time_labels[4] = label_create(trend, "--:--:--", 715, 232, COLOR_CYAN, FONT_SMALL);

    records_update_time_axis();

    lv_obj_t *events = glass_panel_create(page, 0, 275, 840, 215);
    title_create(events, "RECENT EVENTS", 24, 14);

    /* History 按钮：查看 CSV 历史记录 */
    lv_obj_t *btn_history = lv_btn_create(events);
    lv_obj_set_size(btn_history, 96, 34);
    lv_obj_set_pos(btn_history, 710, 12);
    lv_obj_set_style_radius(btn_history, 16, 0);
    lv_obj_set_style_bg_color(btn_history, lv_color_hex(0x071A2F), 0);
    lv_obj_set_style_bg_opa(btn_history, LV_OPA_70, 0);
    lv_obj_set_style_border_width(btn_history, 1, 0);
    lv_obj_set_style_border_color(btn_history, COLOR_CYAN, 0);
    lv_obj_set_style_border_opa(btn_history, LV_OPA_70, 0);
    lv_obj_add_event_cb(btn_history, show_history_popup_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *history_label = lv_label_create(btn_history);
    lv_label_set_text(history_label, "History");
    lv_obj_set_style_text_color(history_label, COLOR_CYAN, 0);
    lv_obj_set_style_text_font(history_label, FONT_SMALL, 0);
    lv_obj_center(history_label);

        /*
     * AI Copilot 按钮：打开端云协同风险研判弹窗。
     */
    lv_obj_t *btn_ai = lv_btn_create(events);
    lv_obj_set_size(btn_ai, 110, 34);
    lv_obj_set_pos(btn_ai, 580, 14);
    lv_obj_set_style_radius(btn_ai, 16, 0);
    lv_obj_set_style_bg_color(btn_ai, lv_color_hex(0x102B45), 0);
    lv_obj_set_style_bg_opa(btn_ai, LV_OPA_80, 0);
    lv_obj_set_style_border_width(btn_ai, 1, 0);
    lv_obj_set_style_border_color(btn_ai, COLOR_GREEN, 0);
    lv_obj_set_style_border_opa(btn_ai, LV_OPA_70, 0);
    lv_obj_add_event_cb(btn_ai, show_ai_copilot_popup_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *ai_label = lv_label_create(btn_ai);
    lv_label_set_text(ai_label, "AI Copilot");
    lv_obj_set_style_text_color(ai_label, COLOR_GREEN, 0);
    lv_obj_set_style_text_font(ai_label, FONT_SMALL, 0);
    lv_obj_center(ai_label);

    /*
     * 初始化事件文本。
     */
    snprintf(rec_event_texts[0], EVENT_TEXT_LEN, "--:--:--   System started");
    snprintf(rec_event_texts[1], EVENT_TEXT_LEN, "--:--:--   Camera and sensors connected");
    snprintf(rec_event_texts[2], EVENT_TEXT_LEN, "--:--:--   Risk engine active");
    snprintf(rec_event_texts[3], EVENT_TEXT_LEN, "--:--:--   Waiting for detection events");
    snprintf(rec_event_texts[4], EVENT_TEXT_LEN, "--:--:--   No abnormal alert");

    for (int i = 0; i < MAX_EVENT_RECORDS; i++) {
        rec_event_labels[i] = label_create(events,
                                           rec_event_texts[i],
                                           35,
                                           55 + i * 28,
                                           (i == 0) ? COLOR_GREEN : COLOR_SUB_TEXT,
                                           FONT_SMALL);
    }
}

/* =========================================================
 * Settings page
 * ========================================================= */
static void power_btn_event_cb(lv_event_t *e);

static void voice_switch_event_cb(lv_event_t *e)
{
    lv_obj_t *sw = lv_event_get_target(e);
    bool enabled = lv_obj_has_state(sw, LV_STATE_CHECKED);
    ui_update_voice_state(enabled);
}

static void write_text_file_simple(const char *path, const char *text)
{
    FILE *fp = fopen(path, "w");
    if (!fp) {
        return;
    }

    fprintf(fp, "%s\n", text);
    fclose(fp);
}

static const char *vehicle_mode_to_text(int mode)
{
    if (mode == 1) {
        return "DRIVING";
    } else if (mode == 2) {
        return "PARKED";
    } else {
        return "AUTO";
    }
}

static int vehicle_mode_from_text(const char *text)
{
    if (!text) {
        return 0;
    }

    if (strstr(text, "DRIVING") || strstr(text, "MOVING")) {
        return 1;
    }

    if (strstr(text, "PARKED") || strstr(text, "STOP")) {
        return 2;
    }

    if (strstr(text, "AUTO")) {
        return 0;
    }

    return 0;
}

static int read_vehicle_mode_file(void)
{
    char buf[64] = {0};

    /*
     * 优先读 runtime，因为现在 Dashboard / watchdog 主要靠它同步。
     */
    if (read_text_file_ui("/home/elf/ai_cloud/runtime/vehicle_mode",
                          buf,
                          sizeof(buf)) == 0) {
        return vehicle_mode_from_text(buf);
    }

    memset(buf, 0, sizeof(buf));
    if (read_text_file_ui("/tmp/cloud_vehicle_mode",
                          buf,
                          sizeof(buf)) == 0) {
        return vehicle_mode_from_text(buf);
    }

    memset(buf, 0, sizeof(buf));
    if (read_text_file_ui("/tmp/vehicle_mode",
                          buf,
                          sizeof(buf)) == 0) {
        return vehicle_mode_from_text(buf);
    }

    return 0;   /* 默认 AUTO */
}

static void write_vehicle_mode_file(int mode)
{
    const char *mode_text = vehicle_mode_to_text(mode);

    /*
     * 推荐优先调用统一脚本。
     * 这样以后 Dashboard、watchdog、UI 三边都一致。
     */
    char cmd[256];
    snprintf(cmd,
             sizeof(cmd),
             "/home/elf/ai_cloud/set_vehicle_mode.sh %s >/tmp/set_vehicle_mode_ui.log 2>&1",
             mode_text);

    int ret = system(cmd);

    /*
     * 如果脚本不存在或执行失败，直接写三个文件兜底。
     */
    if (ret != 0) {
        write_text_file_simple("/tmp/vehicle_mode", mode_text);
        write_text_file_simple("/tmp/cloud_vehicle_mode", mode_text);
        write_text_file_simple("/home/elf/ai_cloud/runtime/vehicle_mode", mode_text);
        write_text_file_simple("/home/elf/ai_cloud/runtime/cloud_vehicle_mode", mode_text);
    }
}

static void update_vehicle_mode_label(void)
{
    if (!setting_vehicle_mode_label) {
        return;
    }

    if (g_vehicle_mode == 1) {
        lv_label_set_text(setting_vehicle_mode_label, "Driving");
        lv_obj_set_style_text_color(setting_vehicle_mode_label, COLOR_GREEN, 0);
    } else if (g_vehicle_mode == 2) {
        lv_label_set_text(setting_vehicle_mode_label, "Parked");
        lv_obj_set_style_text_color(setting_vehicle_mode_label, COLOR_YELLOW, 0);
    } else {
        lv_label_set_text(setting_vehicle_mode_label, "Auto");
        lv_obj_set_style_text_color(setting_vehicle_mode_label, COLOR_CYAN, 0);
    }
}

static void vehicle_mode_btn_event_cb(lv_event_t *e)
{
    (void)e;

    g_vehicle_mode++;
    if (g_vehicle_mode > 2) {
        g_vehicle_mode = 0;
    }

    write_vehicle_mode_file(g_vehicle_mode);
    update_vehicle_mode_label();
}

static float read_cpu_usage_percent(void)
{
    static unsigned long long last_total = 0;
    static unsigned long long last_idle = 0;

    FILE *fp = fopen("/proc/stat", "r");
    if (!fp) {
        return -1.0f;
    }

    char cpu[8];
    unsigned long long user, nice, system, idle;
    unsigned long long iowait, irq, softirq, steal;

    int ret = fscanf(fp,
                     "%7s %llu %llu %llu %llu %llu %llu %llu %llu",
                     cpu,
                     &user,
                     &nice,
                     &system,
                     &idle,
                     &iowait,
                     &irq,
                     &softirq,
                     &steal);

    fclose(fp);

    if (ret < 8) {
        return -1.0f;
    }

    unsigned long long idle_all = idle + iowait;
    unsigned long long non_idle = user + nice + system + irq + softirq + steal;
    unsigned long long total = idle_all + non_idle;

    if (last_total == 0) {
        last_total = total;
        last_idle = idle_all;
        return 0.0f;
    }

    unsigned long long total_delta = total - last_total;
    unsigned long long idle_delta = idle_all - last_idle;

    last_total = total;
    last_idle = idle_all;

    if (total_delta == 0) {
        return 0.0f;
    }

    return (float)(total_delta - idle_delta) * 100.0f / (float)total_delta;
}

static void metrics_timer_cb(lv_timer_t *timer)
{
    (void)timer;

    static uint32_t last_tick = 0;
    static int last_camera_frame = -1;
    static int last_preview_frame = 0;

    uint32_t now = lv_tick_get();

    if (metric_start_tick == 0) {
        metric_start_tick = now;
    }

    if (last_tick == 0) {
        last_tick = now;

        if (g_camera_frame) {
            last_camera_frame = g_camera_frame->frame_count;
        }

        last_preview_frame = camera_preview_frame_count;
        return;
    }

    uint32_t dt = now - last_tick;
    if (dt < 900) {
        return;
    }

    float sec = (float)dt / 1000.0f;

    float camera_fps = 0.0f;
    float preview_fps = 0.0f;

    if (g_camera_frame && last_camera_frame >= 0) {
        int cur = g_camera_frame->frame_count;
        camera_fps = (float)(cur - last_camera_frame) / sec;
        last_camera_frame = cur;
    }

    preview_fps = (float)(camera_preview_frame_count - last_preview_frame) / sec;
    last_preview_frame = camera_preview_frame_count;

    last_tick = now;

    char buf[64];

    /* Runtime */
    uint32_t runtime_s = (now - metric_start_tick) / 1000;
    uint32_t h = runtime_s / 3600;
    uint32_t m = (runtime_s % 3600) / 60;
    uint32_t s = runtime_s % 60;

    snprintf(buf, sizeof(buf), "%02u:%02u:%02u", h, m, s);
    if (metric_runtime_label) {
        lv_label_set_text(metric_runtime_label, buf);
    }

    /* Camera FPS: 采集共享内存帧率 */
    snprintf(buf, sizeof(buf), "%.1f FPS", camera_fps);
    if (metric_camera_fps_label) {
        lv_label_set_text(metric_camera_fps_label, buf);
    }

    /*
     * NPU FPS:
     * 推荐显示 pose_infer_fps，因为它最接近你日志里的 rknn_run FPS。
     * 如果你想显示双模型完整链路，就把 pose_infer_fps 改成 ai_infer_fps。
     */
    float npu_fps = 0.0f;
    float infer_ms = 0.0f;

    if (g_behavior_result) {
        npu_fps = g_behavior_result->pose_infer_fps;
        infer_ms = g_behavior_result->pose_infer_ms;

        /*
         * 如果 pose_infer_fps 没写入，就兜底用 ai_infer_fps。
         */
        if (npu_fps <= 0.1f) {
            npu_fps = g_behavior_result->ai_infer_fps;
            infer_ms = g_behavior_result->ai_infer_ms;
        }
    }

    if (npu_fps > 0.1f) {
        snprintf(buf, sizeof(buf), "%.1f FPS", npu_fps);
    } else {
        snprintf(buf, sizeof(buf), "-- FPS");
    }

    if (metric_ai_fps_label) {
        lv_label_set_text(metric_ai_fps_label, buf);

        if (npu_fps >= 30.0f) {
            lv_obj_set_style_text_color(metric_ai_fps_label, COLOR_GREEN, 0);
        } else if (npu_fps >= 15.0f) {
            lv_obj_set_style_text_color(metric_ai_fps_label, COLOR_YELLOW, 0);
        } else {
            lv_obj_set_style_text_color(metric_ai_fps_label, COLOR_RED, 0);
        }
    }

    if (infer_ms > 0.1f) {
        snprintf(buf, sizeof(buf), "%.2f ms", infer_ms);
    } else {
        snprintf(buf, sizeof(buf), "-- ms");
    }

    if (metric_infer_ms_label) {
        lv_label_set_text(metric_infer_ms_label, buf);
    }

    /* Preview FPS: LVGL 实际预览刷新率 */
    snprintf(buf, sizeof(buf), "%.1f FPS", preview_fps);
    if (metric_preview_fps_label) {
        lv_label_set_text(metric_preview_fps_label, buf);
    }

    /* CPU Usage */
    float cpu = read_cpu_usage_percent();

    if (cpu >= 0.0f) {
        snprintf(buf, sizeof(buf), "%.0f%%", cpu);
    } else {
        snprintf(buf, sizeof(buf), "--%%");
    }

    if (metric_cpu_label) {
        lv_label_set_text(metric_cpu_label, buf);

        if (cpu >= 80.0f) {
            lv_obj_set_style_text_color(metric_cpu_label, COLOR_RED, 0);
        } else if (cpu >= 55.0f) {
            lv_obj_set_style_text_color(metric_cpu_label, COLOR_YELLOW, 0);
        } else {
            lv_obj_set_style_text_color(metric_cpu_label, COLOR_GREEN, 0);
        }
    }

    if (metric_stability_label) {
        lv_label_set_text(metric_stability_label, "Stable");
        lv_obj_set_style_text_color(metric_stability_label, COLOR_GREEN, 0);
    }
}

// static void restart_app_btn_event_cb(lv_event_t *e)
// {
//     lv_obj_t *btn = lv_event_get_target(e);

//     if (btn) {
//         lv_obj_t *label = lv_obj_get_child(btn, 3);  /* 你的 title_label 通常是第 3 个 child */
//         if (label) {
//             lv_label_set_text(label, "RESTARTING");
//         }
//         lv_obj_set_style_bg_color(btn, lv_color_hex(0x1A4A6A), 0);
//         lv_obj_invalidate(btn);
//     }

//     /*
//      * 关键：
//      * 不要在当前服务 cgroup 里直接 systemctl restart 自己。
//      * 用 systemd-run 创建一个临时独立任务，延迟 1 秒后重启 smart_cockpit。
//      */
//     (void)system(
//         "systemd-run --unit=smart-cockpit-restart --collect "
//         "/bin/bash -lc 'sleep 1; systemctl restart smart_cockpit.service' &"
//     );
// }

// static void shutdown_system_btn_event_cb(lv_event_t *e)
// {
//     lv_obj_t *btn = lv_event_get_target(e);

//     if (btn) {
//         lv_obj_t *label = lv_obj_get_child(btn, 3);
//         if (label) {
//             lv_label_set_text(label, "POWERING OFF");
//         }
//         lv_obj_set_style_bg_color(btn, lv_color_hex(0x5A1018), 0);
//         lv_obj_invalidate(btn);
//     }

//     /*
//      * 同理，关机也放到独立 systemd-run 临时任务里。
//      * 否则 stop 当前服务时可能先把 UI 自己杀掉，poweroff 没执行到。
//      */
//     (void)system(
//         "systemd-run --unit=smart-cockpit-poweroff --collect "
//         "/bin/bash -lc 'sleep 1; poweroff' &"
//     );
// }

static void restart_app_btn_event_cb(lv_event_t *e)
{
    lv_obj_t *btn = lv_event_get_target(e);

    if (btn) {
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x1A4A6A), 0);
        lv_obj_invalidate(btn);
    }

    /*
     * Use systemd-run to restart from outside current UI process.
     */
    (void)system(
        "systemd-run --unit=smart-cockpit-restart --collect "
        "/bin/bash -lc 'sleep 1; systemctl restart smart_cockpit.service' &"
    );
}

static void power_main_btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);

    if (code == LV_EVENT_PRESSED) {
        power_press_tick = lv_tick_get();
        power_shutdown_dialog_shown = 0;

        if (btn) {
            lv_obj_set_style_bg_color(btn, lv_color_hex(0x0B2E45), 0);
            lv_obj_set_style_shadow_opa(btn, LV_OPA_60, 0);
            lv_obj_invalidate(btn);
        }
    }
    else if (code == LV_EVENT_PRESSING) {
        uint32_t held = lv_tick_elaps(power_press_tick);

        if (btn) {
            if (held >= POWER_SHUTDOWN_HOLD_MS) {
                lv_obj_set_style_border_color(btn, COLOR_RED, 0);
                lv_obj_set_style_shadow_color(btn, COLOR_RED, 0);
                lv_obj_set_style_bg_color(btn, lv_color_hex(0x4A1018), 0);
            } else if (held >= POWER_RESTART_HOLD_MS) {
                lv_obj_set_style_border_color(btn, COLOR_CYAN, 0);
                lv_obj_set_style_shadow_color(btn, COLOR_CYAN, 0);
                lv_obj_set_style_bg_color(btn, lv_color_hex(0x123A5A), 0);
            }

            lv_obj_invalidate(btn);
        }

        /*
         * 长按达到关机阈值后，自动弹出确认框。
         * 用 power_shutdown_dialog_shown 防止一直重复创建弹窗。
         */
        if (held >= POWER_SHUTDOWN_HOLD_MS && !power_shutdown_dialog_shown) {
            power_shutdown_dialog_shown = 1;
            show_shutdown_confirm_dialog();
        }
    }
    else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
        uint32_t held = lv_tick_elaps(power_press_tick);

        if (btn) {
            lv_obj_set_style_bg_color(btn, lv_color_hex(0x081A2D), 0);
            lv_obj_set_style_border_color(btn, COLOR_CYAN, 0);
            lv_obj_set_style_shadow_color(btn, COLOR_CYAN, 0);
            lv_obj_set_style_shadow_opa(btn, LV_OPA_40, 0);
            lv_obj_invalidate(btn);
        }

        /*
         * 如果已经弹出关机确认框，松手时不要再执行重启。
         */
        if (!power_shutdown_dialog_shown && held >= POWER_RESTART_HOLD_MS) {
            int ret = system(
                "systemd-run --unit=smart-cockpit-restart-$(date +%s) --collect "
                "/bin/bash -lc 'sleep 0.3; systemctl restart --no-block smart_cockpit.service' &"
            );
            (void)ret;
        }

        power_press_tick = 0;
    }
}

static lv_obj_t *power_tile_create(lv_obj_t *parent,
                                   int x, int y,
                                   int w, int h,
                                   const char *title,
                                   const char *sub,
                                   lv_color_t main_color,
                                   lv_event_cb_t cb)
{
    (void)sub;

    lv_obj_t *tile = lv_btn_create(parent);
    lv_obj_set_size(tile, w, h);
    lv_obj_set_pos(tile, x, y);
    lv_obj_clear_flag(tile, LV_OBJ_FLAG_SCROLLABLE);

    /* 整体卡片 */
    lv_obj_set_style_radius(tile, 22, 0);
    lv_obj_set_style_bg_color(tile, lv_color_hex(0x081A2D), 0);
    lv_obj_set_style_bg_opa(tile, LV_OPA_60, 0);

    lv_obj_set_style_border_width(tile, 2, 0);
    lv_obj_set_style_border_color(tile, main_color, 0);
    lv_obj_set_style_border_opa(tile, LV_OPA_80, 0);

    lv_obj_set_style_shadow_width(tile, 24, 0);
    lv_obj_set_style_shadow_color(tile, main_color, 0);
    lv_obj_set_style_shadow_opa(tile, LV_OPA_40, 0);
    lv_obj_set_style_shadow_spread(tile, 1, 0);

    lv_obj_set_style_pad_all(tile, 0, 0);

    /* 按下反馈 */
    lv_obj_set_style_bg_color(tile, lv_color_hex(0x10304A), LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(tile, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_set_style_shadow_opa(tile, LV_OPA_50, LV_STATE_PRESSED);

    if (cb) {
        lv_obj_add_event_cb(tile, cb, LV_EVENT_ALL, NULL);
    }

    /* 只保留电源图标，不要圆圈，不要椭圆底 */
    lv_obj_t *icon = lv_label_create(tile);
    lv_label_set_text(icon, LV_SYMBOL_POWER);
    lv_obj_set_style_text_color(icon, main_color, 0);
    lv_obj_set_style_text_font(icon, &lv_font_montserrat_28, 0);
    lv_obj_align(icon, LV_ALIGN_TOP_MID, 0, 16);
    lv_obj_clear_flag(icon, LV_OBJ_FLAG_CLICKABLE);

    /* 给图标加一点文字阴影，增强发光感 */
    lv_obj_set_style_text_opa(icon, LV_OPA_COVER, 0);

    /* POWER 字样，缩小 */
    lv_obj_t *label = lv_label_create(tile);
    lv_label_set_text(label, title ? title : "POWER");
    lv_obj_set_style_text_color(label, main_color, 0);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);
    lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, -14);
    lv_obj_clear_flag(label, LV_OBJ_FLAG_CLICKABLE);

    return tile;
}

static void shutdown_confirm_cancel_cb(lv_event_t *e)
{
    (void)e;

    if (shutdown_confirm_mask) {
        lv_obj_del(shutdown_confirm_mask);
        shutdown_confirm_mask = NULL;
    }

    shutdown_confirm_box = NULL;
    power_shutdown_dialog_shown = 0;
}

static void shutdown_confirm_yes_cb(lv_event_t *e)
{
    (void)e;

    if (shutdown_confirm_mask) {
        lv_obj_del(shutdown_confirm_mask);
        shutdown_confirm_mask = NULL;
    }

    shutdown_confirm_box = NULL;
    power_shutdown_dialog_shown = 0;

    int ret = system(
        "systemd-run --unit=smart-cockpit-poweroff-$(date +%s) --collect "
        "/bin/bash -lc 'sleep 1; poweroff' &"
    );
    (void)ret;
}

static void show_shutdown_confirm_dialog(void)
{
    if (shutdown_confirm_mask) {
        return;   /* 已经弹出时不重复创建 */
    }

    /* 半透明遮罩层 */
    shutdown_confirm_mask = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(shutdown_confirm_mask);
    lv_obj_set_size(shutdown_confirm_mask, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(shutdown_confirm_mask, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(shutdown_confirm_mask, LV_OPA_40, 0);
    lv_obj_clear_flag(shutdown_confirm_mask, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(shutdown_confirm_mask, LV_OBJ_FLAG_CLICKABLE);

    /* 弹窗主体 */
    lv_obj_t *box = lv_obj_create(shutdown_confirm_mask);
    shutdown_confirm_box = box;
    lv_obj_set_size(box, 430, 220);
    lv_obj_center(box);

    lv_obj_set_style_radius(box, 26, 0);
    lv_obj_set_style_bg_color(box, lv_color_hex(0x101C3A), 0);
    lv_obj_set_style_bg_opa(box, LV_OPA_90, 0);

    lv_obj_set_style_border_width(box, 3, 0);
    lv_obj_set_style_border_color(box, lv_color_hex(0xFF4FA3), 0);
    lv_obj_set_style_border_opa(box, LV_OPA_90, 0);

    lv_obj_set_style_shadow_width(box, 24, 0);
    lv_obj_set_style_shadow_color(box, lv_color_hex(0xFF4FA3), 0);
    lv_obj_set_style_shadow_opa(box, LV_OPA_40, 0);

    lv_obj_set_style_pad_all(box, 0, 0);
    lv_obj_clear_flag(box, LV_OBJ_FLAG_SCROLLABLE);

    /* 标题 */
    lv_obj_t *title = lv_label_create(box);
    lv_label_set_text(title, "CONFIRM SHUTDOWN");
    lv_obj_set_style_text_color(title, lv_color_hex(0xFF6FB7), 0);
    lv_obj_set_style_text_font(title, FONT_NORMAL, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 28);

    /* 提示文字 */
    lv_obj_t *msg = lv_label_create(box);
    lv_label_set_text(msg, "Power off the whole device?");
    lv_obj_set_style_text_color(msg, COLOR_CYAN, 0);
    lv_obj_set_style_text_font(msg, FONT_SMALL, 0);
    lv_obj_align(msg, LV_ALIGN_TOP_MID, 0, 82);

    /* Cancel 按钮 */
    lv_obj_t *btn_cancel = lv_btn_create(box);
    lv_obj_set_size(btn_cancel, 120, 42);
    lv_obj_set_pos(btn_cancel, 65, 145);

    lv_obj_set_style_radius(btn_cancel, 14, 0);
    lv_obj_set_style_bg_color(btn_cancel, lv_color_hex(0x1E5D87), 0);
    lv_obj_set_style_bg_opa(btn_cancel, LV_OPA_80, 0);
    lv_obj_set_style_border_width(btn_cancel, 2, 0);
    lv_obj_set_style_border_color(btn_cancel, COLOR_CYAN, 0);
    lv_obj_set_style_shadow_width(btn_cancel, 14, 0);
    lv_obj_set_style_shadow_color(btn_cancel, COLOR_CYAN, 0);
    lv_obj_set_style_shadow_opa(btn_cancel, LV_OPA_20, 0);

    lv_obj_t *cancel_label = lv_label_create(btn_cancel);
    lv_label_set_text(cancel_label, "Cancel");
    lv_obj_set_style_text_color(cancel_label, COLOR_CYAN, 0);
    lv_obj_set_style_text_font(cancel_label, FONT_SMALL, 0);
    lv_obj_center(cancel_label);

    lv_obj_add_event_cb(btn_cancel, shutdown_confirm_cancel_cb, LV_EVENT_CLICKED, NULL);

    /* Shutdown 按钮 */
    lv_obj_t *btn_yes = lv_btn_create(box);
    lv_obj_set_size(btn_yes, 120, 42);
    lv_obj_set_pos(btn_yes, 245, 145);

    lv_obj_set_style_radius(btn_yes, 14, 0);
    lv_obj_set_style_bg_color(btn_yes, lv_color_hex(0x6E254A), 0);
    lv_obj_set_style_bg_opa(btn_yes, LV_OPA_90, 0);
    lv_obj_set_style_border_width(btn_yes, 2, 0);
    lv_obj_set_style_border_color(btn_yes, lv_color_hex(0xFF5AAE), 0);
    lv_obj_set_style_shadow_width(btn_yes, 14, 0);
    lv_obj_set_style_shadow_color(btn_yes, lv_color_hex(0xFF5AAE), 0);
    lv_obj_set_style_shadow_opa(btn_yes, LV_OPA_20, 0);

    lv_obj_t *yes_label = lv_label_create(btn_yes);
    lv_label_set_text(yes_label, "Shutdown");
    lv_obj_set_style_text_color(yes_label, lv_color_hex(0xFF8CC8), 0);
    lv_obj_set_style_text_font(yes_label, FONT_SMALL, 0);
    lv_obj_center(yes_label);

    lv_obj_add_event_cb(btn_yes, shutdown_confirm_yes_cb, LV_EVENT_CLICKED, NULL);
}

static void brightness_slider_event_cb(lv_event_t *e)
{
    lv_obj_t *slider = lv_event_get_target(e);
    int value = lv_slider_get_value(slider);   /* 10 - 100 */

    const char *brightness_path = "/sys/class/backlight/backlight-dsi0/brightness";
    const char *max_path = "/sys/class/backlight/backlight-dsi0/max_brightness";

    FILE *fp = fopen(max_path, "r");
    if (!fp) {
        return;
    }

    int max_brightness = 0;
    if (fscanf(fp, "%d", &max_brightness) != 1) {
        fclose(fp);
        return;
    }
    fclose(fp);

    if (max_brightness <= 0) {
        return;
    }

    /*
     * 最低不允许到 0，防止屏幕黑掉不好恢复。
     */
    int min_value = max_brightness / 10;
    if (min_value < 1) {
        min_value = 1;
    }

    int hw_value = max_brightness * value / 100;
    if (hw_value < min_value) {
        hw_value = min_value;
    }

    fp = fopen(brightness_path, "w");
    if (!fp) {
        return;
    }

    fprintf(fp, "%d\n", hw_value);
    fclose(fp);
}

static void create_settings_page(lv_obj_t *scr)
{
    lv_obj_t *page = lv_obj_create(scr);
    pages[PAGE_SETTINGS] = page;

    lv_obj_set_size(page, PAGE_W, PAGE_H);
    lv_obj_set_pos(page, PAGE_X, PAGE_Y);
    lv_obj_set_style_bg_opa(page, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(page, 0, 0);
    lv_obj_set_style_pad_all(page, 0, 0);
    lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);

    decorative_curve_lines(page);

    lv_obj_t *settings = glass_panel_create(page, 0, 0, 500, 440);
    title_create(settings, "5. SETTINGS / SYSTEM SETTINGS", 24, 18);

    label_create(settings, "Voice Broadcast", 45, 80, COLOR_SUB_TEXT, FONT_NORMAL);
    setting_voice_state = label_create(settings, "On", 310, 80, COLOR_GREEN, FONT_NORMAL);

    sw_voice = lv_switch_create(settings);
    lv_obj_set_pos(sw_voice, 405, 76);
    lv_obj_add_state(sw_voice, LV_STATE_CHECKED);
    lv_obj_add_event_cb(sw_voice, voice_switch_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    label_create(settings, "Warning Threshold", 45, 130, COLOR_SUB_TEXT, FONT_NORMAL);
    label_create(settings, "75", 405, 130, COLOR_CYAN, FONT_NORMAL);

    label_create(settings, "Danger Threshold", 45, 180, COLOR_SUB_TEXT, FONT_NORMAL);
    label_create(settings, "90", 405, 180, COLOR_CYAN, FONT_NORMAL);

    label_create(settings, "Fan Default Mode", 45, 230, COLOR_SUB_TEXT, FONT_NORMAL);
    label_create(settings, "Auto", 370, 230, COLOR_GREEN, FONT_NORMAL);

    /* 新增：Vehicle Mode */
    label_create(settings, "Vehicle Mode", 45, 280, COLOR_SUB_TEXT, FONT_NORMAL);

    btn_vehicle_mode = lv_btn_create(settings);
    lv_obj_set_size(btn_vehicle_mode, 120, 34);
    lv_obj_set_pos(btn_vehicle_mode, 345, 270);
    lv_obj_set_style_radius(btn_vehicle_mode, 16, 0);
    lv_obj_set_style_bg_color(btn_vehicle_mode, lv_color_hex(0x071A2F), 0);
    lv_obj_set_style_bg_opa(btn_vehicle_mode, LV_OPA_60, 0);
    lv_obj_set_style_border_width(btn_vehicle_mode, 1, 0);
    lv_obj_set_style_border_color(btn_vehicle_mode, COLOR_CYAN, 0);
    lv_obj_set_style_border_opa(btn_vehicle_mode, LV_OPA_70, 0);
    lv_obj_add_event_cb(btn_vehicle_mode, vehicle_mode_btn_event_cb, LV_EVENT_CLICKED, NULL);

    setting_vehicle_mode_label = lv_label_create(btn_vehicle_mode);
    lv_label_set_text(setting_vehicle_mode_label, "Auto");
    lv_obj_set_style_text_color(setting_vehicle_mode_label, COLOR_CYAN, 0);
    lv_obj_set_style_text_font(setting_vehicle_mode_label, FONT_SMALL, 0);
    lv_obj_center(setting_vehicle_mode_label);

    /*
    * 不要开机强制写 AUTO。
    * 现在 start_demo.sh 可能已经把车辆状态设为 DRIVING。
    * UI 启动时应该读取当前真实状态，而不是覆盖它。
    */
    g_vehicle_mode = read_vehicle_mode_file();
    update_vehicle_mode_label();

    label_create(settings, "Screen Brightness", 45, 335, COLOR_SUB_TEXT, FONT_NORMAL);

    lv_obj_t *bright = lv_slider_create(settings);
    lv_obj_set_size(bright, 260, 12);
    lv_obj_set_pos(bright, 200, 342);

    /* 最低 10%，防止误滑黑屏 */
    lv_slider_set_range(bright, 10, 100);
    lv_slider_set_value(bright, 80, LV_ANIM_OFF);

    lv_obj_add_event_cb(bright, brightness_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    lv_obj_t *control = glass_panel_create(page, 535, 0, 305, 440);
    title_create(control, "SYSTEM METRICS", 24, 14);

    label_create(control, "Runtime", 35, 55, COLOR_SUB_TEXT, FONT_SMALL);
    metric_runtime_label = label_create(control, "00:00:00", 165, 55, COLOR_CYAN, FONT_SMALL);

    label_create(control, "Camera FPS", 35, 85, COLOR_SUB_TEXT, FONT_SMALL);
    metric_camera_fps_label = label_create(control, "-- FPS", 165, 85, COLOR_GREEN, FONT_SMALL);

    label_create(control, "NPU FPS", 35, 115, COLOR_SUB_TEXT, FONT_SMALL);
    metric_ai_fps_label = label_create(control, "-- FPS", 165, 115, COLOR_GREEN, FONT_SMALL);

    label_create(control, "Infer Time", 35, 145, COLOR_SUB_TEXT, FONT_SMALL);
    metric_infer_ms_label = label_create(control, "-- ms", 165, 145, COLOR_CYAN, FONT_SMALL);

    label_create(control, "Preview FPS", 35, 175, COLOR_SUB_TEXT, FONT_SMALL);
    metric_preview_fps_label = label_create(control, "-- FPS", 165, 175, COLOR_CYAN, FONT_SMALL);

    label_create(control, "CPU Usage", 35, 205, COLOR_SUB_TEXT, FONT_SMALL);
    metric_cpu_label = label_create(control, "--%", 165, 205, COLOR_GREEN, FONT_SMALL);

    label_create(control, "Stability", 35, 235, COLOR_SUB_TEXT, FONT_SMALL);
    metric_stability_label = label_create(control, "Stable", 165, 235, COLOR_GREEN, FONT_SMALL);

    /* 分割发光线 */
    lv_obj_t *split_line = lv_obj_create(control);
    lv_obj_set_size(split_line, 230, 2);
    lv_obj_set_pos(split_line, 38, 270);
    lv_obj_set_style_bg_color(split_line, COLOR_CYAN, 0);
    lv_obj_set_style_bg_opa(split_line, LV_OPA_50, 0);
    lv_obj_set_style_border_width(split_line, 0, 0);
    lv_obj_set_style_radius(split_line, 2, 0);

    /* ===================== SYSTEM CONTROL ===================== */
    label_create(control, "SYSTEM CONTROL", 35, 286, COLOR_CYAN, FONT_SMALL);

    /*
     * 单个电源控制大方块：
     * 短按：只反馈
     * 长按 3s：Restart App
     * 长按 7s：弹出关机确认
     */
    int power_w = 200;
    int power_h = 96;
    int power_x = (305 - power_w) / 2;
    int power_y = 324;

    lv_obj_t *power_tile = power_tile_create(control,
                                             power_x, power_y,
                                             power_w, power_h,
                                             "POWER",
                                             "Hold",
                                             COLOR_CYAN,
                                             NULL);

    lv_obj_add_event_cb(power_tile, power_main_btn_event_cb, LV_EVENT_ALL, NULL);
}

/* =========================================================
 * Create UI
 * ========================================================= */
void ui_fatigue_create(void)
{
    lv_obj_t *scr = lv_scr_act();

    lv_obj_set_style_bg_color(scr, COLOR_BG, 0);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

    create_header(scr);
    create_sidebar(scr);

    create_home_page(scr);
    create_detection_page(scr);
    create_environment_page(scr);
    create_records_page(scr);
    create_settings_page(scr);

    metric_start_tick = lv_tick_get();
    lv_timer_create(metrics_timer_cb, 1000, NULL);

    /*
     * AI Copilot 结果刷新定时器：
     * 每 1 秒读取 /home/elf/ai_cloud/runtime/llm_answer /home/elf/ai_cloud/runtime/cloud_status /home/elf/ai_cloud/runtime/cloud_latency。
     */
    lv_timer_create(ai_result_timer_cb, 1000, NULL);

    lv_timer_create(danger_flash_timer_cb, 180, NULL);

    lv_timer_create(ai_auto_popup_timer_cb, 300, NULL);

    page_show(PAGE_HOME);
}

/* Current version has shutdown in Settings page */
void ui_add_power_button(void)
{
}

/* =========================================================
 * Update functions
 * ========================================================= */
// void ui_update_fatigue_score(int score)
// {
//     score = clamp_int(score, 0, 100);

//     char buf[16];
//     snprintf(buf, sizeof(buf), "%d", score);

//     if (home_score_label) {
//         lv_label_set_text(home_score_label, buf);
//     }

//     if (home_score_bar) {
//         lv_bar_set_value(home_score_bar, score, LV_ANIM_OFF);
//     }

//     if (home_state_arc) {
//         lv_arc_set_value(home_state_arc, score);
//     }

//     if (rec_chart && rec_score_series) {
//         lv_chart_set_next_value(rec_chart, rec_score_series, score);
//         lv_chart_refresh(rec_chart);
//     }

//     if (score <= 30) {
//         ui_update_state("NORMAL", COLOR_GREEN);
//     } else if (score <= 60) {
//         ui_update_state("MILD RISK", COLOR_YELLOW);
//     } else if (score <= 80) {
//         ui_update_state("DROWSY", COLOR_ORANGE);
//     } else {
//         ui_update_state("DANGEROUS", COLOR_RED);
//     }
// }

void ui_update_fatigue_score(int score)
{
    static int display_score = 0;
    static int last_chart_score = 0;
    static uint32_t last_chart_tick = 0;

    score = clamp_int(score, 0, 100);

    /*
     * UI显示分数做平滑处理：
     * 每次最多变化 2 分，避免进度条抖动。
     */
    if (display_score < score) {
        display_score += 2;
        if (display_score > score) display_score = score;
    } else if (display_score > score) {
        display_score -= 2;
        if (display_score < score) display_score = score;
    }

    char buf[16];
    snprintf(buf, sizeof(buf), "%d", display_score);

    if (home_score_label) {
        lv_label_set_text(home_score_label, buf);
    }

    /*
    * Detect 页面同步显示 Risk Score。
    * 这样演示时不用切回 Home 页面看状态分。
    */
    if (detect_risk_score_label) {
        lv_label_set_text(detect_risk_score_label, buf);

        if (display_score >= 80) {
            lv_obj_set_style_text_color(detect_risk_score_label, COLOR_RED, 0);
        } else if (display_score >= 50) {
            lv_obj_set_style_text_color(detect_risk_score_label, COLOR_YELLOW, 0);
        } else {
            lv_obj_set_style_text_color(detect_risk_score_label, COLOR_GREEN, 0);
        }
    }

    /*
     * 关键：这里用 LV_ANIM_OFF，不要每 500ms 重新开动画。
     */
    if (home_score_bar) {
        lv_bar_set_value(home_score_bar, display_score, LV_ANIM_OFF);
    }

    if (home_state_arc) {
        lv_arc_set_value(home_state_arc, display_score);
    }

    /*
     * 曲线图不要每 500ms 刷一次。
     * 1秒记录一个点就够了，否则容易增加重绘压力。
     */
    uint32_t now = lv_tick_get();

    if (rec_chart && rec_score_series && now - last_chart_tick >= 1000) {
    last_chart_tick = now;
    last_chart_score = display_score;

    lv_chart_set_next_value(rec_chart, rec_score_series, last_chart_score);
    lv_chart_refresh(rec_chart);

    records_update_time_axis();
}

    /*
     * 状态判断仍然用原始 score，响应更及时。
     * 如果你想状态也更稳，可以后面再加迟滞判断。
     */
int fatigue_level = 0;

if (score <= 30) {
    fatigue_level = 0;
    ui_update_state("NORMAL", COLOR_GREEN);
} else if (score <= 60) {
    fatigue_level = 1;
    ui_update_state("MILD RISK", COLOR_YELLOW);
} else if (score <= 80) {
    fatigue_level = 2;
    ui_update_state("DROWSY", COLOR_ORANGE);
} else {
    fatigue_level = 3;
    ui_update_state("DANGEROUS", COLOR_RED);
}

/*
 * 只在疲劳等级变化时记录，避免每 500ms 刷屏。
 */
if (last_fatigue_level != fatigue_level) {
    last_fatigue_level = fatigue_level;

    if (fatigue_level == 0) {
        records_add_event("Fatigue risk normal", COLOR_GREEN);
    } else if (fatigue_level == 1) {
        records_add_event("Mild fatigue risk warning", COLOR_YELLOW);
    } else if (fatigue_level == 2) {
        records_add_event("Drowsy risk warning", COLOR_ORANGE);
    } else {
        records_add_event("Dangerous fatigue risk", COLOR_RED);
    }
}

}

void ui_update_state(const char *state_text, lv_color_t color)
{
    if (home_state_label) {
        lv_label_set_text(home_state_label, state_text);
        lv_obj_set_style_text_color(home_state_label, color, 0);
    }

    if (det_driver_state_label) {
        lv_label_set_text(det_driver_state_label, state_text);
        lv_obj_set_style_text_color(det_driver_state_label, color, 0);
    }

    if (home_state_icon) {
        if (strcmp(state_text, "NORMAL") == 0) {
            lv_label_set_text(home_state_icon, ":)");
        } else if (strcmp(state_text, "NO DRIVER") == 0) {
            lv_label_set_text(home_state_icon, "--");
        } else if (strcmp(state_text, "MILD RISK") == 0) {
            lv_label_set_text(home_state_icon, ":|");
        } else if (strcmp(state_text, "DROWSY") == 0) {
            lv_label_set_text(home_state_icon, "Zz");
        } else {
            lv_label_set_text(home_state_icon, "!!");
        }

        lv_obj_set_style_text_color(home_state_icon, color, 0);
    }

    if (home_state_arc) {
        lv_obj_set_style_arc_color(home_state_arc, color, LV_PART_INDICATOR);
    }

    if (home_state_desc) {
        if (strcmp(state_text, "NORMAL") == 0) {
            lv_label_set_text(home_state_desc, "Driver status is good. Keep focused.");
        } else if (strcmp(state_text, "MILD RISK") == 0) {
            lv_label_set_text(home_state_desc, "Attention is decreasing. Please focus.");
        } else if (strcmp(state_text, "DROWSY") == 0) {
            lv_label_set_text(home_state_desc, "Drowsy risk detected. Rest is recommended.");
        } else {
            lv_label_set_text(home_state_desc, "Dangerous state. Please stop driving.");
        }
    }

    if (home_alert_text) {
        if (strcmp(state_text, "NORMAL") == 0) {
            lv_label_set_text(home_alert_text, "No abnormal alert");
            lv_obj_set_style_text_color(home_alert_text, COLOR_GREEN, 0);
        } else if (strcmp(state_text, "MILD RISK") == 0) {
            lv_label_set_text(home_alert_text, "Please pay attention");
            lv_obj_set_style_text_color(home_alert_text, COLOR_YELLOW, 0);
        } else if (strcmp(state_text, "DROWSY") == 0) {
            lv_label_set_text(home_alert_text, "Rest is recommended");
            lv_obj_set_style_text_color(home_alert_text, COLOR_ORANGE, 0);
        } else {
            lv_label_set_text(home_alert_text, "Danger! Stop driving");
            lv_obj_set_style_text_color(home_alert_text, COLOR_RED, 0);
        }
    }

    if (det_risk_text) {
        char buf[64];
        snprintf(buf, sizeof(buf), "Current Risk:");
        lv_label_set_text(det_risk_text, buf);
        lv_obj_set_style_text_color(det_risk_text, COLOR_SUB_TEXT, 0);
    }

    if (det_explain_text) {
        if (strcmp(state_text, "NORMAL") == 0) {
            lv_label_set_text(det_explain_text, "No abnormal behavior.");
        } else {
            lv_label_set_text(det_explain_text, "Risk rising.");
        }
    }
}

void ui_update_eye_state(const char *eye_state)
{
    if (!eye_state) return;

    if (det_eye_state) {
        lv_label_set_text(det_eye_state, eye_state);

        if (strcmp(eye_state, "Normal") == 0 || strcmp(eye_state, "NORMAL") == 0) {
            set_label_color(det_eye_state, COLOR_GREEN);
        } else if (strcmp(eye_state, "Closed") == 0) {
            set_label_color(det_eye_state, COLOR_RED);
        } else {
            set_label_color(det_eye_state, COLOR_YELLOW);
        }
    }
}

void ui_update_perclos(int perclos)
{
    char buf[16];
    perclos = clamp_int(perclos, 0, 100);
    snprintf(buf, sizeof(buf), "%d%%", perclos);

    if (det_perclos) {
        lv_label_set_text(det_perclos, buf);
    }
}

void ui_update_blink_freq(int blink_freq)
{
    char buf[32];
    snprintf(buf, sizeof(buf), "%d/min", blink_freq);

    if (det_blink) {
        lv_label_set_text(det_blink, buf);
    }
}

void ui_update_yawn_count(int yawn_count)
{
    // char buf[16];
    // snprintf(buf, sizeof(buf), "%d", yawn_count);

    // if (det_yawn) {
    //     lv_label_set_text(det_yawn, buf);
    // }
    (void)yawn_count;
}

void ui_update_phone_state(int detected)
{
    pill_update(home_phone_pill, detected);
    pill_update(det_phone_pill, detected);

    if (last_phone_state != detected) {
        last_phone_state = detected;

        if (detected) {
            records_add_event("Phone use signal detected", COLOR_YELLOW);
        } else {
            records_add_event("Phone signal cleared", COLOR_GREEN);
        }
    }
}

void ui_update_smoking_state(int detected)
{
    /*
     * 注意：后端 smoking 字段现在兼容表示 mouth_risk。
     * UI 上不要再叫 Smoking，统一显示为 Mouth Risk。
     */
    pill_update(home_smoke_pill, detected);
    pill_update(det_smoke_pill, detected);

    if (last_smoking_state != detected) {
        last_smoking_state = detected;

        if (detected) {
            records_add_event("Mouth risk detected", COLOR_YELLOW);
        } else {
            records_add_event("Mouth risk cleared", COLOR_GREEN);
        }
    }
}

static void wheel_pill_update(lv_obj_t *label, int hands_off)
{
    if (!label) return;

    lv_obj_t *pill = lv_obj_get_parent(label);
    if (!pill) return;

    /*
     * hands_off:
     * -1 = No Driver / Unknown
     *  0 = Wheel On
     *  1 = Wheel Off
     */
    if (hands_off < 0) {
        lv_label_set_text(label, "--");
        lv_obj_set_style_text_color(label, COLOR_SUB_TEXT, 0);
        lv_obj_set_style_border_color(pill, COLOR_BORDER_DIM, 0);
        lv_obj_set_style_bg_color(pill, lv_color_hex(0x101826), 0);
    } else if (hands_off) {
        lv_label_set_text(label, "Off");
        lv_obj_set_style_text_color(label, COLOR_RED, 0);
        lv_obj_set_style_border_color(pill, COLOR_RED, 0);
        lv_obj_set_style_bg_color(pill, lv_color_hex(0x361224), 0);
    } else {
        lv_label_set_text(label, "On");
        lv_obj_set_style_text_color(label, COLOR_GREEN, 0);
        lv_obj_set_style_border_color(pill, COLOR_GREEN, 0);
        lv_obj_set_style_bg_color(pill, lv_color_hex(0x092C25), 0);
    }
}

void ui_update_wheel_state(int hands_off)
{
    /*
     * hands_off:
     * -1 = No Driver / Unknown
     *  0 = Wheel On
     *  1 = Wheel Off
     */

    if (home_wheel_pill) {
        wheel_pill_update(home_wheel_pill, hands_off);
    }

    if (det_yawn) {
        if (hands_off < 0) {
            lv_label_set_text(det_yawn, "--");
            lv_obj_set_style_text_color(det_yawn, COLOR_SUB_TEXT, 0);
        } else if (hands_off) {
            lv_label_set_text(det_yawn, "Off");
            lv_obj_set_style_text_color(det_yawn, COLOR_RED, 0);
        } else {
            lv_label_set_text(det_yawn, "On");
            lv_obj_set_style_text_color(det_yawn, COLOR_GREEN, 0);
        }
    }
}

void ui_update_vehicle_state(int moving)
{
    if (!det_vehicle_state) {
        return;
    }

    if (moving > 0) {
        lv_label_set_text(det_vehicle_state, "MOVING");
        lv_obj_set_style_text_color(det_vehicle_state, COLOR_GREEN, 0);
    } else if (moving == 0) {
        lv_label_set_text(det_vehicle_state, "PARKED");
        lv_obj_set_style_text_color(det_vehicle_state, COLOR_YELLOW, 0);
    } else {
        lv_label_set_text(det_vehicle_state, "--");
        lv_obj_set_style_text_color(det_vehicle_state, COLOR_SUB_TEXT, 0);
    }
}

void ui_update_eye_risk(float eye_risk)
{
    /*
     * eye_risk:
     * < 0.0f = No Driver / Unknown
     * 0.0f ~ 1.0f = valid eye risk
     */
    if (eye_risk < 0.0f) {
        if (det_perclos) {
            lv_label_set_text(det_perclos, "--");
            lv_obj_set_style_text_color(det_perclos, COLOR_SUB_TEXT, 0);
        }

        if (det_eye_state) {
            lv_label_set_text(det_eye_state, "--");
            lv_obj_set_style_text_color(det_eye_state, COLOR_SUB_TEXT, 0);
        }

        return;
    }

    if (eye_risk > 1.0f) eye_risk = 1.0f;

    int percent = (int)(eye_risk * 100.0f + 0.5f);

    char buf[16];
    snprintf(buf, sizeof(buf), "%d%%", percent);

    if (det_perclos) {
        lv_label_set_text(det_perclos, buf);

        if (percent < 30) {
            lv_obj_set_style_text_color(det_perclos, COLOR_GREEN, 0);
        } else if (percent < 60) {
            lv_obj_set_style_text_color(det_perclos, COLOR_YELLOW, 0);
        } else {
            lv_obj_set_style_text_color(det_perclos, COLOR_RED, 0);
        }
    }

    if (det_eye_state) {
        if (percent < 30) {
            lv_label_set_text(det_eye_state, "Normal");
            lv_obj_set_style_text_color(det_eye_state, COLOR_GREEN, 0);
        } else if (percent < 60) {
            lv_label_set_text(det_eye_state, "Watch");
            lv_obj_set_style_text_color(det_eye_state, COLOR_YELLOW, 0);
        } else {
            lv_label_set_text(det_eye_state, "Risk");
            lv_obj_set_style_text_color(det_eye_state, COLOR_RED, 0);
        }
    }
}

void ui_update_yaw_state(int yaw_state, float yaw_score)
{
    (void)yaw_score;

    if (!det_blink) {
        return;
    }

    /*
     * yaw_state:
     * -1 = No Driver / Unknown
     *  0 = Normal
     *  1 = Left / Right depending on camera mirror
     *  2 = Left / Right depending on camera mirror
     */
    if (yaw_state < 0) {
        lv_label_set_text(det_blink, "--");
        lv_obj_set_style_text_color(det_blink, COLOR_SUB_TEXT, 0);
        return;
    }

    /*
     * 摄像头面对驾驶员时，图像左右和驾驶员真实左右是镜像关系。
     * behavior_engine 输出的是图像坐标方向，
     * UI 这里显示成驾驶员真实方向，所以左右需要互换。
     */
    if (yaw_state == 1) {
        lv_label_set_text(det_blink, "Right");
        lv_obj_set_style_text_color(det_blink, COLOR_YELLOW, 0);
    } else if (yaw_state == 2) {
        lv_label_set_text(det_blink, "Left");
        lv_obj_set_style_text_color(det_blink, COLOR_YELLOW, 0);
    } else {
        lv_label_set_text(det_blink, "Normal");
        lv_obj_set_style_text_color(det_blink, COLOR_CYAN, 0);
    }
}

void ui_update_hands_state(int hands_off)
{
    if (!det_yawn) {
        return;
    }

    if (hands_off) {
        lv_label_set_text(det_yawn, "Off");
        lv_obj_set_style_text_color(det_yawn, COLOR_RED, 0);
    } else {
        lv_label_set_text(det_yawn, "On");
        lv_obj_set_style_text_color(det_yawn, COLOR_GREEN, 0);
    }
}

void ui_update_head_state(int detected)
{
    pill_update(home_head_pill, detected);
    pill_update(det_head_pill, detected);

    if (last_head_state != detected) {
        last_head_state = detected;

        if (detected) {
            records_add_event("Head down risk detected", COLOR_ORANGE);
        } else {
            records_add_event("Head position recovered", COLOR_GREEN);
        }
    }
}

void ui_update_env(float temp, int humi)
{
    char buf[32];

    snprintf(buf, sizeof(buf), "%.1f°C", (double)temp);

    if (env_temp_label) {
        lv_label_set_text(env_temp_label, buf);
    }

    snprintf(buf, sizeof(buf), "%d%%", humi);

    if (env_humi_label) {
        lv_label_set_text(env_humi_label, buf);
    }

    if (temp >= 18.0f && temp <= 28.0f && humi >= 40 && humi <= 70) {
        if (env_comfort_label) {
            lv_label_set_text(env_comfort_label, "Good");
            set_label_color(env_comfort_label, COLOR_GREEN);
        }

        if (home_comfort) {
            lv_label_set_text(home_comfort, "Good");
            set_label_color(home_comfort, COLOR_GREEN);
        }
    } else if (temp > 28.0f || humi > 75) {
        if (env_comfort_label) {
            lv_label_set_text(env_comfort_label, "Hot");
            set_label_color(env_comfort_label, COLOR_YELLOW);
        }

        if (home_comfort) {
            lv_label_set_text(home_comfort, "Hot");
            set_label_color(home_comfort, COLOR_YELLOW);
        }
    } else {
        if (env_comfort_label) {
            lv_label_set_text(env_comfort_label, "Poor");
            set_label_color(env_comfort_label, COLOR_ORANGE);
        }

        if (home_comfort) {
            lv_label_set_text(home_comfort, "Poor");
            set_label_color(home_comfort, COLOR_ORANGE);
        }
    }
}

void ui_update_fan_speed(int speed)
{
    speed = clamp_int(speed, 0, 100);

    char buf[16];
    snprintf(buf, sizeof(buf), "%d%%", speed);

    if (env_fan_speed) {
        lv_label_set_text(env_fan_speed, buf);
    }

    if (home_fan_speed) {
        lv_label_set_text(home_fan_speed, buf);
    }

    if (env_fan_arc) {
        lv_arc_set_value(env_fan_arc, speed);
    }

    if (!slider_fan) return;

    if (g_fan_ctrl && g_fan_ctrl->mode == 2) {
        return;
    }

    g_updating_slider = true;
    lv_slider_set_value(slider_fan, speed, LV_ANIM_OFF);
    g_updating_slider = false;
}

void ui_update_fan_mode(const char *mode_text)
{
    if (!mode_text) return;

    if (env_fan_mode) {
        lv_label_set_text(env_fan_mode, mode_text);
    }

    if (home_fan_mode) {
        lv_label_set_text(home_fan_mode, mode_text);
    }

    lv_color_t color = COLOR_GREEN;

    if (strcmp(mode_text, "Manual") == 0) {
        color = COLOR_CYAN;
    } else if (strcmp(mode_text, "Off") == 0) {
        color = COLOR_SUB_TEXT;
    }

    set_label_color(env_fan_mode, color);
    set_label_color(home_fan_mode, color);
}

void ui_update_voice_state(bool enabled)
{
    if (setting_voice_state) {
        if (enabled) {
            lv_label_set_text(setting_voice_state, "On");
            lv_obj_set_style_text_color(setting_voice_state, COLOR_GREEN, 0);
        } else {
            lv_label_set_text(setting_voice_state, "Off");
            lv_obj_set_style_text_color(setting_voice_state, COLOR_RED, 0);
        }
    }

    /*
     * Voice Broadcast 开关状态写入临时文件。
     * main.c 的语音预警逻辑会读取这个文件。
     *
     * 1 = 开启主动语音预警
     * 0 = 关闭主动语音预警
     */
    FILE *fp = fopen("/tmp/voice_broadcast_enable", "w");
    if (fp) {
        fprintf(fp, "%d\n", enabled ? 1 : 0);
        fclose(fp);
    }
}

void ui_update_time(const char *time_text, const char *date_text)
{
    if (label_time && time_text) {
        lv_label_set_text(label_time, time_text);
    }

    if (label_date && date_text) {
        lv_label_set_text(label_date, date_text);
    }
}

void ui_update_chip_temp(float chip_temp)
{
    char buf[16];
    snprintf(buf, sizeof(buf), "%.1f°C", (double)chip_temp);

    if (env_chip_temp) {
        lv_label_set_text(env_chip_temp, buf);
    }

    if (home_chip_temp) {
        lv_label_set_text(home_chip_temp, buf);
    }

    lv_color_t color = COLOR_GREEN;

    if (chip_temp >= 60.0f) {
        color = COLOR_RED;
    } else if (chip_temp >= 50.0f) {
        color = COLOR_ORANGE;
    }

    set_label_color(env_chip_temp, color);
    set_label_color(home_chip_temp, color);
}

void ui_show_alarm(const char *msg)
{
    static const char *btns[] = {"OK", ""};

    lv_obj_t *mbox = lv_msgbox_create(NULL, "Alert", msg, btns, true);
    lv_obj_center(mbox);
    lv_obj_set_style_bg_color(mbox, COLOR_PANEL, 0);
    lv_obj_set_style_text_color(mbox, COLOR_TEXT, 0);
    lv_obj_set_style_border_color(mbox, COLOR_RED, 0);
    lv_obj_set_style_border_width(mbox, 2, 0);
}

/* =========================================================
 * Shutdown
 * ========================================================= */
static void power_btn_confirm_cb(lv_event_t *e)
{
    lv_obj_t *mbox = lv_event_get_current_target(e);
    uint32_t btn = lv_msgbox_get_active_btn(mbox);

    if (btn == 1) {
        int ret;

        ret = system("sudo killall gst_producer behavior_engine env_monitor fan_control 2>/dev/null");
        (void)ret;

        ret = system("sudo sh -c \"echo 1 > /sys/class/pwm/pwmchip0/pwm0/enable\" 2>/dev/null");
        (void)ret;

        ret = system("sudo sh -c \"echo 1000000 > /sys/class/pwm/pwmchip0/pwm0/duty_cycle\" 2>/dev/null");
        (void)ret;

        exit(0);
    }

    lv_msgbox_close(mbox);
}

static void power_btn_event_cb(lv_event_t *e)
{
    (void)e;

    static const char *btns[] = {"Cancel", "Exit", ""};

    lv_obj_t *mbox = lv_msgbox_create(NULL,
                                      "Shutdown",
                                      "Stop all processes and exit?",
                                      btns,
                                      true);
    lv_obj_center(mbox);
    lv_obj_add_event_cb(mbox, power_btn_confirm_cb, LV_EVENT_VALUE_CHANGED, NULL);
}





/* =========================================================
 * Camera preview update
 * Source: SharedFrame RGB888
 * Target: LVGL native color buffer
 * ========================================================= */

static void camera_preview_init_buffer(void)
{
    if (camera_preview_buf) {
        return;
    }

    camera_preview_buf = (lv_color_t *)malloc(CAM_PREVIEW_W * CAM_PREVIEW_H * sizeof(lv_color_t));
    if (!camera_preview_buf) {
        printf("[UI] camera_preview_buf malloc failed\n");
        return;
    }

    memset(camera_preview_buf, 0, CAM_PREVIEW_W * CAM_PREVIEW_H * sizeof(lv_color_t));

    memset(&camera_img_dsc, 0, sizeof(camera_img_dsc));
    camera_img_dsc.header.always_zero = 0;
    camera_img_dsc.header.w = CAM_PREVIEW_W;
    camera_img_dsc.header.h = CAM_PREVIEW_H;
    camera_img_dsc.header.cf = LV_IMG_CF_TRUE_COLOR;
    camera_img_dsc.data_size = CAM_PREVIEW_W * CAM_PREVIEW_H * sizeof(lv_color_t);
    camera_img_dsc.data = (const uint8_t *)camera_preview_buf;

    if (camera_img) {
        lv_img_set_src(camera_img, &camera_img_dsc);
    }
}

static void camera_compute_crop_rect(int src_w, int src_h)
{
    if (src_w <= 0 || src_h <= 0) {
        src_w = CAM_WIDTH;
        src_h = CAM_HEIGHT;
    }

    float ratio = camera_view_crop_ratio[g_camera_view_mode];

    int crop_w = (int)((float)src_w * ratio);
    int crop_h = (int)((float)src_h * ratio);

    /*
     * 保持和预览框一样的 16:9 比例。
     * 你的源图 1280x720 和预览 555x312 都接近 16:9。
     */
    float preview_aspect = (float)CAM_PREVIEW_W / (float)CAM_PREVIEW_H;
    float crop_aspect = (float)crop_w / (float)crop_h;

    if (crop_aspect > preview_aspect) {
        crop_w = (int)((float)crop_h * preview_aspect);
    } else {
        crop_h = (int)((float)crop_w / preview_aspect);
    }

    if (crop_w > src_w) crop_w = src_w;
    if (crop_h > src_h) crop_h = src_h;

    int max_x = src_w - crop_w;
    int max_y = src_h - crop_h;

    if (max_x < 0) max_x = 0;
    if (max_y < 0) max_y = 0;

    int crop_x = max_x / 2;

    float y_bias = camera_view_y_bias[g_camera_view_mode];
    int crop_y = (int)((float)max_y * y_bias);

    if (crop_y < 0) crop_y = 0;
    if (crop_y > max_y) crop_y = max_y;

    g_crop_x = crop_x;
    g_crop_y = crop_y;
    g_crop_w = crop_w;
    g_crop_h = crop_h;
}

static void camera_rgb888_to_lvgl_nearest(const unsigned char *src_rgb,
                                          int src_w,
                                          int src_h)
{
    if (!src_rgb || !camera_preview_buf) {
        return;
    }

    if (src_w <= 0 || src_h <= 0) {
        return;
    }

    camera_compute_crop_rect(src_w, src_h);

    /*
     * 从裁剪区域 g_crop_x/g_crop_y/g_crop_w/g_crop_h
     * 缩放到固定预览框 CAM_PREVIEW_W x CAM_PREVIEW_H。
     */
    for (int y = 0; y < CAM_PREVIEW_H; y++) {
        int sy = g_crop_y + (y * g_crop_h) / CAM_PREVIEW_H;

        if (sy < 0) sy = 0;
        if (sy >= src_h) sy = src_h - 1;

        for (int x = 0; x < CAM_PREVIEW_W; x++) {
            int sx = g_crop_x + (x * g_crop_w) / CAM_PREVIEW_W;

            if (sx < 0) sx = 0;
            if (sx >= src_w) sx = src_w - 1;

            int src_idx = (sy * src_w + sx) * 3;

            unsigned char r = src_rgb[src_idx + 0];
            unsigned char g = src_rgb[src_idx + 1];
            unsigned char b = src_rgb[src_idx + 2];

            camera_preview_buf[y * CAM_PREVIEW_W + x] = lv_color_make(r, g, b);
        }
    }
}

void ui_update_camera_preview(void)
{
    if (!g_camera_frame || !camera_img) {
        return;
    }

    /*
     * 防止共享内存还没写入有效数据。
     */
    if (g_camera_frame->width <= 0 ||
        g_camera_frame->height <= 0 ||
        g_camera_frame->width > CAM_WIDTH ||
        g_camera_frame->height > CAM_HEIGHT) {
        return;
    }

    /*
     * 没有新帧就不刷新，降低 UI 重绘压力。
     */
    if (g_camera_frame->frame_count == last_camera_frame_count) {
        return;
    }

    last_camera_frame_count = g_camera_frame->frame_count;

    camera_preview_init_buffer();
    if (!camera_preview_buf) {
        return;
    }

    camera_rgb888_to_lvgl_nearest(g_camera_frame->rgb_data,
                                  g_camera_frame->width,
                                  g_camera_frame->height);

    if (camera_placeholder_1) {
        lv_obj_add_flag(camera_placeholder_1, LV_OBJ_FLAG_HIDDEN);
    }

    if (camera_placeholder_2) {
        lv_obj_add_flag(camera_placeholder_2, LV_OBJ_FLAG_HIDDEN);
    }

    /*
     * 让 LVGL 重新绘制这张图片。
     */
    lv_img_cache_invalidate_src(&camera_img_dsc);
    lv_obj_invalidate(camera_img);

    /* 统计 UI 实际显示帧数，用于 Preview FPS */
    camera_preview_frame_count++;

    ui_update_detection_overlay();

}

static void nav_child_text_color(lv_obj_t *obj, lv_color_t color)
{
    if (!obj) return;

    uint32_t cnt = lv_obj_get_child_cnt(obj);

    for (uint32_t i = 0; i < cnt; i++) {
        lv_obj_t *child = lv_obj_get_child(obj, i);

        if (child) {
            lv_obj_set_style_text_color(child, color, 0);
            nav_child_text_color(child, color);
        }
    }
}

static void nav_set_active_manual(int active_page)
{
    for (int i = 0; i < PAGE_COUNT; i++) {
        if (!nav_btns[i]) continue;

        /*
         * 清掉 LVGL 默认状态，避免红色 checked/focused 残留。
         */
        lv_obj_clear_state(nav_btns[i], LV_STATE_CHECKED);
        lv_obj_clear_state(nav_btns[i], LV_STATE_FOCUSED);
        lv_obj_clear_state(nav_btns[i], LV_STATE_PRESSED);

        if (i == active_page) {
            /*
             * 当前页面：蓝色高亮按钮。
             * 这里用你的系统蓝色/青色风格。
             */
            lv_obj_set_style_bg_color(nav_btns[i], lv_color_hex(0x0B7CFF), 0);
            lv_obj_set_style_bg_opa(nav_btns[i], LV_OPA_70, 0);

            lv_obj_set_style_border_width(nav_btns[i], 2, 0);
            lv_obj_set_style_border_color(nav_btns[i], COLOR_CYAN, 0);
            lv_obj_set_style_border_opa(nav_btns[i], LV_OPA_90, 0);

            lv_obj_set_style_shadow_width(nav_btns[i], 18, 0);
            lv_obj_set_style_shadow_color(nav_btns[i], COLOR_CYAN, 0);
            lv_obj_set_style_shadow_opa(nav_btns[i], LV_OPA_30, 0);

            nav_child_text_color(nav_btns[i], COLOR_CYAN);
        } else {
            /*
             * 非当前页面：恢复普通深色按钮。
             */
            lv_obj_set_style_bg_color(nav_btns[i], lv_color_hex(0x071A2F), 0);
            lv_obj_set_style_bg_opa(nav_btns[i], LV_OPA_30, 0);

            lv_obj_set_style_border_width(nav_btns[i], 1, 0);
            lv_obj_set_style_border_color(nav_btns[i], COLOR_BORDER_DIM, 0);
            lv_obj_set_style_border_opa(nav_btns[i], LV_OPA_60, 0);

            lv_obj_set_style_shadow_width(nav_btns[i], 0, 0);
            lv_obj_set_style_shadow_opa(nav_btns[i], LV_OPA_TRANSP, 0);

            nav_child_text_color(nav_btns[i], COLOR_CYAN);
        }
    }
}

static void ui_show_page_by_id(int page_id)
{
    if (page_id < 0 || page_id >= PAGE_COUNT) {
        return;
    }

    /*
     * 1. 切换页面显示
     */
    for (int i = 0; i < PAGE_COUNT; i++) {
        if (pages[i]) {
            lv_obj_add_flag(pages[i], LV_OBJ_FLAG_HIDDEN);
        }
    }

    if (pages[page_id]) {
        lv_obj_clear_flag(pages[page_id], LV_OBJ_FLAG_HIDDEN);
    }

    /*
     * 2. 手动同步左侧导航蓝色高亮
     * 不使用 LV_STATE_CHECKED，避免红色按钮出现。
     */
    nav_set_active_manual(page_id);
}

void ui_handle_voice_command(const char *cmd)
{
    if (!cmd) return;

    /*
     * Page control
     */
    if (strcmp(cmd, "HOME") == 0) {
        ui_show_page_by_id(PAGE_HOME);
        return;
    } else if (strcmp(cmd, "DETECT") == 0) {
        ui_show_page_by_id(PAGE_DETECTION);
        return;
    } else if (strcmp(cmd, "ENV") == 0) {
        ui_show_page_by_id(PAGE_ENVIRONMENT);
        return;
    } else if (strcmp(cmd, "RECORD") == 0) {
        ui_show_page_by_id(PAGE_RECORDS);
        return;
    } else if (strcmp(cmd, "SETTING") == 0) {
        ui_show_page_by_id(PAGE_SETTINGS);
        return;
    }

    /*
     * Fan control
     *
     * FanCtrl mode:
     * 0 = Auto
     * 2 = Manual
     * 3 = Off
     */
    if (strcmp(cmd, "FAN_AUTO") == 0) {
        if (g_fan_ctrl) {
            g_fan_ctrl->mode = 0;
            g_fan_ctrl->timestamp = (uint64_t)time(NULL);
        }

        ui_update_fan_mode("Auto");

        if (slider_fan) {
            g_updating_slider = true;
            lv_slider_set_value(slider_fan, 0, LV_ANIM_OFF);
            g_updating_slider = false;

            lv_obj_clear_flag(slider_fan, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_add_state(slider_fan, LV_STATE_DISABLED);
        }

        return;
    }

    if (strcmp(cmd, "FAN_ON") == 0) {
        if (g_fan_ctrl) {
            g_fan_ctrl->mode = 2;
            g_fan_ctrl->manual_speed = 70;
            g_fan_ctrl->timestamp = (uint64_t)time(NULL);
        }

        ui_update_fan_mode("Manual");
        ui_update_fan_speed(70);

        if (slider_fan) {
            g_updating_slider = true;
            lv_slider_set_value(slider_fan, 70, LV_ANIM_OFF);
            g_updating_slider = false;

            lv_obj_add_flag(slider_fan, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_clear_state(slider_fan, LV_STATE_DISABLED);
        }

        return;
    }

    if (strcmp(cmd, "FAN_MAX") == 0) {
        if (g_fan_ctrl) {
            g_fan_ctrl->mode = 2;
            g_fan_ctrl->manual_speed = 100;
            g_fan_ctrl->timestamp = (uint64_t)time(NULL);
        }

        ui_update_fan_mode("Manual");
        ui_update_fan_speed(100);

        if (slider_fan) {
            g_updating_slider = true;
            lv_slider_set_value(slider_fan, 100, LV_ANIM_OFF);
            g_updating_slider = false;

            lv_obj_add_flag(slider_fan, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_clear_state(slider_fan, LV_STATE_DISABLED);
        }

        return;
    }

    if (strcmp(cmd, "FAN_OFF") == 0) {
        if (g_fan_ctrl) {
            g_fan_ctrl->mode = 3;
            g_fan_ctrl->manual_speed = 0;
            g_fan_ctrl->timestamp = (uint64_t)time(NULL);
        }

        ui_update_fan_mode("Off");
        ui_update_fan_speed(0);

        if (slider_fan) {
            g_updating_slider = true;
            lv_slider_set_value(slider_fan, 0, LV_ANIM_OFF);
            g_updating_slider = false;

            lv_obj_clear_flag(slider_fan, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_add_state(slider_fan, LV_STATE_DISABLED);
        }

        return;
    }
}