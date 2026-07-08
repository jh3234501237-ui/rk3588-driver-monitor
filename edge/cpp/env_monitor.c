#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/shm.h>
#include <time.h>
#include <stdint.h>
#include "shared_mem.h"

static volatile int running = 1;
void sigint_handler(int sig) { running = 0; }

// 读取 AHT20 传感器数据（通过 i2ctransfer 命令）
static int read_aht20(float *temperature, float *humidity) {
    FILE *fp;
    char command[128];
    unsigned char raw_data[6] = {0};
    uint32_t raw_humidity, raw_temperature;

    // 1. 触发测量
    snprintf(command, sizeof(command), "i2ctransfer -f -y 4 w3@0x38 0xAC 0x33 0x00");
    system(command);

    usleep(80000); // 等待 80ms

    // 2. 读取 6 字节数据
    snprintf(command, sizeof(command), "i2ctransfer -f -y 4 w1@0x38 0x71 r6");
    fp = popen(command, "r");
    if (fp == NULL) {
        return -1;
    }
    int res = fscanf(fp, "%hhx %hhx %hhx %hhx %hhx %hhx",
                     &raw_data[0], &raw_data[1], &raw_data[2],
                     &raw_data[3], &raw_data[4], &raw_data[5]);
    pclose(fp);
    if (res != 6) return -1;

    // 3. 解析数据（根据 AHT20 手册）
    raw_humidity = ((uint32_t)raw_data[1] << 12) | ((uint32_t)raw_data[2] << 4) | (raw_data[3] >> 4);
    *humidity = (float)raw_humidity / 1048576.0 * 100.0;

    raw_temperature = ((uint32_t)(raw_data[3] & 0x0F) << 16) | ((uint32_t)raw_data[4] << 8) | raw_data[5];
    *temperature = (float)raw_temperature / 1048576.0 * 200.0 - 50.0;

    return 0;
}

int main() {
    signal(SIGINT, sigint_handler);

    // 创建/连接共享内存（键值 ENV_SHM_KEY 需要在 shared_mem.h 中定义为 0x567A）
    int shm_id = shmget(ENV_SHM_KEY, sizeof(EnvData), IPC_CREAT | 0666);
    if (shm_id < 0) { perror("shmget env"); return -1; }
    EnvData *env = (EnvData*)shmat(shm_id, NULL, 0);
    if (env == (void*)-1) { perror("shmat env"); return -1; }
    memset(env, 0, sizeof(EnvData));

    printf("env_monitor started, sampling every 2 seconds...\n");

    while (running) {
        float temp = 0, hum = 0;
        if (read_aht20(&temp, &hum) == 0) {
            env->temperature = temp;
            env->humidity = hum;
            env->timestamp = (uint64_t)time(NULL) * 1000000LL;
            printf("Temp: %.2f°C  Hum: %.2f%%\n", temp, hum);
        } else {
            fprintf(stderr, "Failed to read AHT20\n");
        }
        sleep(2);
    }

    shmdt(env);
    return 0;
}