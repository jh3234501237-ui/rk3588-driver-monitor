#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <signal.h>
#include <sys/shm.h>
#include "shared_mem.h"

static int shm_id = -1;
static SharedFrame *shared_frame = NULL;
static volatile int running = 1;

void sigint_handler(int sig) { running = 0; }

// NV12 -> RGB24
static void nv12_to_rgb(const unsigned char *nv12, int width, int height,
                        int stride_y, int stride_uv, unsigned char *rgb) {
    const unsigned char *y = nv12;
    const unsigned char *uv = nv12 + stride_y * height;
    for (int y_i = 0; y_i < height; y_i++) {
        for (int x = 0; x < width; x++) {
            int Y = y[y_i * stride_y + x];
            int U = uv[(y_i/2) * stride_uv + (x/2)];
            int V = uv[(y_i/2) * stride_uv + (x/2) + stride_uv * (height/2)];
            int R = Y + 1.402f * (V - 128);
            int G = Y - 0.344f * (U - 128) - 0.714f * (V - 128);
            int B = Y + 1.772f * (U - 128);
            rgb[(y_i * width + x) * 3 + 0] = (R < 0) ? 0 : (R > 255 ? 255 : R);
            rgb[(y_i * width + x) * 3 + 1] = (G < 0) ? 0 : (G > 255 ? 255 : G);
            rgb[(y_i * width + x) * 3 + 2] = (B < 0) ? 0 : (B > 255 ? 255 : B);
        }
    }
}

int main() {
    signal(SIGINT, sigint_handler);

    // 创建共享内存
    shm_id = shmget(SHM_KEY, sizeof(SharedFrame), IPC_CREAT | 0666);
    if (shm_id < 0) { perror("shmget"); return -1; }
    shared_frame = (SharedFrame*)shmat(shm_id, NULL, 0);
    if (shared_frame == (void*)-1) { perror("shmat"); return -1; }
    memset(shared_frame, 0, sizeof(SharedFrame));

    // 打开摄像头
    int cam_fd = open("/dev/video11", O_RDWR);
    if (cam_fd < 0) { perror("open /dev/video11"); return -1; }

    // 设置格式
    struct v4l2_format fmt = {0};
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    fmt.fmt.pix_mp.width = CAM_WIDTH;
    fmt.fmt.pix_mp.height = CAM_HEIGHT;
    fmt.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_NV12;
    fmt.fmt.pix_mp.field = V4L2_FIELD_NONE;
    fmt.fmt.pix_mp.num_planes = 1;
    if (ioctl(cam_fd, VIDIOC_S_FMT, &fmt) < 0) {
        perror("VIDIOC_S_FMT");
        return -1;
    }

    int stride_y = fmt.fmt.pix_mp.plane_fmt[0].bytesperline;
    int stride_uv = stride_y;
    int buf_len = fmt.fmt.pix_mp.plane_fmt[0].sizeimage;

    // 申请缓冲区
    struct v4l2_requestbuffers req = {0};
    req.count = 2;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    req.memory = V4L2_MEMORY_MMAP;
    if (ioctl(cam_fd, VIDIOC_REQBUFS, &req) < 0) { perror("REQBUFS"); return -1; }

    void *camera_buf[2];
    struct v4l2_buffer buf;
    struct v4l2_plane planes[1];
    for (int i = 0; i < 2; i++) {
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        buf.m.planes = planes;
        buf.length = 1;
        if (ioctl(cam_fd, VIDIOC_QUERYBUF, &buf) < 0) { perror("QUERYBUF"); return -1; }
        camera_buf[i] = mmap(NULL, buf_len, PROT_READ|PROT_WRITE, MAP_SHARED, cam_fd, buf.m.planes[0].m.mem_offset);
        if (camera_buf[i] == MAP_FAILED) { perror("mmap"); return -1; }
        if (ioctl(cam_fd, VIDIOC_QBUF, &buf) < 0) { perror("QBUF"); return -1; }
    }

    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    if (ioctl(cam_fd, VIDIOC_STREAMON, &type) < 0) { perror("STREAMON"); return -1; }

    // 分配转换缓冲区
    unsigned char *nv12_buf = malloc(buf_len);
    unsigned char *rgb_buf = malloc(FRAME_SIZE);
    if (!nv12_buf || !rgb_buf) { perror("malloc"); return -1; }

    printf("Camera started, resolution %dx%d\n", CAM_WIDTH, CAM_HEIGHT);

    while (running) {
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.m.planes = planes;
        buf.length = 1;
        if (ioctl(cam_fd, VIDIOC_DQBUF, &buf) < 0) { perror("DQBUF"); break; }
        memcpy(nv12_buf, camera_buf[buf.index], buf.m.planes[0].bytesused);
        if (ioctl(cam_fd, VIDIOC_QBUF, &buf) < 0) { perror("QBUF"); break; }

        nv12_to_rgb(nv12_buf, CAM_WIDTH, CAM_HEIGHT, stride_y, stride_uv, rgb_buf);

        shared_frame->width = CAM_WIDTH;
        shared_frame->height = CAM_HEIGHT;
        shared_frame->frame_count++;
        shared_frame->timestamp = 0;
        memcpy(shared_frame->rgb_data, rgb_buf, FRAME_SIZE);

        static int cnt = 0;
        if (cnt++ % 30 == 0)
            printf("Produced frame %d\n", shared_frame->frame_count);
        usleep(33333);
    }

    free(nv12_buf);
    free(rgb_buf);
    for (int i = 0; i < 2; i++) munmap(camera_buf[i], buf_len);
    close(cam_fd);
    shmdt(shared_frame);
    shmctl(shm_id, IPC_RMID, NULL);
    return 0;
}