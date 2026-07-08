#!/bin/bash
ipcs -m | grep -E "0x1234|0x5678" | awk '{print $2}' | xargs -r ipcrm -m
echo "Shared memory cleaned."