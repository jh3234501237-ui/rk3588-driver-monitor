#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <ctype.h>
#include <time.h>

#define CMD_FILE "/tmp/voice_cmd"
#define CMD_FILE_TMP "/tmp/voice_cmd.tmp"
#define ALERT_FILE "/tmp/voice_alert"

static speed_t get_baud(int baud)
{
    switch (baud) {
    case 9600: return B9600;
    case 115200: return B115200;
    default: return B9600;
    }
}

static int open_serial(const char *dev, int baud)
{
    int fd = open(dev, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) {
        perror("open serial");
        return -1;
    }

    struct termios tio;
    memset(&tio, 0, sizeof(tio));

    if (tcgetattr(fd, &tio) != 0) {
        perror("tcgetattr");
        close(fd);
        return -1;
    }

    cfmakeraw(&tio);

    speed_t spd = get_baud(baud);
    cfsetispeed(&tio, spd);
    cfsetospeed(&tio, spd);

    tio.c_cflag |= (CLOCAL | CREAD);
    tio.c_cflag &= ~CSIZE;
    tio.c_cflag |= CS8;
    tio.c_cflag &= ~PARENB;
    tio.c_cflag &= ~CSTOPB;
    tio.c_cflag &= ~CRTSCTS;

    tio.c_cc[VMIN]  = 0;
    tio.c_cc[VTIME] = 1;

    tcflush(fd, TCIFLUSH);

    if (tcsetattr(fd, TCSANOW, &tio) != 0) {
        perror("tcsetattr");
        close(fd);
        return -1;
    }

    return fd;
}

static void trim(char *s)
{
    if (!s) return;

    int len = strlen(s);
    while (len > 0 && (s[len - 1] == '\n' || s[len - 1] == '\r' ||
                       s[len - 1] == ' '  || s[len - 1] == '\t')) {
        s[len - 1] = '\0';
        len--;
    }

    char *p = s;
    while (*p && isspace((unsigned char)*p)) p++;

    if (p != s) {
        memmove(s, p, strlen(p) + 1);
    }
}

static const char *normalize_cmd(const char *raw)
{
    if (!raw || raw[0] == '\0') return NULL;

    /*
     * 支持字符串命令
     */
    if (strcasecmp(raw, "HOME") == 0) return "HOME";
    if (strcasecmp(raw, "DETECT") == 0) return "DETECT";
    if (strcasecmp(raw, "DETECTION") == 0) return "DETECT";
    if (strcasecmp(raw, "ENV") == 0) return "ENV";
    if (strcasecmp(raw, "ENVIRONMENT") == 0) return "ENV";
    if (strcasecmp(raw, "RECORD") == 0) return "RECORD";
    if (strcasecmp(raw, "RECORDS") == 0) return "RECORD";
    if (strcasecmp(raw, "SETTING") == 0) return "SETTING";
    if (strcasecmp(raw, "SETTINGS") == 0) return "SETTING";

        /*
     * Fan control commands
     */
    if (strcasecmp(raw, "FAN_ON") == 0) return "FAN_ON";
    if (strcasecmp(raw, "FAN_MAX") == 0) return "FAN_MAX";
    if (strcasecmp(raw, "FAN_OFF") == 0) return "FAN_OFF";
    if (strcasecmp(raw, "FAN_AUTO") == 0) return "FAN_AUTO";



    /*
     * 支持数字命令
     */
    if (strcmp(raw, "1") == 0) return "HOME";
    if (strcmp(raw, "2") == 0) return "DETECT";
    if (strcmp(raw, "3") == 0) return "ENV";
    if (strcmp(raw, "4") == 0) return "RECORD";
    if (strcmp(raw, "5") == 0) return "SETTING";

    return NULL;
}

static void write_cmd_file(const char *cmd)
{
    FILE *fp = fopen(CMD_FILE_TMP, "w");
    if (!fp) {
        perror("fopen cmd tmp");
        return;
    }

    fprintf(fp, "%s\n", cmd);
    fclose(fp);

    rename(CMD_FILE_TMP, CMD_FILE);
}

static void process_alert_file(int fd)
{
    FILE *fp = fopen(ALERT_FILE, "r");
    if (!fp) {
        return;
    }

    char alert[128] = {0};

    if (fgets(alert, sizeof(alert), fp) == NULL) {
        fclose(fp);
        unlink(ALERT_FILE);
        return;
    }

    fclose(fp);
    unlink(ALERT_FILE);

    trim(alert);

    if (alert[0] == '\0') {
        return;
    }

    /*
     * 发送给 ASRPRO。
     * 这里不加换行，因为你已经测试 printf ALERT_HEAD 可以播报。
     */
    int n = write(fd, alert, strlen(alert));

    if (n > 0) {
        printf("[VOICE] send alert: %s\n", alert);
        fflush(stdout);
    } else {
        perror("[VOICE] write alert");
    }
}

int main(int argc, char **argv)
{
    const char *dev = "/dev/ttyUSB0";
    int baud = 9600;

    if (argc >= 2) dev = argv[1];
    if (argc >= 3) baud = atoi(argv[2]);

    printf("[VOICE] voice_monitor starting...\n");
    printf("[VOICE] serial dev: %s, baud: %d\n", dev, baud);
    fflush(stdout);

    int fd = open_serial(dev, baud);
    if (fd < 0) {
        printf("[VOICE] failed to open serial: %s\n", dev);
        return 1;
    }

    char line[128];
    int pos = 0;

    while (1) {
        process_alert_file(fd);
        
        char ch;
        int n = read(fd, &ch, 1);

        if (n > 0) {
            if (ch == '\n' || ch == '\r') {
                line[pos] = '\0';
                trim(line);

                if (pos > 0 && line[0] != '\0') {
                    const char *cmd = normalize_cmd(line);

                    if (cmd) {
                        printf("[VOICE] raw='%s' -> cmd=%s\n", line, cmd);
                        write_cmd_file(cmd);
                    } else {
                        printf("[VOICE] unknown raw='%s'\n", line);
                    }

                    fflush(stdout);
                }

                pos = 0;
            } else {
                if (pos < (int)sizeof(line) - 1) {
                    line[pos++] = ch;
                } else {
                    pos = 0;
                }
            }
        } else {
            usleep(20000);
        }
    }

    close(fd);
    return 0;
}