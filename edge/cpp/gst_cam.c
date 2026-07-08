// #define _GNU_SOURCE
// #include <stdio.h>
// #include <stdlib.h>
// #include <stdint.h>
// #include <string.h>
// #include <unistd.h>
// #include <signal.h>
// #include <sys/shm.h>
// #include <gst/gst.h>
// #include <gst/app/gstappsink.h>

// #define SHM_KEY          0x1234
// #define CAM_WIDTH        1280
// #define CAM_HEIGHT       720
// #define FRAME_SIZE       (CAM_WIDTH * CAM_HEIGHT * 3)

// typedef struct {
//     int width;
//     int height;
//     int frame_count;
//     uint64_t timestamp;
//     unsigned char rgb_data[FRAME_SIZE];
// } SharedFrame;

// static int shm_id = -1;
// static SharedFrame *shared_frame = NULL;
// static volatile int running = 1;

// void sigint_handler(int sig) { running = 0; }

// static GstFlowReturn on_new_sample(GstElement *sink, gpointer user_data) {
//     GstSample *sample = gst_app_sink_pull_sample(GST_APP_SINK(sink));
//     if (!sample) return GST_FLOW_ERROR;
//     GstBuffer *buffer = gst_sample_get_buffer(sample);
//     GstMapInfo map;
//     if (gst_buffer_map(buffer, &map, GST_MAP_READ)) {
//         if (shared_frame && map.size >= FRAME_SIZE) {
//             shared_frame->width = CAM_WIDTH;
//             shared_frame->height = CAM_HEIGHT;
//             shared_frame->frame_count++;
//             shared_frame->timestamp = g_get_real_time();
//             memcpy(shared_frame->rgb_data, map.data, FRAME_SIZE);
//         }
//         gst_buffer_unmap(buffer, &map);
//     }
//     gst_sample_unref(sample);
//     return GST_FLOW_OK;
// }

// int main(int argc, char *argv[]) {
//     signal(SIGINT, sigint_handler);
//     gst_init(&argc, &argv);

//     shm_id = shmget(SHM_KEY, sizeof(SharedFrame), IPC_CREAT | 0666);
//     if (shm_id < 0) { perror("shmget"); return -1; }
//     shared_frame = (SharedFrame*)shmat(shm_id, NULL, 0);
//     if (shared_frame == (void*)-1) { perror("shmat"); return -1; }
//     memset(shared_frame, 0, sizeof(SharedFrame));

//     char pipeline[512];
//     snprintf(pipeline, sizeof(pipeline),
//         "v4l2src device=/dev/video11 ! "
//         "video/x-raw,format=NV12,width=%d,height=%d,framerate=30/1 ! "
//         "rkisp ! videoconvert ! video/x-raw,format=RGB ! "
//         "appsink name=sink emit-signals=true max-buffers=1 drop=true",
//         CAM_WIDTH, CAM_HEIGHT);

//     GError *error = NULL;
//     GstElement *pipeline_elem = gst_parse_launch(pipeline, &error);
//     if (!pipeline_elem) { g_printerr("Parse error: %s\n", error->message); return -1; }

//     GstElement *sink = gst_bin_get_by_name(GST_BIN(pipeline_elem), "sink");
//     g_signal_connect(sink, "new-sample", G_CALLBACK(on_new_sample), NULL);

//     gst_element_set_state(pipeline_elem, GST_STATE_PLAYING);
//     printf("GStreamer producer started, resolution %dx%d\n", CAM_WIDTH, CAM_HEIGHT);

//     while (running) g_usleep(100000);

//     gst_element_set_state(pipeline_elem, GST_STATE_NULL);
//     gst_object_unref(pipeline_elem);
//     shmdt(shared_frame);
//     shmctl(shm_id, IPC_RMID, NULL);
//     return 0;
// }

// #include <stdio.h>
// #include <stdint.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>
// #include <signal.h>
// #include <sys/shm.h>
// #include <gst/gst.h>
// #include <gst/app/gstappsink.h>

// #define SHM_KEY          0x1234
// #define CAM_WIDTH        1280
// #define CAM_HEIGHT       720
// #define FRAME_SIZE       (CAM_WIDTH * CAM_HEIGHT * 3)

// typedef struct {
//     int width;
//     int height;
//     int frame_count;
//     uint64_t timestamp;
//     unsigned char rgb_data[FRAME_SIZE];
// } SharedFrame;

// static int shm_id = -1;
// static SharedFrame *shared_frame = NULL;
// static volatile int running = 1;

// void sigint_handler(int sig) { running = 0; }

// static GstFlowReturn on_new_sample(GstElement *sink, gpointer user_data) {
//     GstSample *sample = gst_app_sink_pull_sample(GST_APP_SINK(sink));
//     if (!sample) return GST_FLOW_ERROR;
//     GstBuffer *buffer = gst_sample_get_buffer(sample);
//     GstMapInfo map;
//     if (gst_buffer_map(buffer, &map, GST_MAP_READ)) {
//         if (shared_frame && map.size >= FRAME_SIZE) {
//             shared_frame->width = CAM_WIDTH;
//             shared_frame->height = CAM_HEIGHT;
//             shared_frame->frame_count++;
//             shared_frame->timestamp = g_get_real_time();
//             memcpy(shared_frame->rgb_data, map.data, FRAME_SIZE);
//         }
//         gst_buffer_unmap(buffer, &map);
//     }
//     gst_sample_unref(sample);
//     return GST_FLOW_OK;
// }

// int main(int argc, char *argv[]) {
//     signal(SIGINT, sigint_handler);
//     gst_init(&argc, &argv);

//     shm_id = shmget(SHM_KEY, sizeof(SharedFrame), IPC_CREAT | 0666);
//     if (shm_id < 0) { perror("shmget"); return -1; }
//     shared_frame = (SharedFrame*)shmat(shm_id, NULL, 0);
//     if (shared_frame == (void*)-1) { perror("shmat"); return -1; }
//     memset(shared_frame, 0, sizeof(SharedFrame));

//     // 使用 videoconvert 代替 rkisp（兼容性最好）
//     char pipeline[512];
//     snprintf(pipeline, sizeof(pipeline),
//         "v4l2src device=/dev/video11 ! "
//         "video/x-raw,format=NV12,width=%d,height=%d,framerate=30/1 ! "
//         "videoconvert ! video/x-raw,format=RGB ! "
//         "appsink name=sink emit-signals=true max-buffers=1 drop=true",
//         CAM_WIDTH, CAM_HEIGHT);

//     GError *error = NULL;
//     GstElement *pipeline_elem = gst_parse_launch(pipeline, &error);
//     if (!pipeline_elem) { g_printerr("Parse error: %s\n", error->message); return -1; }

//     GstElement *sink = gst_bin_get_by_name(GST_BIN(pipeline_elem), "sink");
//     g_signal_connect(sink, "new-sample", G_CALLBACK(on_new_sample), NULL);

//     gst_element_set_state(pipeline_elem, GST_STATE_PLAYING);
//     printf("GStreamer producer started, resolution %dx%d\n", CAM_WIDTH, CAM_HEIGHT);

//     while (running) g_usleep(100000);

//     gst_element_set_state(pipeline_elem, GST_STATE_NULL);
//     gst_object_unref(pipeline_elem);
//     shmdt(shared_frame);
//     // shmctl(shm_id, IPC_RMID, NULL);
//     return 0;
// }

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/shm.h>
#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <rga.h>
#include <im2d.h>

#define SHM_KEY          0x1234
#define CAM_WIDTH        1280
#define CAM_HEIGHT       720
#define FRAME_SIZE       (CAM_WIDTH * CAM_HEIGHT * 3)   // RGB888

typedef struct {
    int width;
    int height;
    int frame_count;
    uint64_t timestamp;
    unsigned char rgb_data[FRAME_SIZE];
} SharedFrame;

static int shm_id = -1;
static SharedFrame *shared_frame = NULL;
static volatile int running = 1;

void sigint_handler(int sig) { running = 0; }

static GstFlowReturn on_new_sample(GstElement *sink, gpointer user_data) {
    GstSample *sample = gst_app_sink_pull_sample(GST_APP_SINK(sink));
    if (!sample) return GST_FLOW_ERROR;

    GstBuffer *buffer = gst_sample_get_buffer(sample);
    GstMapInfo map;
    if (gst_buffer_map(buffer, &map, GST_MAP_READ)) {
        if (shared_frame && map.size >= (CAM_WIDTH * CAM_HEIGHT * 3 / 2)) {
            shared_frame->width = CAM_WIDTH;
            shared_frame->height = CAM_HEIGHT;
            shared_frame->frame_count++;
            shared_frame->timestamp = g_get_real_time();

            // ---------- 使用 RGA 硬件加速 NV12 -> RGB ----------
            rga_buffer_t src, dst;
            int ret = -1;

            // 封装源图像（NV12）
            src = wrapbuffer_virtualaddr((void*)map.data, CAM_WIDTH, CAM_HEIGHT,
                                         RK_FORMAT_YCbCr_420_SP);
            // 封装目标图像（RGB888）
            dst = wrapbuffer_virtualaddr((void*)shared_frame->rgb_data, CAM_WIDTH, CAM_HEIGHT,
                                         RK_FORMAT_RGB_888);

            ret = imcopy(src, dst);
            if (ret != 0) {
                // RGA 转换失败，回退到 CPU 软件转换（仅作保险）
                fprintf(stderr, "RGA NV12->RGB failed, fallback to CPU\n");
                // 简单的 CPU 转换（NV12 到 RGB，仅供参考，实际颜色可能有偏差）
                unsigned char *nv12 = (unsigned char*)map.data;
                unsigned char *rgb = shared_frame->rgb_data;
                int y, x;
                for (y = 0; y < CAM_HEIGHT; y++) {
                    for (x = 0; x < CAM_WIDTH; x++) {
                        int Y = nv12[y * CAM_WIDTH + x];
                        int U = nv12[CAM_WIDTH * CAM_HEIGHT + (y/2) * CAM_WIDTH + (x/2)];
                        int V = nv12[CAM_WIDTH * CAM_HEIGHT + (y/2) * CAM_WIDTH + (x/2) + CAM_WIDTH/2];
                        int R = Y + 1.402 * (V - 128);
                        int G = Y - 0.344 * (U - 128) - 0.714 * (V - 128);
                        int B = Y + 1.772 * (U - 128);
                        R = R<0?0:(R>255?255:R);
                        G = G<0?0:(G>255?255:G);
                        B = B<0?0:(B>255?255:B);
                        rgb[(y * CAM_WIDTH + x) * 3 + 0] = R;
                        rgb[(y * CAM_WIDTH + x) * 3 + 1] = G;
                        rgb[(y * CAM_WIDTH + x) * 3 + 2] = B;
                    }
                }
            }
        }
        gst_buffer_unmap(buffer, &map);
    }
    gst_sample_unref(sample);
    return GST_FLOW_OK;
}

int main(int argc, char *argv[]) {
    signal(SIGINT, sigint_handler);
    gst_init(&argc, &argv);

    shm_id = shmget(SHM_KEY, sizeof(SharedFrame), IPC_CREAT | 0666);
    if (shm_id < 0) { perror("shmget"); return -1; }
    shared_frame = (SharedFrame*)shmat(shm_id, NULL, 0);
    if (shared_frame == (void*)-1) { perror("shmat"); return -1; }
    memset(shared_frame, 0, sizeof(SharedFrame));

    // Pipeline: 直接输出 NV12，无 videoconvert
    char pipeline[512];
    snprintf(pipeline, sizeof(pipeline),
        "v4l2src device=/dev/video11 ! "
        "video/x-raw,format=NV12,width=%d,height=%d,framerate=30/1 ! "
        "appsink name=sink emit-signals=true max-buffers=1 drop=true",
        CAM_WIDTH, CAM_HEIGHT);

    GError *error = NULL;
    GstElement *pipeline_elem = gst_parse_launch(pipeline, &error);
    if (!pipeline_elem) { g_printerr("Parse error: %s\n", error->message); return -1; }

    GstElement *sink = gst_bin_get_by_name(GST_BIN(pipeline_elem), "sink");
    g_signal_connect(sink, "new-sample", G_CALLBACK(on_new_sample), NULL);

    gst_element_set_state(pipeline_elem, GST_STATE_PLAYING);
    printf("GStreamer producer started (RGA hardware conversion), resolution %dx%d\n", CAM_WIDTH, CAM_HEIGHT);

    while (running) g_usleep(100000);

    gst_element_set_state(pipeline_elem, GST_STATE_NULL);
    gst_object_unref(pipeline_elem);
    shmdt(shared_frame);
    // 不删除共享内存，留给其他进程清理
    return 0;
}