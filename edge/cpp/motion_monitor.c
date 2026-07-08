#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#define MPU_ADDR        0x68

#define REG_PWR_MGMT_1  0x6B
#define REG_ACCEL_CFG   0x1C
#define REG_GYRO_CFG    0x1B
#define REG_ACCEL_XOUT  0x3B
#define REG_WHO_AM_I    0x75

#define VEHICLE_STATE_FILE "/tmp/vehicle_state"

/*
 * 运动判断阈值：
 *
 * dyn_acc:
 *   当前加速度模长与低通重力估计之间的差值。
 *   静止时接近 0，晃动时变大。
 *
 * gyro_mag:
 *   去除零偏后的角速度模长。
 *
 * 这组参数适合比赛演示：
 *   晃动约 0.4 秒进入 MOVING
 *   停止约 1.5 秒恢复 PARKED
 */
#define MOVING_ACC_THRESHOLD   0.14f
#define MOVING_GYRO_THRESHOLD  80.0f

#define MOVING_HOLD_COUNT      4
#define PARKED_HOLD_COUNT      8

/*
 * 采样周期 100ms。
 */
#define SAMPLE_INTERVAL_US     100000

static int i2c_write_byte(int fd, uint8_t reg, uint8_t value)
{
    uint8_t buf[2] = {reg, value};

    if (write(fd, buf, 2) != 2) {
        perror("[MOTION] i2c write");
        return -1;
    }

    return 0;
}

static int i2c_read_bytes(int fd, uint8_t reg, uint8_t *buf, int len)
{
    if (write(fd, &reg, 1) != 1) {
        perror("[MOTION] i2c set reg");
        return -1;
    }

    if (read(fd, buf, len) != len) {
        perror("[MOTION] i2c read");
        return -1;
    }

    return 0;
}

static int16_t to_int16(uint8_t high, uint8_t low)
{
    return (int16_t)((high << 8) | low);
}

static void write_vehicle_state(int moving,
                                float dyn_acc,
                                float gyro_mag,
                                float ax,
                                float ay,
                                float az,
                                float gx,
                                float gy,
                                float gz)
{
    FILE *fp = fopen(VEHICLE_STATE_FILE, "w");
    if (!fp) {
        return;
    }

    /*
     * moving:
     *   0 = PARKED
     *   1 = MOVING
     */
    fprintf(fp,
            "moving=%d\n"
            "state=%s\n"
            "dyn_acc=%.4f\n"
            "gyro_mag=%.2f\n"
            "ax=%.4f\n"
            "ay=%.4f\n"
            "az=%.4f\n"
            "gx=%.2f\n"
            "gy=%.2f\n"
            "gz=%.2f\n",
            moving,
            moving ? "MOVING" : "PARKED",
            dyn_acc,
            gyro_mag,
            ax, ay, az,
            gx, gy, gz);

    fclose(fp);
}

static int read_mpu_raw(int fd,
                        int16_t *ax_raw,
                        int16_t *ay_raw,
                        int16_t *az_raw,
                        int16_t *gx_raw,
                        int16_t *gy_raw,
                        int16_t *gz_raw)
{
    uint8_t data[14];

    if (i2c_read_bytes(fd, REG_ACCEL_XOUT, data, 14) < 0) {
        return -1;
    }

    if (ax_raw) *ax_raw = to_int16(data[0], data[1]);
    if (ay_raw) *ay_raw = to_int16(data[2], data[3]);
    if (az_raw) *az_raw = to_int16(data[4], data[5]);

    /*
     * data[6], data[7] 是温度，这里暂时不用。
     */

    if (gx_raw) *gx_raw = to_int16(data[8], data[9]);
    if (gy_raw) *gy_raw = to_int16(data[10], data[11]);
    if (gz_raw) *gz_raw = to_int16(data[12], data[13]);

    return 0;
}

int main(int argc, char **argv)
{
    const char *dev = "/dev/i2c-7";

    if (argc >= 2) {
        dev = argv[1];
    }

    int fd = open(dev, O_RDWR);
    if (fd < 0) {
        perror("[MOTION] open i2c");
        return 1;
    }

    if (ioctl(fd, I2C_SLAVE, MPU_ADDR) < 0) {
        perror("[MOTION] ioctl I2C_SLAVE");
        close(fd);
        return 1;
    }

    uint8_t who = 0;
    if (i2c_read_bytes(fd, REG_WHO_AM_I, &who, 1) < 0) {
        close(fd);
        return 1;
    }

    printf("[MOTION] MPU6050 WHO_AM_I = 0x%02X\n", who);

    if (who != 0x68) {
        printf("[MOTION] Warning: WHO_AM_I is not 0x68, please check sensor.\n");
    }

    /*
     * 唤醒 MPU6050。
     */
    if (i2c_write_byte(fd, REG_PWR_MGMT_1, 0x00) < 0) {
        close(fd);
        return 1;
    }

    usleep(100000);

    /*
     * 设置量程：
     *
     * ACCEL_CONFIG:
     *   0x00 = ±2g,  16384 LSB/g
     *   0x08 = ±4g,   8192 LSB/g
     *   0x10 = ±8g,   4096 LSB/g
     *   0x18 = ±16g,  2048 LSB/g
     *
     * GYRO_CONFIG:
     *   0x00 = ±250 dps,  131 LSB/(deg/s)
     *   0x08 = ±500 dps,  65.5 LSB/(deg/s)
     *   0x10 = ±1000 dps, 32.8 LSB/(deg/s)
     *   0x18 = ±2000 dps, 16.4 LSB/(deg/s)
     */
    if (i2c_write_byte(fd, REG_ACCEL_CFG, 0x08) < 0) {
        close(fd);
        return 1;
    }

    if (i2c_write_byte(fd, REG_GYRO_CFG, 0x08) < 0) {
        close(fd);
        return 1;
    }

    const float accel_scale = 8192.0f;
    const float gyro_scale = 65.5f;

    /*
     * 陀螺仪零偏校准。
     * 启动后请保持模块静止约 1 秒。
     */
    float gx_bias = 0.0f;
    float gy_bias = 0.0f;
    float gz_bias = 0.0f;

    printf("[MOTION] Calibrating gyro bias, keep sensor still...\n");

    int calib_count = 100;
    int valid_count = 0;

    float gx_sum = 0.0f;
    float gy_sum = 0.0f;
    float gz_sum = 0.0f;

    for (int i = 0; i < calib_count; i++) {
        int16_t ax_raw, ay_raw, az_raw;
        int16_t gx_raw, gy_raw, gz_raw;

        if (read_mpu_raw(fd,
                         &ax_raw, &ay_raw, &az_raw,
                         &gx_raw, &gy_raw, &gz_raw) == 0) {
            gx_sum += gx_raw / gyro_scale;
            gy_sum += gy_raw / gyro_scale;
            gz_sum += gz_raw / gyro_scale;
            valid_count++;
        }

        usleep(10000);
    }

    if (valid_count > 0) {
        gx_bias = gx_sum / valid_count;
        gy_bias = gy_sum / valid_count;
        gz_bias = gz_sum / valid_count;
    }

    printf("[MOTION] Gyro bias: gx=%.2f gy=%.2f gz=%.2f, valid=%d\n",
           gx_bias, gy_bias, gz_bias, valid_count);

    /*
     * 低通估计重力分量。
     * 静止时加速度模长约为 1g。
     */
    float gravity_mag_lp = 1.0f;

    int moving_count = 0;
    int parked_count = 0;
    int moving_state = 0;

    printf("[MOTION] motion_monitor started on %s\n", dev);
    printf("[MOTION] output: %s\n", VEHICLE_STATE_FILE);
    printf("[MOTION] thresholds: dyn_acc > %.3f g, gyro > %.1f dps\n",
           MOVING_ACC_THRESHOLD,
           MOVING_GYRO_THRESHOLD);
    printf("[MOTION] hold: moving=%d samples, parked=%d samples\n",
           MOVING_HOLD_COUNT,
           PARKED_HOLD_COUNT);
    printf("[MOTION] Press Ctrl+C to stop.\n\n");

    /*
     * 先写一次默认 PARKED，避免主程序启动时读不到文件。
     */
    write_vehicle_state(0, 0.0f, 0.0f,
                        0.0f, 0.0f, 1.0f,
                        0.0f, 0.0f, 0.0f);

    while (1) {
        int16_t ax_raw, ay_raw, az_raw;
        int16_t gx_raw, gy_raw, gz_raw;

        if (read_mpu_raw(fd,
                         &ax_raw, &ay_raw, &az_raw,
                         &gx_raw, &gy_raw, &gz_raw) < 0) {
            usleep(SAMPLE_INTERVAL_US);
            continue;
        }

        float ax = ax_raw / accel_scale;
        float ay = ay_raw / accel_scale;
        float az = az_raw / accel_scale;

        float gx = gx_raw / gyro_scale - gx_bias;
        float gy = gy_raw / gyro_scale - gy_bias;
        float gz = gz_raw / gyro_scale - gz_bias;

        float acc_mag = sqrtf(ax * ax + ay * ay + az * az);
        float gyro_mag = sqrtf(gx * gx + gy * gy + gz * gz);

        /*
         * 低通估计重力大小。
         * 系数越接近 1，越平滑。
         */
        gravity_mag_lp = gravity_mag_lp * 0.98f + acc_mag * 0.02f;

        /*
         * 动态加速度：当前加速度模长与低通重力估计的差值。
         */
        float dyn_acc = fabsf(acc_mag - gravity_mag_lp);

        int instant_moving =
            (dyn_acc > MOVING_ACC_THRESHOLD) ||
            (gyro_mag > MOVING_GYRO_THRESHOLD);

        if (instant_moving) {
            moving_count++;
            parked_count = 0;
        } else {
            parked_count++;
            moving_count = 0;
        }

        if (moving_count >= MOVING_HOLD_COUNT) {
            moving_state = 1;
        }

        if (parked_count >= PARKED_HOLD_COUNT) {
            moving_state = 0;
        }

        write_vehicle_state(moving_state,
                            dyn_acc,
                            gyro_mag,
                            ax, ay, az,
                            gx, gy, gz);

        printf("[MOTION] %-6s | dyn_acc=%.4f gyro=%.2f | "
               "cnt_m=%02d cnt_p=%02d | "
               "acc=(%.3f %.3f %.3f) gyro=(%.2f %.2f %.2f)\n",
               moving_state ? "MOVING" : "PARKED",
               dyn_acc,
               gyro_mag,
               moving_count,
               parked_count,
               ax, ay, az,
               gx, gy, gz);

        usleep(SAMPLE_INTERVAL_US);
    }

    close(fd);
    return 0;
}