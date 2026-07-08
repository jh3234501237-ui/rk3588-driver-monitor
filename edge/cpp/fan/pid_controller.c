#include "pid_controller.h"

void pid_init(PID_Controller *pid, float kp, float ki, float kd, float setpoint) {
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
    pid->setpoint = setpoint;
    pid->integral = 0.0f;
    pid->last_error = 0.0f;
    pid->output_min = 0.0f;
    pid->output_max = 1.0f;
}

float pid_compute(PID_Controller *pid, float current_value) {
    float error = pid->setpoint - current_value;
    // 积分项 (抗积分饱和)
    pid->integral += error;
    if (pid->integral > 100.0f) pid->integral = 100.0f;
    if (pid->integral < -100.0f) pid->integral = -100.0f;
    // 微分项
    float derivative = error - pid->last_error;
    pid->last_error = error;
    // 计算输出
    float output = pid->kp * error + pid->ki * pid->integral + pid->kd * derivative;
    // 限制输出范围
    if (output > pid->output_max) output = pid->output_max;
    if (output < pid->output_min) output = pid->output_min;
    return output;
}

void pid_reset(PID_Controller *pid) {
    pid->integral = 0.0f;
    pid->last_error = 0.0f;
}