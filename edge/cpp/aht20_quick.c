#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>   // 添加 uint32_t 类型

int read_sensor_data(unsigned char *data) {
    FILE *fp;
    char command[128];
    // 1. 触发测量
    snprintf(command, sizeof(command), "i2ctransfer -f -y 4 w3@0x38 0xAC 0x33 0x00");
    system(command);

    usleep(80000); // 等待80ms，手册要求至少75ms

    // 2. 读取6字节数据
    snprintf(command, sizeof(command), "i2ctransfer -f -y 4 w1@0x38 0x71 r6");
    fp = popen(command, "r");
    if (fp == NULL) {
        return -1;
    }

    // 解析输出，例如 "0x00 0x00 0x00 ..."
    int res = fscanf(fp, "%hhx %hhx %hhx %hhx %hhx %hhx", 
                     &data[0], &data[1], &data[2], &data[3], &data[4], &data[5]);
    pclose(fp);
    
    return (res == 6) ? 0 : -1;
}

int main() {
    unsigned char raw_data[6] = {0};
    uint32_t raw_humidity, raw_temperature;
    float humidity, temperature;

    if (read_sensor_data(raw_data) != 0) {
        printf("Failed to read sensor data.\n");
        return 1;
    }

    // 根据 AHT20 数据手册解析数据 (20位湿度 + 20位温度)
    raw_humidity = ((uint32_t)raw_data[1] << 12) | ((uint32_t)raw_data[2] << 4) | (raw_data[3] >> 4);
    humidity = (float)raw_humidity / 1048576.0 * 100.0;

    raw_temperature = ((uint32_t)(raw_data[3] & 0x0F) << 16) | ((uint32_t)raw_data[4] << 8) | raw_data[5];
    temperature = (float)raw_temperature / 1048576.0 * 200.0 - 50.0;

    printf("Temperature: %.2f°C\n", temperature);
    printf("Humidity: %.2f%%\n", humidity);

    return 0;
}