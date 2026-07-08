#!/bin/bash

APP_DIR=/home/elf/rknn_model_zoo/rknn_model_zoo-2.1.0/examples/yolov8_pose/cpp
UI_BIN=/home/elf/rknn_model_zoo/rknn_model_zoo-2.1.0/examples/yolov8_pose/lvgl_display/lv_port_linux_frame_buffer/demo
LOG_DIR=$APP_DIR/logs

cd "$APP_DIR" || exit 1
mkdir -p "$LOG_DIR"

echo "[START] Smart fatigue system starting..."

# 1. 从 RTC 恢复系统时间
echo "[START] Sync time from RTC..."
hwclock -s 2>/dev/null

# 2. 停止桌面，释放 framebuffer / 显示资源
systemctl stop gdm3 2>/dev/null

export LD_LIBRARY_PATH=/usr/lib:$LD_LIBRARY_PATH

cleanup() {
    echo "[STOP] Stopping all processes..."

    killall gst_producer behavior_engine env_monitor fan_control 2>/dev/null

    # 风扇安全处理：保持一个安全低速，避免系统退出后风扇完全失控
    sh -c "echo 1 > /sys/class/pwm/pwmchip0/pwm0/enable" 2>/dev/null
    sh -c "echo 1000000 > /sys/class/pwm/pwmchip0/pwm0/duty_cycle" 2>/dev/null

    echo "[STOP] All processes stopped."
    exit 0
}

trap cleanup SIGINT SIGTERM

# 3. 先清理旧进程
killall gst_producer behavior_engine env_monitor fan_control demo 2>/dev/null
sleep 1

# 4. 摄像头采集
echo "[START] gst_producer..."
taskset -c 4 ./gst_producer > "$LOG_DIR/gst.log" 2>&1 &
PID_CAM=$!
sleep 2

# 5. AI 推理
echo "[START] behavior_engine..."
(cd build && taskset -c 5-7 ./behavior_engine > "$LOG_DIR/behavior.log" 2>&1) &
PID_AI=$!
sleep 1

# 6. 温湿度监控
echo "[START] env_monitor..."
taskset -c 2 ./build/env_monitor > "$LOG_DIR/env.log" 2>&1 &
PID_ENV=$!
sleep 1

# 7. 风扇控制
echo "[START] fan_control..."
taskset -c 3 ./build/fan_control > "$LOG_DIR/fan.log" 2>&1 &
PID_FAN=$!
sleep 2

# 8. 启动 UI
echo "[START] UI demo..."
taskset -c 0-1 "$UI_BIN" > "$LOG_DIR/ui.log" 2>&1 &
DEMO_PID=$!

wait $DEMO_PID

cleanup