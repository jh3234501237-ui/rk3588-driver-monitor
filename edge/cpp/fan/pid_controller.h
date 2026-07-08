#ifndef PID_CONTROLLER_H
#define PID_CONTROLLER_H

typedef struct {
    float kp;          // 比例系数
    float ki;          // 积分系数
    float kd;          // 微分系数
    float setpoint;    // 目标温度 (摄氏度)
    float integral;    // 积分累加
    float last_error;  // 上次误差
    float output_min;  // 最小输出 (0.0)
    float output_max;  // 最大输出 (1.0)
} PID_Controller;

void pid_init(PID_Controller *pid, float kp, float ki, float kd, float setpoint);
float pid_compute(PID_Controller *pid, float current_value);
void pid_reset(PID_Controller *pid);

#endif