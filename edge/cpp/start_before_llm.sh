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