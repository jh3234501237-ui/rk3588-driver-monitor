#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/shm.h>
#include <time.h>
#include <stdint.h>
#include <string.h>

#include "lvgl.h"
#include "lv_drivers/display/fbdev.h"
#include "lv_drivers/indev/evdev.h"

#include "ui_fatigue.h"
#include "shared_mem.h"
#include "driver_state_engine_simple.h"

static volatile int running = 1;

static EnvData *env_data = NULL;
static BehaviorResult *behav_res = NULL;
static FanCtrl *fan_ctrl = NULL;
static SharedFrame *shared_frame = NULL;

static DriverStateEngineSimple g_dse;

void sigint_handler(int sig)
{
    (void)sig;
    running = 0;
}

static uint64_t get_now_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (uint64_t)ts.tv_sec * 1000ULL + (uint64_t)ts.tv_nsec / 1000000ULL;
}

static void trim_cmd(char *s)
{
    if (!s) return;

    int len = strlen(s);
    while (len > 0 &&
           (s[len - 1] == '\n' || s[len - 1] == '\r' ||
            s[len - 1] == ' '  || s[len - 1] == '\t')) {
        s[len - 1] = '\0';
        len--;
    }
}

static void process_voice_command_file(void)
{
    FILE *fp = fopen("/tmp/voice_cmd", "r");
    if (!fp) {
        return;
    }

    char cmd[64] = {0};

    if (fgets(cmd, sizeof(cmd), fp) == NULL) {
        fclose(fp);
        unlink("/tmp/voice_cmd");
        return;
    }

    fclose(fp);
    unlink("/tmp/voice_cmd");

    trim_cmd(cmd);

    if (cmd[0] == '\0') {
        return;
    }

    printf("[VOICE_UI] command: %s\n", cmd);
    ui_handle_voice_command(cmd);
}

#define VOICE_ALERT_FILE "/tmp/voice_alert"
#define EVENT_LOG_FILE "/home/elf/rknn_model_zoo/rknn_model_zoo-2.1.0/examples/yolov8_pose/cpp/logs/events.csv"

#define ALERT_HEAD_HOLD_MS     2000
#define ALERT_PHONE_HOLD_MS    2500
#define ALERT_MOUTH_HOLD_MS    2500
#define ALERT_WHEEL_HOLD_MS    3000
#define ALERT_DANGER_HOLD_MS   2500

#define ALERT_COOLDOWN_MS      15000

static int voice_broadcast_enabled(void)
{
    FILE *fp = fopen("/tmp/voice_broadcast_enable", "r");
    if (!fp) {
        /*
         * 文件不存在时默认开启。
         */
        return 1;
    }

    int enabled = 1;
    fscanf(fp, "%d", &enabled);
    fclose(fp);

    return enabled ? 1 : 0;
}

static int read_motion_sensor_moving(void)
{
    FILE *fp = fopen("/tmp/vehicle_state", "r");
    if (!fp) {
        /*
         * 没有传感器状态文件时，默认认为正在行驶。
         * 这样 motion_monitor 异常时不会影响原有演示。
         */
        return 1;
    }

    char line[128];
    int moving = 1;

    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "moving=%d", &moving) == 1) {
            break;
        }
    }

    fclose(fp);

    return moving ? 1 : 0;
}

static int vehicle_is_moving(void)
{
    FILE *fp = fopen("/tmp/vehicle_mode", "r");

    if (fp) {
        char mode[32] = {0};

        if (fgets(mode, sizeof(mode), fp)) {
            /*
             * 去掉换行。
             */
            mode[strcspn(mode, "\r\n")] = '\0';

            fclose(fp);

            if (strcmp(mode, "DRIVING") == 0) {
                return 1;
            }

            if (strcmp(mode, "PARKED") == 0) {
                return 0;
            }

            /*
             * AUTO 或其他内容，走 MPU6050 自动判断。
             */
            return read_motion_sensor_moving();
        }

        fclose(fp);
    }

    /*
     * 没有 mode 文件时，默认 AUTO。
     */
    return read_motion_sensor_moving();
}

static const char *event_name_from_alert(const char *alert)
{
    if (!alert) return "UNKNOWN";

    if (strcmp(alert, "ALERT_HEAD") == 0) {
        return "HEAD_DOWN";
    } else if (strcmp(alert, "ALERT_PHONE") == 0) {
        return "PHONE_CALL";
    } else if (strcmp(alert, "ALERT_MOUTH") == 0) {
        return "MOUTH_RISK";
    } else if (strcmp(alert, "ALERT_WHEEL") == 0) {
        return "WHEEL_OFF";
    } else if (strcmp(alert, "ALERT_DANGER") == 0) {
        return "DANGEROUS";
    }

    return "UNKNOWN";
}

static void get_event_time_string(char *buf, size_t size)
{
    if (!buf || size == 0) {
        return;
    }

    time_t now = time(NULL);
    struct tm tm_now;

    localtime_r(&now, &tm_now);
    strftime(buf, size, "%Y-%m-%d %H:%M:%S", &tm_now);
}

static void read_vehicle_mode_string(char *buf, size_t size)
{
    if (!buf || size == 0) {
        return;
    }

    snprintf(buf, size, "AUTO");

    FILE *fp = fopen("/tmp/vehicle_mode", "r");
    if (!fp) {
        return;
    }

    if (fgets(buf, size, fp)) {
        buf[strcspn(buf, "\r\n")] = '\0';
    }

    fclose(fp);

    if (buf[0] == '\0') {
        snprintf(buf, size, "AUTO");
    }
}

static int read_vehicle_moving_for_log(void)
{
    FILE *fp_mode = fopen("/tmp/vehicle_mode", "r");
    if (fp_mode) {
        char mode[32] = {0};

        if (fgets(mode, sizeof(mode), fp_mode)) {
            mode[strcspn(mode, "\r\n")] = '\0';
            fclose(fp_mode);

            if (strcmp(mode, "DRIVING") == 0) {
                return 1;
            }

            if (strcmp(mode, "PARKED") == 0) {
                return 0;
            }
        } else {
            fclose(fp_mode);
        }
    }

    FILE *fp = fopen("/tmp/vehicle_state", "r");
    if (!fp) {
        return 1;
    }

    char line[128];
    int moving = 1;

    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "moving=%d", &moving) == 1) {
            break;
        }
    }

    fclose(fp);
    return moving ? 1 : 0;
}

static void append_event_csv(const char *alert,
                             const BehaviorResult *res,
                             int vehicle_moving,
                             int voice_enabled)
{
    if (!alert) {
        return;
    }

    FILE *fp = fopen(EVENT_LOG_FILE, "a");
    if (!fp) {
        return;
    }

    fseek(fp, 0, SEEK_END);
    long pos = ftell(fp);

    if (pos == 0) {
        fprintf(fp,
                "time,event,score,head_down,phone_call,mouth_risk,hands_off,vehicle_mode,vehicle_moving,voice_broadcast\n");
    }

    char time_buf[32];
    char vehicle_mode[32];

    get_event_time_string(time_buf, sizeof(time_buf));
    read_vehicle_mode_string(vehicle_mode, sizeof(vehicle_mode));

    int score = 0;
    int head_down = 0;
    int phone_call = 0;
    int mouth_risk = 0;
    int hands_off = 0;

    if (res) {
        score = res->fatigue_score;
        head_down = res->head_down;
        phone_call = res->phone_call;
        mouth_risk = res->mouth_risk;
        hands_off = res->hands_off;
    }

    fprintf(fp,
            "%s,%s,%d,%d,%d,%d,%d,%s,%d,%d\n",
            time_buf,
            event_name_from_alert(alert),
            score,
            head_down,
            phone_call,
            mouth_risk,
            hands_off,
            vehicle_mode,
            vehicle_moving ? 1 : 0,
            voice_enabled ? 1 : 0);

    fclose(fp);
}

static void write_voice_alert_with_record(const char *alert,
                                          const BehaviorResult *res)
{
    if (!alert) {
        return;
    }

    int voice_enabled = voice_broadcast_enabled();
    int vehicle_moving = read_vehicle_moving_for_log();

    /*
     * 记录触发预警逻辑的事件。
     * Voice Broadcast 关闭时，也记录，但不播报。
     */
    append_event_csv(alert, res, vehicle_moving, voice_enabled);

    /*
     * Voice Broadcast 关闭：只记录，不播报。
     */
    if (!voice_enabled) {
        return;
    }

    FILE *fp = fopen(VOICE_ALERT_FILE, "w");
    if (!fp) {
        return;
    }

    fprintf(fp, "%s\n", alert);
    fclose(fp);
}

static uint64_t update_hold_start(int active, uint64_t now_ms, uint64_t old_start)
{
    if (active) {
        if (old_start == 0) {
            return now_ms;
        }
        return old_start;
    }

    return 0;
}

static int hold_reached(uint64_t start_ms, uint64_t now_ms, uint64_t hold_ms)
{
    return (start_ms > 0 && now_ms >= start_ms && (now_ms - start_ms) >= hold_ms);
}

static int cooldown_passed(uint64_t last_ms, uint64_t now_ms)
{
    return (last_ms == 0 || now_ms < last_ms || (now_ms - last_ms) >= ALERT_COOLDOWN_MS);
}

static void process_behavior_voice_alerts(const BehaviorResult *res,
                                          int driver_present,
                                          uint64_t now_ms)
{
    static uint64_t head_start_ms = 0;
    static uint64_t phone_start_ms = 0;
    static uint64_t mouth_start_ms = 0;
    static uint64_t wheel_start_ms = 0;
    static uint64_t danger_start_ms = 0;

    /*
     * 全局播报冷却：
     * 任意一种播报触发后，冷却时间内不再播报其他事件。
     */
    static uint64_t last_any_alert_ms = 0;

    if (!res || !driver_present) {
        head_start_ms = 0;
        phone_start_ms = 0;
        mouth_start_ms = 0;
        wheel_start_ms = 0;
        danger_start_ms = 0;
        return;
    }

    /*
    * 车辆静止 / 停车休息状态：
    * 检测结果继续在 UI 显示，但不主动语音预警。
    *
    * 这样可以避免停车休息时低头、打电话、离盘被误当成驾驶违规。
    */
    if (!vehicle_is_moving()) {
        head_start_ms = 0;
        phone_start_ms = 0;
        mouth_start_ms = 0;
        wheel_start_ms = 0;
        danger_start_ms = 0;
        return;
    }

    int head_active   = res->head_down ? 1 : 0;
    int phone_active  = res->phone_call ? 1 : 0;
    int mouth_active  = res->mouth_risk ? 1 : 0;
    int wheel_active  = res->hands_off ? 1 : 0;
    int danger_active = (res->fatigue_score >= 85) ? 1 : 0;

    /*
     * 如果已经存在更明确的危险行为，
     * 就不单独播报双手离盘。
     *
     * 例如：
     * 打电话时手离方向盘，是打电话动作的伴随现象；
     * 抽烟/嘴部风险时手也可能离盘；
     * 低头时也可能导致方向盘状态异常。
     *
     * 这样可以避免连续播报：
     * “请勿接打电话” + “请握好方向盘”
     */
    if (head_active || phone_active || mouth_active) {
        wheel_active = 0;
    }

    head_start_ms   = update_hold_start(head_active, now_ms, head_start_ms);
    phone_start_ms  = update_hold_start(phone_active, now_ms, phone_start_ms);
    mouth_start_ms  = update_hold_start(mouth_active, now_ms, mouth_start_ms);
    wheel_start_ms  = update_hold_start(wheel_active, now_ms, wheel_start_ms);
    danger_start_ms = update_hold_start(danger_active, now_ms, danger_start_ms);

    /*
     * 总冷却时间没过，不播报任何语音。
     */
    if (!cooldown_passed(last_any_alert_ms, now_ms)) {
        return;
    }

    /*
     * 播报优先级：
     * 低头 > 打电话 > 嘴部风险 > 双手离盘 > 综合危险
     */
    if (hold_reached(head_start_ms, now_ms, ALERT_HEAD_HOLD_MS) &&
    cooldown_passed(last_any_alert_ms, now_ms)) {
    write_voice_alert_with_record("ALERT_HEAD", res);
    last_any_alert_ms = now_ms;
    return;
    }

    if (hold_reached(phone_start_ms, now_ms, ALERT_PHONE_HOLD_MS) &&
        cooldown_passed(last_any_alert_ms, now_ms)) {
        write_voice_alert_with_record("ALERT_PHONE", res);
        last_any_alert_ms = now_ms;
        return;
    }

    if (hold_reached(mouth_start_ms, now_ms, ALERT_MOUTH_HOLD_MS) &&
        cooldown_passed(last_any_alert_ms, now_ms)) {
        write_voice_alert_with_record("ALERT_MOUTH", res);
        last_any_alert_ms = now_ms;
        return;
    }

    if (hold_reached(wheel_start_ms, now_ms, ALERT_WHEEL_HOLD_MS) &&
        cooldown_passed(last_any_alert_ms, now_ms)) {
        write_voice_alert_with_record("ALERT_WHEEL", res);
        last_any_alert_ms = now_ms;
        return;
    }

    if (hold_reached(danger_start_ms, now_ms, ALERT_DANGER_HOLD_MS) &&
        cooldown_passed(last_any_alert_ms, now_ms)) {
        write_voice_alert_with_record("ALERT_DANGER", res);
        last_any_alert_ms = now_ms;
        return;
    }
}

static void update_ui_timer(lv_timer_t *timer)
{
    (void)timer;

    /*
     * 1. 环境数据
     */
    if (env_data) {
        ui_update_env(env_data->temperature, (int)env_data->humidity);
    }

    int vehicle_moving = vehicle_is_moving();
    ui_update_vehicle_state(vehicle_moving);

    /*
    * 2. 驾驶员状态引擎
    */
    if (behav_res) {
        DseOutput out;
        uint64_t now_ms = get_now_ms();

        dse_simple_update(&g_dse, behav_res, now_ms, &out);

        /*
        * 判断是否检测到驾驶员。
        * 只要有人脸或姿态，就认为当前有人。
        */
        int driver_present = (behav_res->pose_detected || behav_res->face_detected);

        if (driver_present) {
            /*
            * 有驾驶员：正常更新所有检测状态。
            */

            /*
            * 疲劳分数：现在优先使用 behavior_engine 输出的综合风险分数。
            */
            ui_update_fatigue_score(behav_res->fatigue_score);

            /*
            * 原来的 PERCLOS / Blink / Yawn 三个位置现在复用为：
            * EyeRisk / Yaw / Hands
            */
            ui_update_eye_risk(behav_res->eye_risk_score);
            ui_update_yaw_state(behav_res->head_yaw_state, behav_res->head_yaw_score);
            ui_update_hands_state(behav_res->hands_off);

            /*
            * Vision / Behavior 状态：
            * smoking 字段现在兼容表示 mouth_risk。
            * 推荐使用 mouth_risk。
            */
            ui_update_phone_state(behav_res->phone_call);
            ui_update_smoking_state(behav_res->mouth_risk);
            ui_update_head_state(behav_res->head_down);
            ui_update_wheel_state(behav_res->hands_off);

            process_behavior_voice_alerts(behav_res, driver_present, now_ms);
        } else {
            /*
            * 没有检测到驾驶员：
            * 不应该显示 Normal + Wheel On。
            * 显示 NO DRIVER，Wheel 显示 --。
            */

            ui_update_fatigue_score(0);

            /*
            * 注意顺序：
            * ui_update_fatigue_score(0) 可能会把状态改成 NORMAL，
            * 所以 NO DRIVER 必须放在它后面覆盖。
            */
            ui_update_state("NO DRIVER", lv_color_hex(0x8FA6C8));

            ui_update_eye_risk(-1.0f);
            ui_update_yaw_state(-1, 0.0f);

            /*
            * 这里可以保留，后面的 ui_update_wheel_state(-1)
            * 会把 Wheel 从 On 覆盖成 --。
            */
            //ui_update_hands_state(0);

            ui_update_phone_state(-1);
            ui_update_smoking_state(-1);
            ui_update_head_state(-1);
            ui_update_wheel_state(-1);

            process_behavior_voice_alerts(behav_res, driver_present, now_ms);
        }
    } else {
        /*
        * 没有行为检测共享内存数据。
        */
        ui_update_fatigue_score(0);
        ui_update_state("NO DRIVER", lv_color_hex(0x8FA6C8));

        ui_update_eye_risk(-1.0f);
        ui_update_yaw_state(-1, 0.0f);
        //ui_update_hands_state(0);

        ui_update_phone_state(-1);
        ui_update_smoking_state(-1);
        ui_update_head_state(-1);
        ui_update_wheel_state(-1);
    }

    /*
     * 3. 风扇状态与芯片温度
     */
    if (fan_ctrl) {
        if (fan_ctrl->mode == 0) {
            ui_update_fan_mode("Auto");
        } else if (fan_ctrl->mode == 2) {
            ui_update_fan_mode("Manual");
        } else if (fan_ctrl->mode == 3) {
            ui_update_fan_mode("Off");
        } else {
            ui_update_fan_mode("Unknown");
        }

        ui_update_fan_speed(fan_ctrl->current_speed);
        ui_update_chip_temp(fan_ctrl->chip_temp);
    }

    /*
     * 4. 时间日期
     */
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);

    if (tm_info) {
        char time_buf[16];
        char date_buf[32];

        strftime(time_buf, sizeof(time_buf), "%H:%M:%S", tm_info);
        strftime(date_buf, sizeof(date_buf), "%Y-%m-%d %A", tm_info);

        ui_update_time(time_buf, date_buf);
    }
    ui_update_detection_overlay();
    process_voice_command_file();
}

static void update_camera_timer(lv_timer_t *timer)
{
    (void)timer;
    ui_update_camera_preview();
}

uint32_t custom_tick_get(void)
{
    return (uint32_t)get_now_ms();
}

static void connect_shared_memory(void)
{
    int shm_cam = shmget(SHM_KEY, sizeof(SharedFrame), 0666);
    if (shm_cam >= 0) {
        shared_frame = (SharedFrame*)shmat(shm_cam, NULL, 0);
        if (shared_frame == (void*)-1) {
            shared_frame = NULL;
            perror("shmat camera");
        } else {
            printf("[UI] Camera shared memory connected\n");
        }
    } else {
        perror("shmget camera");
    }

    int shm_env = shmget(ENV_SHM_KEY, sizeof(EnvData), 0666);
    if (shm_env >= 0) {
        env_data = (EnvData*)shmat(shm_env, NULL, 0);
        if (env_data == (void*)-1) {
            env_data = NULL;
            perror("shmat env");
        } else {
            printf("[UI] Env shared memory connected\n");
        }
    } else {
        perror("shmget env");
    }

    int shm_res = shmget(RESULT_SHM_KEY, sizeof(BehaviorResult), 0666);
    if (shm_res >= 0) {
        behav_res = (BehaviorResult*)shmat(shm_res, NULL, 0);
        if (behav_res == (void*)-1) {
            behav_res = NULL;
            perror("shmat result");
        } else {
            printf("[UI] Result shared memory connected\n");
        }
    } else {
        perror("shmget result");
    }

    int shm_fan = shmget(FAN_CTRL_SHM_KEY, sizeof(FanCtrl), 0666);
    if (shm_fan >= 0) {
        fan_ctrl = (FanCtrl*)shmat(shm_fan, NULL, 0);
        if (fan_ctrl == (void*)-1) {
            fan_ctrl = NULL;
            perror("shmat fan");
        } else {
            printf("[UI] Fan shared memory connected\n");
        }
    } else {
        perror("shmget fan");
    }
}

static void detach_shared_memory(void)
{
    if (shared_frame) shmdt(shared_frame);
    if (env_data) shmdt(env_data);
    if (behav_res) shmdt(behav_res);
    if (fan_ctrl) shmdt(fan_ctrl);

    shared_frame = NULL;
    env_data = NULL;
    behav_res = NULL;
    fan_ctrl = NULL;
}

int main(void)
{
    signal(SIGINT, sigint_handler);

    dse_simple_init(&g_dse);

    connect_shared_memory();


    ui_set_fan_ctrl(fan_ctrl);

    lv_init();

    fbdev_init();
    evdev_init();

    static lv_color_t buf[1024 * 10];
    static lv_disp_draw_buf_t disp_buf;

    lv_disp_draw_buf_init(&disp_buf, buf, NULL, 1024 * 10);

    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);

    disp_drv.draw_buf = &disp_buf;
    disp_drv.flush_cb = fbdev_flush;
    disp_drv.hor_res = 1024;
    disp_drv.ver_res = 600;

    lv_disp_drv_register(&disp_drv);

    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);

    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = evdev_read;

    lv_indev_drv_register(&indev_drv);

ui_fatigue_create();

ui_set_fan_ctrl(fan_ctrl);
ui_set_camera_frame(shared_frame);
ui_set_behavior_result(behav_res);

/*
 * 数据刷新：500ms
 * 摄像头刷新：100ms，大约 10FPS
 */
lv_timer_create(update_ui_timer, 500, NULL);
lv_timer_create(update_camera_timer, 100, NULL);

    while (running) {
        lv_timer_handler();
        usleep(5000);
    }

    detach_shared_memory();

    return 0;
}