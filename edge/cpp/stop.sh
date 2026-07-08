#!/bin/bash

killall gst_producer behavior_engine env_monitor fan_control voice_monitor demo ui_display 2>/dev/null
rm -f /tmp/voice_cmd /tmp/voice_cmd.tmp

sudo systemctl start gdm3