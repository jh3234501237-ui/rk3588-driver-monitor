#include "driver_state_engine_simple.h"
#include <string.h>
#include <stdio.h>

static int clamp_int(int v, int min_v, int max_v)
{
    if (v < min_v) return min_v;
    if (v > max_v) return max_v;
    return v;
}

void dse_simple_init(DriverStateEngineSimple *dse)
{
    if (!dse) return;
    memset(dse, 0, sizeof(DriverStateEngineSimple));
}

static void dse_add_blink(DriverStateEngineSimple *dse, uint64_t ts_ms)
{
    if (!dse) return;
    dse->blink_times[dse->blink_index] = ts_ms;
    dse->blink_index++;
    if (dse->blink_index >= DSE_BLINK_MAX) dse->blink_index = 0;
    if (dse->blink_count < DSE_BLINK_MAX) dse->blink_count++;
}

static int dse_count_recent_blinks(DriverStateEngineSimple *dse, uint64_t now_ms)
{
    if (!dse) return 0;
    uint64_t start_ms = (now_ms > 60000ULL) ? (now_ms - 60000ULL) : 0;
    int count = 0;
    for (int i = 0; i < dse->blink_count; i++) {
        if (dse->blink_times[i] >= start_ms && dse->blink_times[i] <= now_ms)
            count++;
    }
    return count;
}

static void dse_make_output(int score, int perclos, int blink_freq,
                            int eye_closed, int head_down, DseOutput *out)
{
    if (!out) return;
    score = clamp_int(score, 0, 100);
    out->fatigue_score = score;
    out->perclos = clamp_int(perclos, 0, 100);
    out->blink_freq = clamp_int(blink_freq, 0, 80);
    out->yawn_count = 0;

    if (score <= 30) {
        out->driver_state = DSE_STATE_NORMAL;
        out->state_text = "Normal";
    } else if (score <= 60) {
        out->driver_state = DSE_STATE_MILD;
        out->state_text = "Mild Risk";
    } else if (score <= 80) {
        out->driver_state = DSE_STATE_DROWSY;
        out->state_text = "Drowsy";
    } else {
        out->driver_state = DSE_STATE_DANGEROUS;
        out->state_text = "Dangerous";
    }

    if (eye_closed)
        out->eye_text = "Closed";
    else if (head_down)
        out->eye_text = "Tired";
    else
        out->eye_text = "Normal";
}

void dse_simple_update(DriverStateEngineSimple *dse,
                       const BehaviorResult *res,
                       uint64_t now_ms,
                       DseOutput *out)
{
    if (!dse || !out) return;
    if (!res) {
        dse_make_output(0, 0, 0, 0, 0, out);
        return;
    }

    int head_down   = res->head_down ? 1 : 0;
    int phone_call  = res->phone_call ? 1 : 0;
    int smoking     = res->smoking ? 1 : 0;
    int eye_closed  = res->eye_closed ? 1 : 0;
    int face_lost   = (res->face_detected == 0) ? 1 : 0;
    int pose_lost   = (res->pose_detected == 0) ? 1 : 0;

    // 存入滑动窗口
    DseSample *s = &dse->samples[dse->index];
    s->valid = 1;
    s->head_down = head_down;
    s->phone_call = phone_call;
    s->smoking = smoking;
    s->eye_closed = eye_closed;
    s->face_lost = face_lost;
    s->pose_lost = pose_lost;
    s->ts_ms = now_ms;

    dse->index++;
    if (dse->index >= DSE_WINDOW_SIZE) dse->index = 0;
    if (dse->count < DSE_WINDOW_SIZE) dse->count++;

    // 连续计数（用于风险加速）
    if (head_down) dse->head_down_cnt++; else dse->head_down_cnt = 0;
    if (phone_call) dse->phone_call_cnt++; else dse->phone_call_cnt = 0;
    if (smoking) dse->smoking_cnt++; else dse->smoking_cnt = 0;
    if (face_lost) dse->face_lost_cnt++; else dse->face_lost_cnt = 0;
    if (eye_closed) dse->eye_closed_cnt++; else dse->eye_closed_cnt = 0;

    // 眨眼检测（基于 eye_closed 变化）
    if (!dse->in_closed_segment && eye_closed) {
        dse->in_closed_segment = 1;
        dse->eye_close_start_ms = now_ms;
    }
    if (dse->in_closed_segment && !eye_closed) {
        uint64_t dur = now_ms - dse->eye_close_start_ms;
        if (dur >= 80 && dur <= 800) {
            dse_add_blink(dse, now_ms);
        }
        dse->in_closed_segment = 0;
    }

    // 风险值变化（delta）
    int delta = 0;
    if (head_down) {
        delta += 5;
        if (dse->head_down_cnt >= 6) delta += 4;
        if (dse->head_down_cnt >= 12) delta += 5;
    }
    if (phone_call) {
        delta += 6;
        if (dse->phone_call_cnt >= 4) delta += 4;
        if (dse->phone_call_cnt >= 10) delta += 5;
    }
    if (smoking) {
        delta += 4;
        if (dse->smoking_cnt >= 4) delta += 3;
    }
    if (face_lost) {
        delta += 4;
        if (dse->face_lost_cnt >= 6) delta += 5;
    }
    if (pose_lost) delta += 2;
    if (eye_closed) {
        delta += 6;
        if (dse->eye_closed_cnt >= 4) delta += 6;
        if (dse->eye_closed_cnt >= 8) delta += 8;
    }

    int abnormal = head_down || phone_call || smoking || face_lost || pose_lost || eye_closed;
    if (abnormal)
        dse->risk_score += delta;
    else
        dse->risk_score -= 5;   // 正常时风险下降
    dse->risk_score = clamp_int(dse->risk_score, 0, 100);

    // 计算 PERCLOS（最近 30 秒闭眼比例）
    uint64_t perclos_start = (now_ms > 30000ULL) ? (now_ms - 30000ULL) : 0;
    int valid_cnt = 0, closed_cnt = 0;
    for (int i = 0; i < dse->count; i++) {
        DseSample *p = &dse->samples[i];
        if (!p->valid) continue;
        if (p->ts_ms < perclos_start || p->ts_ms > now_ms) continue;
        valid_cnt++;
        if (p->eye_closed) closed_cnt++;
    }
    int perclos = (valid_cnt > 0) ? (closed_cnt * 100 / valid_cnt) : 0;

    // 眨眼频率（最近 60 秒）
    int blink_freq = dse_count_recent_blinks(dse, now_ms);
    // 如果启动未满 60 秒，估算每分钟眨眼次数
    uint64_t oldest = now_ms;
    for (int i = 0; i < dse->count; i++) {
        DseSample *p = &dse->samples[i];
        if (p->valid && p->ts_ms < oldest) oldest = p->ts_ms;
    }
    uint64_t run_ms = now_ms - oldest;
    if (run_ms >= 1000 && run_ms < 60000) {
        blink_freq = (int)(blink_freq * 60000ULL / run_ms);
    }

    dse->last_update_ms = now_ms;
    dse_make_output(dse->risk_score, perclos, blink_freq, eye_closed, head_down, out);
}