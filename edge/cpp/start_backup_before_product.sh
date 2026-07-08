# #!/bin/bash
# cd /home/elf/rknn_model_zoo/rknn_model_zoo-2.1.0/examples/yolov8_pose/cpp

# # 停止图形界面，释放 framebuffer
# sudo systemctl stop gdm3

# # 设置环境变量
# export LD_LIBRARY_PATH=/usr/lib:$LD_LIBRARY_PATH

# # 启动三个进程（注意顺序：先生产者，再消费者）
# ./gst_producer &
# sleep 2
# (cd build && ./behavior_engine) &   # ← 关键修改
# sleep 1
# sudo ./build/ui_display

# # 当 ui_display 退出时，自动杀死其他两个
# killall gst_producer behavior_engine 2>/dev/null

# #!/bin/bash
# cd /home/elf/rknn_model_zoo/rknn_model_zoo-2.1.0/examples/yolov8_pose/cpp

# cleanup() {
#     echo "Stopping all processes..."
#     # 强制停转风扇（硬件级）
#     sudo sh -c "echo 1 > /sys/class/pwm/pwmchip0/pwm0/enable" 2>/dev/null
#     sudo sh -c "echo 1000000 > /sys/class/pwm/pwmchip0/pwm0/duty_cycle" 2>/dev/null
#     sudo killall gst_producer behavior_engine env_monitor fan_control 2>/dev/null
#     exit 0
# }
# trap cleanup SIGINT

# sudo systemctl stop gdm3
# export LD_LIBRARY_PATH=/usr/lib:$LD_LIBRARY_PATH

# # 摄像头采集 → 大核 CPU4
# taskset -c 4 ./gst_producer &
# PID_CAM=$!

# sleep 2

# # AI 推理 → 大核 CPU5-7
# (cd build && taskset -c 5-7 ./behavior_engine) &
# PID_AI=$!

# sleep 1

# # 温湿度监控 → 小核 CPU2
# # sudo taskset -c 2 ./build/env_monitor &
# sudo taskset -c 2 ./build/env_monitor > /dev/null 2>&1 &
# PID_ENV=$!

# #风扇控制（可选）→ 小核 CPU3
# sudo taskset -c 3 ./build/fan_control &
# PID_FAN=$!

# # # LVGL 界面（前台，绑定小核 CPU0-1）
# # sudo taskset -c 0-1 /home/elf/rknn_model_zoo/rknn_model_zoo-2.1.0/examples/yolov8_pose/lvgl_display/lv_port_linux_frame_buffer/demo

# # # 当 LVGL 退出时，杀死所有后台进程
# # sudo sh -c "echo 1 > /sys/class/pwm/pwmchip0/pwm0/enable" 2>/dev/null
# # sudo sh -c "echo 1000000 > /sys/class/pwm/pwmchip0/pwm0/duty_cycle" 2>/dev/null
# # sudo killall gst_producer behavior_engine env_monitor fan_control 2>/dev/null

# # echo "All processes stopped."

# # ===== 关键修改开始 =====
# # 将 demo 放到后台，并记录 PID
# sudo taskset -c 0-1 /home/elf/rknn_model_zoo/rknn_model_zoo-2.1.0/examples/yolov8_pose/lvgl_display/lv_port_linux_frame_buffer/demo &
# DEMO_PID=$!

# # 等待 demo 进程结束（如果正常退出或被杀）
# wait $DEMO_PID
# # ===== 关键修改结束 =====

# # 当 demo 退出后执行清理（正常退出也会走这里）
# sudo sh -c "echo 1 > /sys/class/pwm/pwmchip0/pwm0/enable" 2>/dev/null
# sudo sh -c "echo 1000000 > /sys/class/pwm/pwmchip0/pwm0/duty_cycle" 2>/dev/null
# sudo killall gst_producer behavior_engine env_monitor fan_control 2>/dev/null

# echo "All processes stopped."





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