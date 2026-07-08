// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>
// #include <signal.h>
// #include <fcntl.h>
// #include <sys/shm.h>
// #include <math.h>
// #include <time.h>
// #include "shared_mem.h"

// // ========== 温控参数 ==========
// #define TEMP_MIN_REAL 35.0f    // 真实模式最低温度 → 最低转速
// #define TEMP_MAX_REAL 65.0f    // 真实模式最高温度 → 最高转速
// #define TEMP_MIN_SIM  30.0f    // 模拟模式最低温度
// #define TEMP_MAX_SIM  80.0f    // 模拟模式最高温度

// #define DUTY_CYCLE_MIN 200000  // 最高转速时的 duty_cycle（高温）
// #define DUTY_CYCLE_MAX 500000  // 最低转速时的 duty_cycle（低温），保证稳定转动

// // ========== 平滑与自适应参数 ==========
// #define BASE_ALPHA      0.05f  // 温度变化缓慢时的滤波系数
// #define MAX_ALPHA       0.30f  // 温度变化剧烈时的最大滤波系数
// #define RATE_SENS       0.5f   // 速率转换为 α 增量的灵敏度
// #define DUTY_DEADBAND   10000  // 输出死区（ns），变化小于 1% 周期时不更新

// // ========== 硬件路径 ==========
// #define PWM_CHIP    "/sys/class/pwm/pwmchip0"
// #define PWM_CHAN    "pwm0"
// #define PERIOD_NS   1000000
// #define TEMP_PATH   "/sys/class/thermal/thermal_zone0/temp"

// static volatile int running = 1;
// void sigint_handler(int sig) { running = 0; }

// // 写入 sysfs 文件
// int write_sysfs(const char *path, const char *value) {
//     int fd = open(path, O_WRONLY);
//     if (fd < 0) return -1;
//     ssize_t len = write(fd, value, strlen(value));
//     close(fd);
//     return (len == (ssize_t)strlen(value)) ? 0 : -1;
// }

// // 设置 PWM duty_cycle (单位: ns)
// void set_pwm_duty_cycle(long long duty_ns) {
//     if (duty_ns < 0) duty_ns = 0;
//     if (duty_ns > PERIOD_NS) duty_ns = PERIOD_NS;
//     char buf[32], path[128];
//     snprintf(buf, sizeof(buf), "%lld", duty_ns);
//     snprintf(path, sizeof(path), "%s/%s/duty_cycle", PWM_CHIP, PWM_CHAN);
//     if (write_sysfs(path, buf) != 0) {
//         fprintf(stderr, "ERROR: Failed to write duty_cycle\n");
//     }
// }

// // 读取原始芯片温度 (摄氏度)
// float get_cpu_temp_raw(void) {
//     FILE *fp = fopen(TEMP_PATH, "r");
//     if (!fp) return 45.0f;
//     int val;
//     fscanf(fp, "%d", &val);
//     fclose(fp);
//     return val / 1000.0f;
// }

// // 线性映射温度到 duty_cycle (温度越低 duty_cycle 越大，转速越慢)
// long long map_temp_to_duty_cycle(float temp, float t_min, float t_max) {
//     if (temp <= t_min) return DUTY_CYCLE_MAX;
//     if (temp >= t_max) return DUTY_CYCLE_MIN;
//     float ratio = (temp - t_min) / (t_max - t_min);
//     return (long long)(DUTY_CYCLE_MAX - ratio * (DUTY_CYCLE_MAX - DUTY_CYCLE_MIN));
// }

// int main(void) {
//     signal(SIGINT, sigint_handler);

//     // 连接/创建风扇控制共享内存
//     int shm_id = shmget(FAN_CTRL_SHM_KEY, sizeof(FanCtrl), IPC_CREAT | 0666);
//     if (shm_id < 0) { perror("shmget fan_ctrl"); return -1; }
//     FanCtrl *fan_ctrl = (FanCtrl*)shmat(shm_id, NULL, 0);
//     if (fan_ctrl == (void*)-1) { perror("shmat fan_ctrl"); return -1; }
//     memset(fan_ctrl, 0, sizeof(FanCtrl));
//     fan_ctrl->mode = 0;          // 默认真实模式
//     fan_ctrl->sim_temp = 45.0f;  // 默认模拟温度

//     // 初始化 PWM
//     write_sysfs(PWM_CHIP "/export", "0");
//     usleep(100000);
//     write_sysfs(PWM_CHIP "/" PWM_CHAN "/period", "1000000");
//     set_pwm_duty_cycle(DUTY_CYCLE_MAX);
//     write_sysfs(PWM_CHIP "/" PWM_CHAN "/enable", "1");

//     printf("Adaptive Smooth Fan Control Started\n");
//     printf("Real mode: %.0f°C~%.0f°C -> duty_cycle %lld~%lld ns\n",
//            TEMP_MIN_REAL, TEMP_MAX_REAL, DUTY_CYCLE_MAX, DUTY_CYCLE_MIN);
//     printf("Sim mode:  %.0f°C~%.0f°C -> duty_cycle %lld~%lld ns\n",
//            TEMP_MIN_SIM, TEMP_MAX_SIM, DUTY_CYCLE_MAX, DUTY_CYCLE_MIN);
//     printf("Adaptive alpha: base=%.2f, max=%.2f, deadband=%lld ns\n",
//            BASE_ALPHA, MAX_ALPHA, DUTY_DEADBAND);

//     // 滤波状态变量
//     float last_raw_temp = get_cpu_temp_raw();
//     float last_smooth_temp = last_raw_temp;
//     long long last_duty_ns = DUTY_CYCLE_MAX;

//     int print_cnt = 0;
//     while (running) {
//         // 1. 获取原始温度
//         float raw_temp;
//         if (fan_ctrl->mode == 0) {
//             raw_temp = get_cpu_temp_raw();
//         } else {
//             raw_temp = fan_ctrl->sim_temp;
//             if (raw_temp < TEMP_MIN_SIM) raw_temp = TEMP_MIN_SIM;
//             if (raw_temp > TEMP_MAX_SIM) raw_temp = TEMP_MAX_SIM;
//         }

//         // 2. 计算温度变化率 (°C/s)，假设循环周期约 2 秒
//         float delta_temp = raw_temp - last_raw_temp;
//         float abs_rate = fabsf(delta_temp) / 2.0f;
//         last_raw_temp = raw_temp;

//         // 3. 自适应滤波系数
//         float alpha = BASE_ALPHA + fminf(MAX_ALPHA - BASE_ALPHA, abs_rate * RATE_SENS);
//         if (alpha > MAX_ALPHA) alpha = MAX_ALPHA;

//         // 4. 温度滤波
//         float smooth_temp = alpha * raw_temp + (1.0f - alpha) * last_smooth_temp;
//         last_smooth_temp = smooth_temp;

//         // 5. 映射到 duty_cycle
//         long long target_duty_ns = map_temp_to_duty_cycle(smooth_temp,
//             fan_ctrl->mode == 0 ? TEMP_MIN_REAL : TEMP_MIN_SIM,
//             fan_ctrl->mode == 0 ? TEMP_MAX_REAL : TEMP_MAX_SIM);

//         // 6. 输出死区：变化小于阈值时不更新
//         if (llabs(target_duty_ns - last_duty_ns) < DUTY_DEADBAND) {
//             target_duty_ns = last_duty_ns;
//         } else {
//             last_duty_ns = target_duty_ns;
//         }

//         // 7. 应用 PWM
//         set_pwm_duty_cycle(target_duty_ns);

//         // 8. 每 5 次循环（约 10 秒）打印一次状态
//         if (++print_cnt >= 5) {
//             print_cnt = 0;
//             printf("Temp: %.1f°C (raw %.1f, rate %.2f°C/s, α=%.2f) | duty_cycle: %lld ns | Mode: %s\n",
//                    smooth_temp, raw_temp, abs_rate, alpha, target_duty_ns,
//                    fan_ctrl->mode ? "SIM" : "REAL");
//         }

//         sleep(2);   // 每 2 秒调节一次

        
//     }

//     // 退出时停转
//     set_pwm_duty_cycle(PERIOD_NS);
//     shmdt(fan_ctrl);
//     printf("Fan stopped.\n");
//     return 0;
// }


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <math.h>
#include <time.h>
#include "shared_mem.h"

// ========== 温控参数 ==========
#define TEMP_LOW       40.0f
#define TEMP_HIGH      60.0f
#define TEMP_CRITICAL  TEMP_HIGH
#define TEMP_RANGE     (TEMP_HIGH - TEMP_LOW)

#define ENV_TEMP_MIN   22.0f
#define ENV_TEMP_MAX   26.0f
#define ENV_HUMI_MIN   40
#define ENV_HUMI_MAX   70
#define ENV_MIN_SPEED  40          // 环境舒适最低风速（%），提高至30%确保风扇启动

#define SILENT_MAX_SPEED 30

#define THERMAL_MIN_SPEED 50       // 散热模式最低风速（%），保证风扇持续转动

#define MODE_AUTO       0
#define MODE_MANUAL     2
#define MODE_OFF        3

#define PWM_CHIP    "/sys/class/pwm/pwmchip0"
#define PWM_CHAN    "pwm0"
#define PERIOD_NS   1000000
#define TEMP_PATH   "/sys/class/thermal/thermal_zone0/temp"

static volatile int running = 1;
void sigint_handler(int sig) { running = 0; }

int write_sysfs(const char *path, const char *value) {
    int fd = open(path, O_WRONLY);
    if (fd < 0) return -1;
    ssize_t len = write(fd, value, strlen(value));
    close(fd);
    return (len == (ssize_t)strlen(value)) ? 0 : -1;
}

int set_pwm_duty_cycle(long long duty_ns) {
    if (duty_ns < 0) duty_ns = 0;
    if (duty_ns > PERIOD_NS) duty_ns = PERIOD_NS;
    char buf[32], path[128];
    snprintf(buf, sizeof(buf), "%lld", duty_ns);
    snprintf(path, sizeof(path), "%s/%s/duty_cycle", PWM_CHIP, PWM_CHAN);
    return write_sysfs(path, buf);
}

static long long speed_to_duty(int speed_percent) {
    if (speed_percent < 0) speed_percent = 0;
    if (speed_percent > 100) speed_percent = 100;
    // 极性 inversed: 转速100% -> duty=0; 转速0% -> duty=PERIOD_NS
    return (long long)((100 - speed_percent) / 100.0f * PERIOD_NS);
}

float get_cpu_temp(void) {
    FILE *fp = fopen(TEMP_PATH, "r");
    if (!fp) return 45.0f;
    int val;
    fscanf(fp, "%d", &val);
    fclose(fp);
    return val / 1000.0f;
}

int calc_env_speed(float env_temp, int env_humi) {
    float score = 0.3f;   // 基础30%
    if (env_temp < ENV_TEMP_MIN) {
        score += (ENV_TEMP_MIN - env_temp) * 0.05f;
    } else if (env_temp > ENV_TEMP_MAX) {
        score += (env_temp - ENV_TEMP_MAX) * 0.05f;
    } else {
        score -= 0.1f;
    }
    if (env_humi < ENV_HUMI_MIN) {
        score += (ENV_HUMI_MIN - env_humi) * 0.01f;
    } else if (env_humi > ENV_HUMI_MAX) {
        score += (env_humi - ENV_HUMI_MAX) * 0.01f;
    } else {
        score -= 0.1f;
    }
    if (score < ENV_MIN_SPEED / 100.0f) score = ENV_MIN_SPEED / 100.0f;
    if (score > 1.0f) score = 1.0f;
    return (int)(score * 100);
}

int main(void) {
    signal(SIGINT, sigint_handler);

    int shm_fan = shmget(FAN_CTRL_SHM_KEY, sizeof(FanCtrl), 0666);
    int created = 0;
    if (shm_fan < 0) {
        shm_fan = shmget(FAN_CTRL_SHM_KEY, sizeof(FanCtrl), IPC_CREAT | 0666);
        created = 1;
    }
    if (shm_fan < 0) { perror("shmget fan_ctrl"); return -1; }

    FanCtrl *fan_ctrl = (FanCtrl*)shmat(shm_fan, NULL, 0);
    if (fan_ctrl == (void*)-1) { perror("shmat fan_ctrl"); return -1; }

    if (created) {
        memset(fan_ctrl, 0, sizeof(FanCtrl));
        fan_ctrl->mode = MODE_AUTO;
        fan_ctrl->manual_speed = 50;
        fan_ctrl->sim_temp = 45.0f;
    }

    int shm_env = shmget(ENV_SHM_KEY, sizeof(EnvData), 0666);
    EnvData *env_data = NULL;
    if (shm_env >= 0) {
        env_data = (EnvData*)shmat(shm_env, NULL, 0);
        if (env_data == (void*)-1) env_data = NULL;
    }

    int shm_res = shmget(RESULT_SHM_KEY, sizeof(BehaviorResult), 0666);
    BehaviorResult *behav = NULL;
    if (shm_res >= 0) {
        behav = (BehaviorResult*)shmat(shm_res, NULL, 0);
        if (behav == (void*)-1) behav = NULL;
    }

    write_sysfs(PWM_CHIP "/export", "0");
    usleep(100000);
    write_sysfs(PWM_CHIP "/" PWM_CHAN "/period", "1000000");
    write_sysfs(PWM_CHIP "/" PWM_CHAN "/enable", "1");
    // 初始转速设为 THERMAL_MIN_SPEED，确保启动
    set_pwm_duty_cycle(speed_to_duty(THERMAL_MIN_SPEED));

    printf("Smart Fan Control Started (min thermal speed = %d%%)\n", THERMAL_MIN_SPEED);

    int print_cnt = 0;
    while (running) {
        int mode = fan_ctrl->mode;
        int manual_speed = fan_ctrl->manual_speed;
        if (manual_speed < 0) manual_speed = 0;
        if (manual_speed > 100) manual_speed = 100;

        float chip_temp = get_cpu_temp();
        int target_speed = 0;

        if (chip_temp >= TEMP_CRITICAL) {
    // 高温保护：强制全速
    target_speed = 100;
} else {
    // 60°C 以下，用户模式优先
    if (mode == MODE_OFF) {
        target_speed = 0;
    } else if (mode == MODE_MANUAL) {
        target_speed = manual_speed;
    } else { // MODE_AUTO
        float env_temp = env_data ? env_data->temperature : 25.0f;
        int env_humi = env_data ? (int)env_data->humidity : 50;
        if (chip_temp >= TEMP_LOW) {
            // 45-60°C 线性散热
            float ratio = (chip_temp - TEMP_LOW) / TEMP_RANGE;
            target_speed = (int)(ratio * 100);
            if (target_speed < THERMAL_MIN_SPEED) target_speed = THERMAL_MIN_SPEED;
        } else {
            // <45°C 环境舒适
            target_speed = calc_env_speed(env_temp, env_humi);
        }
        // 静音限制（仅自动模式）
        if (chip_temp < TEMP_LOW && behav && (behav->phone_call || behav->eye_closed || behav->head_down)) {
            if (target_speed > SILENT_MAX_SPEED) target_speed = SILENT_MAX_SPEED;
        }
    }
}

        if (target_speed < 0) target_speed = 0;
        if (target_speed > 100) target_speed = 100;

        set_pwm_duty_cycle(speed_to_duty(target_speed));

        fan_ctrl->current_speed = target_speed;
        fan_ctrl->chip_temp = chip_temp;
        fan_ctrl->timestamp = (uint64_t)time(NULL);

        if (++print_cnt >= 5) {
            print_cnt = 0;
            const char *mode_str;
            if (mode == MODE_AUTO) mode_str = "AUTO";
            else if (mode == MODE_MANUAL) mode_str = "MANUAL";
            else if (mode == MODE_OFF) mode_str = "OFF";
            else mode_str = "UNKNOWN";
            printf("[STATUS] Mode=%s, Chip=%.1f°C, Speed=%d%%, Duty=%lldns\n",
                   mode_str, chip_temp, target_speed,
                   speed_to_duty(target_speed));
        }

        sleep(2);
    }

    // 退出时停转：先确保使能，再设置周期占空比
write_sysfs(PWM_CHIP "/" PWM_CHAN "/enable", "1");
set_pwm_duty_cycle(PERIOD_NS);
printf("Fan stopped.\n");
shmdt(fan_ctrl);
if (env_data) shmdt(env_data);
if (behav) shmdt(behav);
return 0;
}