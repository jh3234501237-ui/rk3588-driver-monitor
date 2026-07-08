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

echo "[START] Sync time from RTC..."
hwclock -s 2>/dev/null

echo "[START] Stop desktop display manager..."
systemctl stop gdm3 2>/dev/null

export LD_LIBRARY_PATH=/usr/lib:$LD_LIBRARY_PATH

cleanup() {
    echo "[STOP] Stopping all processes..."

    killall gst_producer behavior_engine env_monitor fan_control voice_monitor motion_monitor demo 2>/dev/null

    sh -c "echo 1 > /sys/class/pwm/pwmchip0/pwm0/enable" 2>/dev/null
    sh -c "echo 1000000 > /sys/class/pwm/pwmchip0/pwm0/duty_cycle" 2>/dev/null

    rm -f /tmp/voice_cmd /tmp/voice_cmd.tmp /tmp/voice_alert /tmp/voice_broadcast_enable /tmp/vehicle_state /tmp/vehicle_mode

    echo "[STOP] All processes stopped."
    exit 0
}

trap cleanup SIGINT SIGTERM

echo "[START] Kill old processes..."
killall gst_producer behavior_engine env_monitor fan_control voice_monitor motion_monitor demo 2>/dev/null
rm -f /tmp/voice_cmd /tmp/voice_cmd.tmp /tmp/voice_alert /tmp/vehicle_state /tmp/vehicle_mode
echo 1 > /tmp/voice_broadcast_enable
echo AUTO > /tmp/vehicle_mode
sleep 1

echo "[START] gst_producer..."
taskset -c 4 ./gst_producer > "$LOG_DIR/gst.log" 2>&1 &
PID_CAM=$!
sleep 2

echo "[START] behavior_engine..."
(cd build && taskset -c 5-7 ./behavior_engine > "$LOG_DIR/behavior.log" 2>&1) &
PID_AI=$!
sleep 1

echo "[START] env_monitor..."
taskset -c 2 ./build/env_monitor > "$LOG_DIR/env.log" 2>&1 &
PID_ENV=$!
sleep 1

echo "[START] fan_control..."
taskset -c 3 ./build/fan_control > "$LOG_DIR/fan.log" 2>&1 &
PID_FAN=$!
sleep 1

echo "[START] motion_monitor..."
if [ -e /dev/i2c-7 ] && [ -x ./build/motion_monitor ]; then
    ./build/motion_monitor /dev/i2c-7 > "$LOG_DIR/motion.log" 2>&1 &
    PID_MOTION=$!
    echo "[START] motion_monitor started on /dev/i2c-7, pid=$PID_MOTION"
else
    echo "[WARN] /dev/i2c-7 not found or build/motion_monitor not executable, motion_monitor skipped."
    PID_MOTION=0

    # 默认写入 PARKED，避免主程序读取不到文件
    cat > /tmp/vehicle_state <<EOF
moving=0
state=PARKED
dyn_acc=0.0000
gyro_mag=0.00
EOF
fi
sleep 0.5

echo "[START] voice_monitor..."

VOICE_DEV=""

for i in $(seq 1 20); do
    for dev in /dev/ttyUSB0 /dev/ttyUSB1 /dev/ttyUSB2 /dev/ttyUSB3 /dev/ttyUSB4 /dev/ttyUSB5; do
        if [ -e "$dev" ]; then
            VOICE_DEV="$dev"
            break
        fi
    done

    if [ -n "$VOICE_DEV" ]; then
        break
    fi

    echo "[WAIT] waiting for voice serial device... $i"
    sleep 0.5
done

if [ -n "$VOICE_DEV" ]; then
    rm -f /tmp/voice_cmd /tmp/voice_cmd.tmp /tmp/voice_alert

    ./build/voice_monitor "$VOICE_DEV" 9600 > "$LOG_DIR/voice.log" 2>&1 &
    PID_VOICE=$!

    echo "[START] voice_monitor started on $VOICE_DEV, pid=$PID_VOICE"
else
    echo "[WARN] voice serial device not found, voice_monitor skipped."
    PID_VOICE=0
fi

sleep 0.5

echo "[START] UI demo..."
taskset -c 0-1 "$UI_BIN" > "$LOG_DIR/ui.log" 2>&1 &
DEMO_PID=$!

wait $DEMO_PID

cleanup