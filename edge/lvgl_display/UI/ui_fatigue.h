#ifndef UI_FATIGUE_H
#define UI_FATIGUE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "lvgl/lvgl.h"
#include "shared_mem.h"
LV_FONT_DECLARE(lv_font_chinese_18);
//LV_FONT_DECLARE(lv_font_simsun_16_cjk);

#define SCREEN_W 1024
#define SCREEN_H 600

void ui_fatigue_create(void);

void ui_update_fatigue_score(int score);
void ui_update_state(const char *state_text, lv_color_t color);

void ui_update_eye_state(const char *eye_state);
void ui_update_perclos(int perclos);
void ui_update_blink_freq(int blink_freq);
void ui_update_yawn_count(int yawn_count);

void ui_update_phone_state(int detected);
void ui_update_smoking_state(int detected);
void ui_update_head_state(int detected);
void ui_update_wheel_state(int hands_off);

void ui_update_env(float temp, int humi);
void ui_update_fan_speed(int speed);
void ui_update_fan_mode(const char *mode_text);

void ui_update_voice_state(bool enabled);
void ui_update_time(const char *time_text, const char *date_text);

void ui_show_alarm(const char *msg);

void ui_set_fan_ctrl(FanCtrl *ctrl);
/* 摄像头实时预览 */
void ui_set_camera_frame(SharedFrame *frame);
void ui_set_behavior_result(BehaviorResult *result);

void ui_update_camera_preview(void);
void ui_update_detection_overlay(void);
void ui_update_chip_temp(float chip_temp);

/* 兼容旧代码。当前电源入口放在 Settings 页面 */
void ui_add_power_button(void);

void ui_update_eye_risk(float eye_risk);
void ui_update_yaw_state(int yaw_state, float yaw_score);
void ui_update_hands_state(int hands_off);

//语音
void ui_handle_voice_command(const char *cmd);
void ui_update_vehicle_state(int moving);


#ifdef __cplusplus
}
#endif

#endif