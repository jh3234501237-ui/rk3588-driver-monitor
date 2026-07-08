#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#define MPU_ADDR        0x68

#define REG_PWR_MGMT_1  0x6B
#define REG_ACCEL_XOUT  0x3B
#define REG_WHO_AM_I    0x75

static int i2c_write_byte(int fd, uint8_t reg, uint8_t value)
{
    uint8_t buf[2] = {reg, value};
    if (write(fd, buf, 2) != 2) {
        perror("i2c write");
        return -1;
    }
    return 0;
}

static int i2c_read_bytes(int fd, uint8_t reg, uint8_t *buf, int len)
{
    if (write(fd, &reg, 1) != 1) {
        perror("i2c set reg");
        return -1;
    }

    if (read(fd, buf, len) != len) {
        perror("i2c read");
        return -1;
    }

    return 0;
}

static int16_t to_int16(uint8_t high, uint8_t low)
{
    return (int16_t)((high << 8) | low);
}

int main(int argc, char **argv)
{
    const char *dev = "/dev/i2c-7";

    if (argc >= 2) {
        dev = argv[1];
    }

    int fd = open(dev, O_RDWR);
    if (fd < 0) {
        perror("open i2c");
        return 1;
    }

    if (ioctl(fd, I2C_SLAVE, MPU_ADDR) < 0) {
        perror("ioctl I2C_SLAVE");
        close(fd);
        return 1;
    }

    uint8_t who = 0;
    if (i2c_read_bytes(fd, REG_WHO_AM_I, &who, 1) < 0) {
        close(fd);
        return 1;
    }

    printf("MPU6050 WHO_AM_I = 0x%02X\n", who);

    if (who != 0x68) {
        printf("Warning: WHO_AM_I is not 0x68, please check device.\n");
    }

    /*
     * 唤醒 MPU6050。
     * 默认上电后 MPU6050 处于 sleep 状态，必须写 0。
     */
    if (i2c_write_byte(fd, REG_PWR_MGMT_1, 0x00) < 0) {
        close(fd);
        return 1;
    }

    usleep(100000);

    printf("Reading accel/gyro data from %s, addr 0x68...\n", dev);
    printf("Press Ctrl+C to stop.\n\n");

    while (1) {
        uint8_t data[14];

        if (i2c_read_bytes(fd, REG_ACCEL_XOUT, data, 14) < 0) {
            break;
        }

        int16_t ax_raw = to_int16(data[0], data[1]);
        int16_t ay_raw = to_int16(data[2], data[3]);
        int16_t az_raw = to_int16(data[4], data[5]);

        int16_t temp_raw = to_int16(data[6], data[7]);

        int16_t gx_raw = to_int16(data[8], data[9]);
        int16_t gy_raw = to_int16(data[10], data[11]);
        int16_t gz_raw = to_int16(data[12], data[13]);

        /*
         * 默认量程：
         * 加速度 ±2g，比例 16384 LSB/g
         * 陀螺仪 ±250 deg/s，比例 131 LSB/(deg/s)
         */
        float ax = ax_raw / 16384.0f;
        float ay = ay_raw / 16384.0f;
        float az = az_raw / 16384.0f;

        float gx = gx_raw / 131.0f;
        float gy = gy_raw / 131.0f;
        float gz = gz_raw / 131.0f;

        float temp = temp_raw / 340.0f + 36.53f;

        printf("ACC[g] ax=%7.3f ay=%7.3f az=%7.3f | "
               "GYRO[dps] gx=%8.2f gy=%8.2f gz=%8.2f | "
               "TEMP=%.2f C\n",
               ax, ay, az, gx, gy, gz, temp);

        usleep(200000);
    }

    close(fd);
    return 0;
}