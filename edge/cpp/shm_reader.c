#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/shm.h>
#include "shared_mem.h"

int main() {
    int shm_id = shmget(SHM_KEY, sizeof(SharedFrame), 0666);
    if (shm_id < 0) {
        perror("shmget");
        return 1;
    }
    SharedFrame *frame = (SharedFrame*)shmat(shm_id, NULL, 0);
    if (frame == (void*)-1) {
        perror("shmat");
        return 1;
    }
    int last_cnt = -1;
    while (1) {
        if (frame->frame_count != last_cnt) {
            last_cnt = frame->frame_count;
            printf("Frame %d: %dx%d, timestamp=%llu\n", frame->frame_count,
                   frame->width, frame->height, frame->timestamp);
            // 可选：保存第一帧为文件验证图像
            if (last_cnt == 1) {
                FILE *fp = fopen("/tmp/first_frame.rgb", "wb");
                fwrite(frame->rgb_data, 1, FRAME_SIZE, fp);
                fclose(fp);
                printf("Saved first frame to /tmp/first_frame.rgb\n");
            }
        }
        usleep(100000);
    }
    return 0;
}
