#ifndef DRIVER_STATE_ENGINE_SIMPLE_H
#define DRIVER_STATE_ENGINE_SIMPLE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "shared_mem.h"   // 会用到 BehaviorResult

#define DSE_STATE_NORMAL      0
#define DSE_STATE_MILD        1
#define DSE_STATE_DROWSY      2
#define DSE_STATE_DANGEROUS   3

#define DSE_WINDOW_SIZE       120      // 约 60 帧 (假设 30fps, 2 秒)
#define DSE_BLINK_MAX         120

typedef struct {
    int valid;
    int head_down;
    int phone_call;
    int smoking;
    int eye_closed;
    int face_lost;
    int pose_lost;
    uint64_t ts_ms;
} DseSample;

typedef struct {
    int fatigue_score;
    int driver_state;
    int perclos;
    int blink_freq;
    int yawn_count;
    const char *state_text;
    const char *eye_text;
} DseOutput;

typedef struct {
    DseSample samples[DSE_WINDOW_SIZE];
    int index;
    int count;

    int risk_score;

    int head_down_cnt;
    int phone_call_cnt;
    int smoking_cnt;
    int face_lost_cnt;
    int eye_closed_cnt;

    int in_closed_segment;
    uint64_t eye_close_start_ms;

    uint64_t blink_times[DSE_BLINK_MAX];
    int blink_index;
    int blink_count;

    uint64_t last_update_ms;
} DriverStateEngineSimple;

void dse_simple_init(DriverStateEngineSimple *dse);
void dse_simple_update(DriverStateEngineSimple *dse,
                       const BehaviorResult *res,
                       uint64_t now_ms,
                       DseOutput *out);

#ifdef __cplusplus
}
#endif

#endif