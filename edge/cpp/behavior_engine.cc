#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/shm.h>
#include <time.h>
#include <math.h>
#include <stdint.h>

#include "yolov8-pose.h"
#include "retinaface.h"
#include "image_utils.h"
#include "shared_mem.h"

/*
 * behavior_engine_final.cc
 *
 * Final integrated version:
 * 1. YOLOv8-Pose + RetinaFace dual-model inference.
 * 2. Real inference metrics written to BehaviorResult.
 * 3. Head Down, Phone Risk, Mouth Risk, Head Yaw, Hands Off, Eye Risk Estimate.
 * 4. Smoking is now treated as Mouth Risk internally.
 *
 * IMPORTANT:
 * shared_mem.h must add the new fields listed in my message.
 */

#define WINDOW_SIZE         15
#define SMOOTH_ALPHA        0.30f
#define KP_CONF_TH          0.35f
#define FACE_CONF_TH        0.30f
#define STABILITY_WIN       8

static volatile int running = 1;

void sigint_handler(int sig)
{
    (void)sig;
    running = 0;
}

/* ================================================================
 * Basic math helpers
 * ================================================================ */
static float clamp_float(float v, float lo, float hi)
{
    return v < lo ? lo : (v > hi ? hi : v);
}

static float distance2d(float x1, float y1, float x2, float y2)
{
    float dx = x1 - x2;
    float dy = y1 - y2;
    return sqrtf(dx * dx + dy * dy);
}

static float point_to_segment_dist(float px, float py,
                                   float ax, float ay,
                                   float bx, float by)
{
    float vx = bx - ax;
    float vy = by - ay;
    float wx = px - ax;
    float wy = py - ay;

    float c1 = vx * wx + vy * wy;
    if (c1 <= 0.0f) {
        return distance2d(px, py, ax, ay);
    }

    float c2 = vx * vx + vy * vy;
    if (c2 <= 1e-6f) {
        return distance2d(px, py, ax, ay);
    }

    if (c2 <= c1) {
        return distance2d(px, py, bx, by);
    }

    float t = c1 / c2;
    float proj_x = ax + t * vx;
    float proj_y = ay + t * vy;

    return distance2d(px, py, proj_x, proj_y);
}

static float score_by_dist_near(float d, float near_v, float far_v)
{
    if (d <= near_v) return 1.0f;
    if (d >= far_v)  return 0.0f;
    return (far_v - d) / (far_v - near_v);
}

static float score_by_value_high(float v, float low, float high)
{
    if (v <= low)  return 0.0f;
    if (v >= high) return 1.0f;
    return (v - low) / (high - low);
}

static double get_time_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1000.0 + (double)ts.tv_nsec / 1000000.0;
}

static float avg_window(const float *buf, int n)
{
    float s = 0.0f;
    for (int i = 0; i < n; i++) s += buf[i];
    return s / (float)n;
}

static int count_window_above(const float *buf, int n, float th)
{
    int c = 0;
    for (int i = 0; i < n; i++) {
        if (buf[i] >= th) c++;
    }
    return c;
}

static float update_behavior_accum(float current,
                                   int evidence,
                                   float dt_sec,
                                   float rise_rate,
                                   float fall_rate,
                                   float max_value)
{
    if (evidence) {
        current += dt_sec * rise_rate;
    } else {
        current -= dt_sec * fall_rate;
    }

    return clamp_float(current, 0.0f, max_value);
}

static int box_area_retina(const retinaface_object_t *obj)
{
    if (!obj) return 0;

    int w = obj->box.right - obj->box.left;
    int h = obj->box.bottom - obj->box.top;

    if (w < 0) w = 0;
    if (h < 0) h = 0;

    return w * h;
}

/* ================================================================
 * Wrist stability tracker
 * Phone call: hand tends to stay near ear.
 * Mouth risk: hand may move near mouth, so stability is not required.
 * ================================================================ */
typedef struct {
    float hist_x[STABILITY_WIN];
    float hist_y[STABILITY_WIN];
    int idx;
    int filled;
} WristTracker;

static void wrist_tracker_push(WristTracker *t, float x, float y)
{
    if (!t) return;

    t->hist_x[t->idx] = x;
    t->hist_y[t->idx] = y;
    t->idx = (t->idx + 1) % STABILITY_WIN;

    if (t->filled < STABILITY_WIN) {
        t->filled++;
    }
}

static float wrist_stability_score(const WristTracker *t, float body_scale)
{
    if (!t || t->filled < 3 || body_scale < 1.0f) {
        return 0.5f;
    }

    float sum_move = 0.0f;
    int cnt = 0;
    int n = t->filled;

    for (int i = 1; i < n; i++) {
        int cur  = (t->idx - 1 - (i - 1) + STABILITY_WIN * 2) % STABILITY_WIN;
        int prev = (t->idx - 1 - i + STABILITY_WIN * 2) % STABILITY_WIN;

        float dx = t->hist_x[cur] - t->hist_x[prev];
        float dy = t->hist_y[cur] - t->hist_y[prev];

        sum_move += sqrtf(dx * dx + dy * dy);
        cnt++;
    }

    if (cnt == 0) {
        return 0.5f;
    }

    float avg_move = (sum_move / (float)cnt) / body_scale;

    /*
     * avg_move < 0.02 body_scale -> stable.
     * avg_move > 0.10 body_scale -> moving.
     */
    return 1.0f - clamp_float((avg_move - 0.02f) / 0.08f, 0.0f, 1.0f);
}

/* ================================================================
 * Engine state
 * ================================================================ */
typedef struct {
    float prev_keypoints[17][2];
    int init;

    float smoothed_pose_pitch;
    float smoothed_face_pitch;
    float smoothed_yaw;

    float smoothed_mouth_x;
    float smoothed_mouth_y;
    int mouth_init;

    WristTracker left_wrist_tracker;
    WristTracker right_wrist_tracker;

    float phone_window[WINDOW_SIZE];
    float mouth_window[WINDOW_SIZE];
    float head_window[WINDOW_SIZE];
    float yaw_window[WINDOW_SIZE];
    float hands_window[WINDOW_SIZE];
    float eye_window[WINDOW_SIZE];

    int window_idx;
    int window_filled;

    float phone_accum;
    float mouth_accum;
    double last_logic_ms;

    int phone_state;
    int mouth_state;
    int head_state;
    int yaw_state;
    int hands_off_state;
} EngineState;

/* ================================================================
 * Face and pose selection
 * ================================================================ */
static void keep_largest_face(retinaface_result *face_res)
{
    if (!face_res || face_res->count <= 1) return;

    int max_idx = 0;
    int max_area = 0;

    for (int i = 0; i < face_res->count; i++) {
        int area = box_area_retina(&face_res->object[i]);
        if (area > max_area) {
            max_area = area;
            max_idx = i;
        }
    }

    if (max_idx != 0) {
        retinaface_object_t tmp = face_res->object[0];
        face_res->object[0] = face_res->object[max_idx];
        face_res->object[max_idx] = tmp;
    }

    face_res->count = 1;
}

/*
 * Match pose to driver face.
 * Nose-based matching is kept because it is simple and compatible with current output.
 * The margin uses face size instead of a fixed 30 px threshold.
 */
static int select_driver_pose(const object_detect_result_list *pose_res,
                              const retinaface_result *face_res)
{
    if (!pose_res || pose_res->count <= 0) return -1;

    if (face_res && face_res->count > 0) {
        const retinaface_object_t *df = &face_res->object[0];

        int fw = df->box.right - df->box.left;
        int fh = df->box.bottom - df->box.top;

        if (fw < 1) fw = 1;
        if (fh < 1) fh = 1;

        int mx = fw / 2;
        int my = fh / 2;

        for (int i = 0; i < pose_res->count; i++) {
            float nx = pose_res->results[i].keypoints[0][0];
            float ny = pose_res->results[i].keypoints[0][1];
            float nc = pose_res->results[i].keypoints[0][2];

            if (nc < 0.50f) continue;

            if (nx >= df->box.left - mx &&
                nx <= df->box.right + mx &&
                ny >= df->box.top - my &&
                ny <= df->box.bottom + my) {
                return i;
            }
        }
    }

    return 0;
}

static float compute_body_scale(float shoulder_width,
                                const retinaface_result *face_res,
                                int frame_w)
{
    if (shoulder_width > 30.0f) {
        return shoulder_width;
    }

    if (face_res && face_res->count > 0) {
        float fw = (float)(face_res->object[0].box.right -
                           face_res->object[0].box.left);
        if (fw > 20.0f) {
            return fw * 2.2f;
        }
    }

    return (frame_w > 0) ? (float)frame_w * 0.25f : 200.0f;
}

/* ================================================================
 * Phone Risk / Mouth Risk
 *
 * Strategy:
 * - Phone: wrist close to ear/face-side, height close to ear, and relatively stable.
 * - Mouth Risk: wrist close to mouth and height close to mouth.
 * - We no longer claim precise cigarette detection here.
 * ================================================================ */
static void compute_phone_mouth_scores(
    float lw_x, float lw_y, float lw_conf,
    float rw_x, float rw_y, float rw_conf,

    float lelb_x, float lelb_y, float lelb_conf,
    float relb_x, float relb_y, float relb_conf,

    float le_x, float le_y, float le_conf,
    float re_x, float re_y, float re_conf,
    float mouth_x, float mouth_y, int mouth_valid,
    int face_valid,
    float face_left, float face_top, float face_right, float face_bottom,
    float body_scale,
    float left_stability,
    float right_stability,
    float *phone_score_out,
    float *mouth_score_out)
{
    const float WRIST_CONF_TH = 0.12f;

    const float PHONE_NEAR = 0.38f;
    const float PHONE_FAR  = 0.80f;

    const float MOUTH_NEAR = 0.20f;
    const float MOUTH_FAR  = 0.55f;

    const float PHONE_HEIGHT_TOL = 0.32f;
    const float MOUTH_HEIGHT_TOL = 0.24f;

    if (body_scale < 1.0f) body_scale = 1.0f;

    int lw_ok = (lw_conf >= WRIST_CONF_TH && lw_x > 1.0f && lw_y > 1.0f);
    int rw_ok = (rw_conf >= WRIST_CONF_TH && rw_x > 1.0f && rw_y > 1.0f);

    int lelb_ok = (lelb_conf >= 0.12f && lelb_x > 1.0f && lelb_y > 1.0f);
    int relb_ok = (relb_conf >= 0.12f && relb_x > 1.0f && relb_y > 1.0f);

    int le_ok = (le_conf >= 0.15f && le_x > 1.0f && le_y > 1.0f);
    int re_ok = (re_conf >= 0.15f && re_x > 1.0f && re_y > 1.0f);

    float face_w = face_right - face_left;
    float face_h = face_bottom - face_top;

    if (face_valid && face_w > 10.0f && face_h > 10.0f) {
        if (!le_ok) {
            le_x = face_left + face_w * 0.05f;
            le_y = face_top + face_h * 0.45f;
            le_ok = 1;
        }

        if (!re_ok) {
            re_x = face_right - face_w * 0.05f;
            re_y = face_top + face_h * 0.45f;
            re_ok = 1;
        }
    }

    float left_phone = 0.0f;
    float right_phone = 0.0f;
    float left_mouth = 0.0f;
    float right_mouth = 0.0f;

    float left_ear_dist = 999.0f;
    float right_ear_dist = 999.0f;
    float left_mouth_dist = 999.0f;
    float right_mouth_dist = 999.0f;

    if (lw_ok && le_ok) {
        left_ear_dist = distance2d(lw_x, lw_y, le_x, le_y) / body_scale;
        float dist_s = score_by_dist_near(left_ear_dist, PHONE_NEAR, PHONE_FAR);

        float dy_ear = fabsf(lw_y - le_y) / body_scale;
        float height_s = 1.0f - clamp_float(
            (dy_ear - PHONE_HEIGHT_TOL * 0.30f) / (PHONE_HEIGHT_TOL * 0.70f),
            0.0f,
            1.0f
        );

        left_phone = dist_s * height_s * (0.35f + 0.65f * left_stability);
    }

    if (rw_ok && re_ok) {
        right_ear_dist = distance2d(rw_x, rw_y, re_x, re_y) / body_scale;
        float dist_s = score_by_dist_near(right_ear_dist, PHONE_NEAR, PHONE_FAR);

        float dy_ear = fabsf(rw_y - re_y) / body_scale;
        float height_s = 1.0f - clamp_float(
            (dy_ear - PHONE_HEIGHT_TOL * 0.30f) / (PHONE_HEIGHT_TOL * 0.70f),
            0.0f,
            1.0f
        );

        right_phone = dist_s * height_s * (0.35f + 0.65f * right_stability);
    }

    /*
     * Face-side fallback: keep phone responsive, but only when wrist height is close
     * to the virtual ear height. This avoids stealing most mouth actions.
     */
    if (face_valid && face_w > 10.0f && face_h > 10.0f) {
        float ear_y = face_top + face_h * 0.45f;
        float y_top = face_top - face_h * 0.25f;
        float y_bottom = face_bottom + face_h * 0.40f;

        float left_zone_x1 = face_left - face_w * 1.10f;
        float left_zone_x2 = face_left + face_w * 0.35f;

        float right_zone_x1 = face_right - face_w * 0.35f;
        float right_zone_x2 = face_right + face_w * 1.10f;

        if (lw_ok &&
            lw_x >= left_zone_x1 && lw_x <= left_zone_x2 &&
            lw_y >= y_top && lw_y <= y_bottom) {
            float dy = fabsf(lw_y - ear_y) / body_scale;
            float h = 1.0f - clamp_float((dy - 0.06f) / 0.22f, 0.0f, 1.0f);
            left_phone += 0.35f * h * (0.50f + 0.50f * left_stability);
        }

        if (rw_ok &&
            rw_x >= right_zone_x1 && rw_x <= right_zone_x2 &&
            rw_y >= y_top && rw_y <= y_bottom) {
            float dy = fabsf(rw_y - ear_y) / body_scale;
            float h = 1.0f - clamp_float((dy - 0.06f) / 0.22f, 0.0f, 1.0f);
            right_phone += 0.35f * h * (0.50f + 0.50f * right_stability);
        }
    }

    if (lw_ok && mouth_valid) {
        left_mouth_dist = distance2d(lw_x, lw_y, mouth_x, mouth_y) / body_scale;
        float wrist_dist_s = score_by_dist_near(left_mouth_dist, 0.20f, 0.62f);

        float dy_mouth = fabsf(lw_y - mouth_y) / body_scale;
        float height_s = 1.0f - clamp_float(
            (dy_mouth - 0.08f) / 0.30f,
            0.0f,
            1.0f
        );

    float seg_s = 0.0f;

    if (lelb_ok) {
        float d_seg = point_to_segment_dist(mouth_x, mouth_y,
                                            lelb_x, lelb_y,
                                            lw_x, lw_y) / body_scale;

        seg_s = score_by_dist_near(d_seg, 0.04f, 0.24f);
    }

    //left_mouth = fmaxf(wrist_dist_s * height_s, seg_s * 0.95f);
    float mouth_base = wrist_dist_s * height_s;

    /*
    * Elbow-free mouth fallback:
    * 当肘关节不可见时，只要手腕靠近嘴部区域，也允许触发 Mouth Risk。
    * 这样身体靠后、肘部出画面时，不会完全失效。
    */
    float lower_face_s = 0.0f;

    if (face_valid && face_w > 10.0f && face_h > 10.0f) {
        float mouth_zone_x1 = face_left  - face_w * 0.55f;
        float mouth_zone_x2 = face_right + face_w * 0.55f;
        float mouth_zone_y1 = mouth_y    - face_h * 0.35f;
        float mouth_zone_y2 = mouth_y    + face_h * 0.55f;

        if (lw_x >= mouth_zone_x1 && lw_x <= mouth_zone_x2 &&
            lw_y >= mouth_zone_y1 && lw_y <= mouth_zone_y2) {

            float d_norm = distance2d(lw_x, lw_y, mouth_x, mouth_y) / body_scale;
            lower_face_s = score_by_dist_near(d_norm, 0.18f, 0.58f);

            /*
            * 手越接近嘴部高度，分数越高。
            */
            lower_face_s *= height_s;
        }
    }

    left_mouth = fmaxf(mouth_base, seg_s * 0.95f);
    left_mouth = fmaxf(left_mouth, lower_face_s * 0.85f);

}

    if (rw_ok && mouth_valid) {
        right_mouth_dist = distance2d(rw_x, rw_y, mouth_x, mouth_y) / body_scale;
        float wrist_dist_s = score_by_dist_near(right_mouth_dist, 0.20f, 0.62f);

        float dy_mouth = fabsf(rw_y - mouth_y) / body_scale;
        float height_s = 1.0f - clamp_float(
            (dy_mouth - 0.08f) / 0.30f,
            0.0f,
            1.0f
        );

        float seg_s = 0.0f;

        if (relb_ok) {
            float d_seg = point_to_segment_dist(mouth_x, mouth_y,
                                                relb_x, relb_y,
                                                rw_x, rw_y) / body_scale;

            seg_s = score_by_dist_near(d_seg, 0.04f, 0.24f);
        }

        //right_mouth = fmaxf(wrist_dist_s * height_s, seg_s * 0.95f);
        float mouth_base = wrist_dist_s * height_s;

    /*
    * Elbow-free mouth fallback:
    * 右手同样不强依赖肘关节。
    */
    float lower_face_s = 0.0f;

    if (face_valid && face_w > 10.0f && face_h > 10.0f) {
        float mouth_zone_x1 = face_left  - face_w * 0.55f;
        float mouth_zone_x2 = face_right + face_w * 0.55f;
        float mouth_zone_y1 = mouth_y    - face_h * 0.35f;
        float mouth_zone_y2 = mouth_y    + face_h * 0.55f;

        if (rw_x >= mouth_zone_x1 && rw_x <= mouth_zone_x2 &&
            rw_y >= mouth_zone_y1 && rw_y <= mouth_zone_y2) {

            float d_norm = distance2d(rw_x, rw_y, mouth_x, mouth_y) / body_scale;
            lower_face_s = score_by_dist_near(d_norm, 0.18f, 0.58f);
            lower_face_s *= height_s;
        }
    }

    right_mouth = fmaxf(mouth_base, seg_s * 0.95f);
    right_mouth = fmaxf(right_mouth, lower_face_s * 0.85f);
    }

    float phone_score = (left_phone > right_phone) ? left_phone : right_phone;
    float mouth_score = (left_mouth > right_mouth) ? left_mouth : right_mouth;


        /*
     * ============================================================
     * Strict demo geometry gate: Phone vs Mouth
     *
     * Demo rule:
     * 1. Wrist under face box / lower center area -> Mouth Risk
     * 2. Wrist at left/right side of face box     -> Phone Risk
     * 3. Other areas                             -> suppress both
     *
     * This creates a clear boundary for live demo and avoids
     * phone/mouth jumping.
     * ============================================================
     */
    if (face_valid && face_w > 10.0f && face_h > 10.0f && mouth_valid) {
        int mouth_zone_hit = 0;
        int phone_zone_hit = 0;

        /*
        * Mouth zone:
        * 只允许人脸框正下方触发 Mouth Risk。
        * 不再覆盖人脸左右边，避免打电话动作被抢成 Mouth。
        */
        float mouth_x1 = face_left  + face_w * 0.10f;
        float mouth_x2 = face_right - face_w * 0.10f;
        float mouth_y1 = face_bottom - face_h * 0.12f;
        float mouth_y2 = face_bottom + face_h * 0.75f;

        /*
         * Phone side zones:
         * 人脸框左右两侧，接近耳朵/脸侧高度。
         * 适合演示打电话。
         */
        float phone_y1 = face_top    - face_h * 0.20f;
        float phone_y2 = face_bottom + face_h * 0.25f;

        float left_phone_x1  = face_left  - face_w * 1.15f;
        float left_phone_x2  = face_left  + face_w * 0.08f;

        float right_phone_x1 = face_right - face_w * 0.08f;
        float right_phone_x2 = face_right + face_w * 1.15f;

        /*
         * Left wrist region check.
         */
        if (lw_ok) {
            int lw_in_mouth =
                (lw_x >= mouth_x1 && lw_x <= mouth_x2 &&
                 lw_y >= mouth_y1 && lw_y <= mouth_y2);

            int lw_in_phone =
                ((lw_x >= left_phone_x1 && lw_x <= left_phone_x2) ||
                 (lw_x >= right_phone_x1 && lw_x <= right_phone_x2)) &&
                (lw_y >= phone_y1 && lw_y <= phone_y2);

            if (lw_in_mouth) {
                mouth_zone_hit = 1;
            }

            if (lw_in_phone) {
                phone_zone_hit = 1;
            }
        }

        /*
         * Right wrist region check.
         */
        if (rw_ok) {
            int rw_in_mouth =
                (rw_x >= mouth_x1 && rw_x <= mouth_x2 &&
                 rw_y >= mouth_y1 && rw_y <= mouth_y2);

            int rw_in_phone =
                ((rw_x >= left_phone_x1 && rw_x <= left_phone_x2) ||
                 (rw_x >= right_phone_x1 && rw_x <= right_phone_x2)) &&
                (rw_y >= phone_y1 && rw_y <= phone_y2);

            if (rw_in_mouth) {
                mouth_zone_hit = 1;
            }

            if (rw_in_phone) {
                phone_zone_hit = 1;
            }
        }

        /*
         * Strict priority:
         * - 如果在嘴部下方区域，Mouth 优先。
         * - 如果在脸侧区域，Phone 优先。
         * - 如果两个区域都没命中，两个风险都压低。
         *
         * 注意：mouth 和 phone 区域理论上有少量重叠。
         * 如果同时命中，用 y 位置判断：
         * 更靠下 -> Mouth；更靠上/脸侧 -> Phone。
         */
        if (mouth_zone_hit && !phone_zone_hit) {
            mouth_score = fmaxf(mouth_score, 0.68f);
            phone_score = 0.0f;
        }
        else if (phone_zone_hit && !mouth_zone_hit) {
            phone_score = fmaxf(phone_score, 0.68f);
            mouth_score = 0.0f;
        }
        else if (mouth_zone_hit && phone_zone_hit) {
            /*
             * 重叠区域：用最接近嘴的手腕 y 坐标判断。
             */
            float best_y = 999999.0f;

            if (lw_ok) {
                float d_l = distance2d(lw_x, lw_y, mouth_x, mouth_y);
                best_y = lw_y;
                if (rw_ok) {
                    float d_r = distance2d(rw_x, rw_y, mouth_x, mouth_y);
                    if (d_r < d_l) {
                        best_y = rw_y;
                    }
                }
            } else if (rw_ok) {
                best_y = rw_y;
            }

            if (best_y >= face_bottom - face_h * 0.10f) {
                mouth_score = fmaxf(mouth_score, 0.68f);
                phone_score = 0.0f;
            } else {
                phone_score = fmaxf(phone_score, 0.68f);
                mouth_score = 0.0f;
            }
        }
        else {
            /*
             * 不在人脸正下方，也不在人脸左右侧：
             * 不触发 Phone / Mouth。
             */
            phone_score *= 0.10f;
            mouth_score *= 0.10f;
        }
    }

    *phone_score_out = clamp_float(phone_score, 0.0f, 1.0f);
    *mouth_score_out = clamp_float(mouth_score, 0.0f, 1.0f);
}

/* ================================================================
 * Additional lightweight features from existing models
 * ================================================================ */
static void compute_face_yaw_eye_risk(
    const BehaviorResult *result,
    EngineState *eng,
    float head_score,
    float *yaw_score_out,
    int *yaw_state_out,
    float *eye_risk_out)
{
    float yaw_score = 0.0f;
    int yaw_state = 0;  /* 0 normal, 1 left, 2 right */

    if (result && result->face_detected && result->face_count > 0) {
        const FaceInfo *f = &result->faces[0];

        float face_w = (float)(f->right - f->left);
        if (face_w > 10.0f) {
            float left_eye_x = f->points[0][0];
            float right_eye_x = f->points[1][0];
            float nose_x = f->points[2][0];

            float eye_cx = (left_eye_x + right_eye_x) * 0.5f;
            float offset = (nose_x - eye_cx) / face_w;

            eng->smoothed_yaw = 0.75f * eng->smoothed_yaw + 0.25f * offset;

            float abs_yaw = fabsf(eng->smoothed_yaw);
            yaw_score = score_by_value_high(abs_yaw, 0.08f, 0.20f);

            if (yaw_score > 0.60f) {
                yaw_state = (eng->smoothed_yaw > 0.0f) ? 2 : 1;
            }
        }
    }

    /*
     * Eye Risk Estimate:
     * This is NOT true PERCLOS. It is a lightweight estimate using head-down
     * and yaw distraction because current models do not provide eyelid contours.
     */
    float eye_risk = clamp_float(0.75f * head_score + 0.25f * yaw_score, 0.0f, 1.0f);

    *yaw_score_out = yaw_score;
    *yaw_state_out = yaw_state;
    *eye_risk_out = eye_risk;
}

static float compute_hands_off_score(
    int pose_detected,
    float lw_x, float lw_y, float lw_conf,
    float rw_x, float rw_y, float rw_conf,
    int src_w, int src_h)
{
    if (!pose_detected || src_w <= 0 || src_h <= 0) {
        return 0.0f;
    }

    /*
     * Wheel On 判断逻辑：
     * 1. 左右手腕都必须检测到
     * 2. 两只手腕都在方向盘候选区域
     * 3. 两只手腕必须左右分开
     * 4. 两只手腕高度不能差太多
     *
     * 这样可以避免：
     * 单手靠嘴、单手靠耳、单手出现在画面中被误判为 Wheel On。
     */

    int left_valid =
        (lw_conf >= 0.20f &&
         lw_x > 1.0f && lw_y > 1.0f &&
         lw_x < (float)src_w - 1.0f &&
         lw_y < (float)src_h - 1.0f);

    int right_valid =
        (rw_conf >= 0.20f &&
         rw_x > 1.0f && rw_y > 1.0f &&
         rw_x < (float)src_w - 1.0f &&
         rw_y < (float)src_h - 1.0f);

    /*
     * 只要有一只手腕无效，直接判定 Wheel Off。
     * 这样单手永远不能触发 On。
     */
    if (!left_valid || !right_valid) {
        return 1.0f;
    }

    /*
     * 方向盘候选区域：
     * 上边界放宽到 0.22，让正常 10点/2点、9点/3点握法也能覆盖。
     * 下边界到 0.95，兼容较低握法。
     */
    float x1 = (float)src_w * 0.08f;
    float x2 = (float)src_w * 0.92f;
    float y1 = (float)src_h * 0.22f;
    float y2 = (float)src_h * 0.95f;

    int left_in_region =
        (lw_x >= x1 && lw_x <= x2 &&
         lw_y >= y1 && lw_y <= y2);

    int right_in_region =
        (rw_x >= x1 && rw_x <= x2 &&
         rw_y >= y1 && rw_y <= y2);

    if (!left_in_region || !right_in_region) {
        return 1.0f;
    }

    /*
     * 两只手必须左右明显分开。
     * 这是防止单手、双手贴胸前、双手靠嘴边误判为方向盘握持的关键。
     */
    float wrist_sep = fabsf(lw_x - rw_x);

    if (wrist_sep < (float)src_w * 0.22f) {
        return 1.0f;
    }

    /*
     * 两只手不能都挤在画面中间。
     * 方向盘握持时，左右手一般分布在中心线两侧。
     */
    float center_x = (float)src_w * 0.50f;
    float margin_x = (float)src_w * 0.08f;

    float min_x = (lw_x < rw_x) ? lw_x : rw_x;
    float max_x = (lw_x > rw_x) ? lw_x : rw_x;

    if (!(min_x < center_x - margin_x && max_x > center_x + margin_x)) {
        return 1.0f;
    }

    /*
     * 双手高度差不能过大。
     * 正常握方向盘时，两只手腕高度接近。
     * 允许一定倾斜，比如 10点/2点、9点/3点。
     */
    float dy = fabsf(lw_y - rw_y);

    if (dy > (float)src_h * 0.25f) {
        return 1.0f;
    }

    /*
     * 通过所有条件，认为双手在方向盘区域。
     */
    return 0.0f;
}
/* ================================================================
 * main
 * ================================================================ */
int main(int argc, char **argv)
{
    const char *pose_model_path;
    const char *face_model_path;

    if (argc == 3) {
        pose_model_path = argv[1];
        face_model_path = argv[2];
    } else if (argc == 1) {
        pose_model_path = "../../model/yolov8_pose.rknn";
        face_model_path = "../../model/RetinaFace.rknn";
        printf("Using default model paths:\n  %s\n  %s\n",
               pose_model_path, face_model_path);
    } else {
        printf("Usage: %s [pose_model_path face_model_path]\n", argv[0]);
        return -1;
    }

    signal(SIGINT, sigint_handler);

    int shm_id = -1;
    int res_shm = -1;

    SharedFrame *frame = NULL;
    BehaviorResult *result = NULL;

    rknn_app_context_t pose_ctx;
    rknn_app_context_t face_ctx;

    memset(&pose_ctx, 0, sizeof(pose_ctx));
    memset(&face_ctx, 0, sizeof(face_ctx));

    unsigned char *rgb_buf = NULL;

    int last_frame = 0;
    int debug_frame = 0;

    EngineState eng;
    memset(&eng, 0, sizeof(eng));

    init_post_process();

    shm_id = shmget(SHM_KEY, sizeof(SharedFrame), IPC_CREAT | 0666);
    if (shm_id < 0) {
        perror("shmget camera");
        goto out;
    }

    frame = (SharedFrame *)shmat(shm_id, NULL, 0);
    if (frame == (void *)-1) {
        frame = NULL;
        perror("shmat camera");
        goto out;
    }

    res_shm = shmget(RESULT_SHM_KEY, sizeof(BehaviorResult), IPC_CREAT | 0666);
    if (res_shm < 0) {
        perror("shmget result");
        goto out;
    }

    result = (BehaviorResult *)shmat(res_shm, NULL, 0);
    if (result == (void *)-1) {
        result = NULL;
        perror("shmat result");
        goto out;
    }

    memset(result, 0, sizeof(BehaviorResult));

    if (init_yolov8_pose_model(pose_model_path, &pose_ctx) != 0) {
        printf("[ERROR] init_yolov8_pose_model failed\n");
        goto out;
    }

    if (init_retinaface_model(face_model_path, &face_ctx) != 0) {
        printf("[ERROR] init_retinaface_model failed\n");
        goto out;
    }

    rgb_buf = (unsigned char *)malloc(FRAME_SIZE);
    if (!rgb_buf) {
        perror("malloc rgb_buf");
        goto out;
    }

    printf("[behavior_engine final] started\n");

    while (running) {
        if (frame->frame_count == last_frame) {
            usleep(10000);
            continue;
        }

        double ai_t0 = get_time_ms();

        last_frame = frame->frame_count;

        int src_w = frame->width;
        int src_h = frame->height;

        if (src_w <= 0 || src_h <= 0) {
            usleep(10000);
            continue;
        }

        memcpy(rgb_buf, frame->rgb_data, FRAME_SIZE);

        image_buffer_t src;
        memset(&src, 0, sizeof(src));

        src.width = src_w;
        src.height = src_h;
        src.format = IMAGE_FORMAT_RGB888;
        src.size = FRAME_SIZE;
        src.virt_addr = rgb_buf;

        /* ================= YOLOv8-Pose ================= */
        object_detect_result_list pose_res;
        memset(&pose_res, 0, sizeof(pose_res));

        double pose_t0 = get_time_ms();
        inference_yolov8_pose_model(&pose_ctx, &src, &pose_res);
        double pose_t1 = get_time_ms();

        float pose_ms = (float)(pose_t1 - pose_t0);
        float pose_fps = (pose_ms > 0.01f) ? (1000.0f / pose_ms) : 0.0f;

        /* ================= RetinaFace ================= */
        image_buffer_t retina_input;
        memset(&retina_input, 0, sizeof(retina_input));

        double face_t0 = get_time_ms();

        resize_image_cpu(&src, &retina_input, 320, 320);

        retinaface_result face_res;
        memset(&face_res, 0, sizeof(face_res));

        inference_retinaface_model(&face_ctx, &retina_input, &face_res);

        double face_t1 = get_time_ms();

        float face_ms = (float)(face_t1 - face_t0);
        float face_fps = (face_ms > 0.01f) ? (1000.0f / face_ms) : 0.0f;

        float scale_x = (float)src_w / 320.0f;
        float scale_y = (float)src_h / 320.0f;

        for (int i = 0; i < face_res.count; i++) {
            retinaface_object_t *obj = &face_res.object[i];

            obj->box.left   = (int)((float)obj->box.left * scale_x);
            obj->box.right  = (int)((float)obj->box.right * scale_x);
            obj->box.top    = (int)((float)obj->box.top * scale_y);
            obj->box.bottom = (int)((float)obj->box.bottom * scale_y);

            for (int j = 0; j < 5; j++) {
                obj->point[j].x = (int)((float)obj->point[j].x * scale_x);
                obj->point[j].y = (int)((float)obj->point[j].y * scale_y);
            }
        }

        keep_largest_face(&face_res);

        /* ================= Face result ================= */
        result->face_detected = (face_res.count > 0 &&
                                 face_res.object[0].score >= FACE_CONF_TH);
        result->face_count = 0;

        if (result->face_detected) {
            result->face_count = (face_res.count > 3) ? 3 : face_res.count;
        }

        for (int i = 0; i < 3; i++) {
            result->faces[i].left = 0;
            result->faces[i].top = 0;
            result->faces[i].right = 0;
            result->faces[i].bottom = 0;
            result->faces[i].score = 0.0f;

            for (int j = 0; j < 5; j++) {
                result->faces[i].points[j][0] = 0.0f;
                result->faces[i].points[j][1] = 0.0f;
            }
        }

        for (int i = 0; i < result->face_count; i++) {
            result->faces[i].left   = face_res.object[i].box.left;
            result->faces[i].top    = face_res.object[i].box.top;
            result->faces[i].right  = face_res.object[i].box.right;
            result->faces[i].bottom = face_res.object[i].box.bottom;
            result->faces[i].score  = face_res.object[i].score;

            for (int j = 0; j < 5; j++) {
                result->faces[i].points[j][0] = (float)face_res.object[i].point[j].x;
                result->faces[i].points[j][1] = (float)face_res.object[i].point[j].y;
            }
        }

        /* ================= Select driver pose ================= */
        int best_pose_idx = select_driver_pose(&pose_res, &face_res);
        object_detect_result *det = NULL;

        if (best_pose_idx >= 0 && best_pose_idx < pose_res.count) {
            det = &pose_res.results[best_pose_idx];
        }

        /* ================= Extract pose keypoints ================= */
        float lw_x = 0.0f, lw_y = 0.0f;
        float rw_x = 0.0f, rw_y = 0.0f;
        float le_x = 0.0f, le_y = 0.0f;
        float re_x = 0.0f, re_y = 0.0f;

        float lelb_x = 0.0f, lelb_y = 0.0f;
        float relb_x = 0.0f, relb_y = 0.0f;

        float nose_y = 0.0f;
        float left_eye_y = 0.0f;
        float right_eye_y = 0.0f;

        float shoulder_left_x = 0.0f, shoulder_left_y = 0.0f;
        float shoulder_right_x = 0.0f, shoulder_right_y = 0.0f;

        float mouth_x = 0.0f, mouth_y = 0.0f;

        float lw_conf = 0.0f;
        float rw_conf = 0.0f;
        float lelb_conf = 0.0f;
        float relb_conf = 0.0f;
        float le_conf = 0.0f;
        float re_conf = 0.0f;
        float nose_conf = 0.0f;
        float leye_conf = 0.0f;
        float reye_conf = 0.0f;

        int pose_detected = 0;
        int mouth_valid = 0;

        if (det && det->keypoints[0][2] > 0.50f) {
            float raw[17][3];

            for (int i = 0; i < 17; i++) {
                raw[i][0] = det->keypoints[i][0];
                raw[i][1] = det->keypoints[i][1];
                raw[i][2] = det->keypoints[i][2];
            }

            lw_conf = raw[9][2];
            rw_conf = raw[10][2];

            lelb_conf = raw[7][2];
            relb_conf = raw[8][2];

            le_conf = raw[3][2];
            re_conf = raw[4][2];

            nose_conf = raw[0][2];
            leye_conf = raw[1][2];
            reye_conf = raw[2][2];

            if (!eng.init) {
                for (int i = 0; i < 17; i++) {
                    eng.prev_keypoints[i][0] = raw[i][0];
                    eng.prev_keypoints[i][1] = raw[i][1];
                }

                eng.init = 1;
            } else {
                for (int i = 0; i < 17; i++) {
                    if (raw[i][2] >= KP_CONF_TH) {
                        eng.prev_keypoints[i][0] =
                            SMOOTH_ALPHA * raw[i][0] +
                            (1.0f - SMOOTH_ALPHA) * eng.prev_keypoints[i][0];

                        eng.prev_keypoints[i][1] =
                            SMOOTH_ALPHA * raw[i][1] +
                            (1.0f - SMOOTH_ALPHA) * eng.prev_keypoints[i][1];
                    }
                }
            }

            lw_x = eng.prev_keypoints[9][0];
            lw_y = eng.prev_keypoints[9][1];

            rw_x = eng.prev_keypoints[10][0];
            rw_y = eng.prev_keypoints[10][1];

            lelb_x = eng.prev_keypoints[7][0];
            lelb_y = eng.prev_keypoints[7][1];

            relb_x = eng.prev_keypoints[8][0];
            relb_y = eng.prev_keypoints[8][1];

            le_x = eng.prev_keypoints[3][0];
            le_y = eng.prev_keypoints[3][1];

            re_x = eng.prev_keypoints[4][0];
            re_y = eng.prev_keypoints[4][1];

            nose_y = eng.prev_keypoints[0][1];

            left_eye_y = eng.prev_keypoints[1][1];
            right_eye_y = eng.prev_keypoints[2][1];

            shoulder_left_x = eng.prev_keypoints[5][0];
            shoulder_left_y = eng.prev_keypoints[5][1];

            shoulder_right_x = eng.prev_keypoints[6][0];
            shoulder_right_y = eng.prev_keypoints[6][1];

            pose_detected = 1;

            if (lw_conf >= KP_CONF_TH) {
                wrist_tracker_push(&eng.left_wrist_tracker, lw_x, lw_y);
            }

            if (rw_conf >= KP_CONF_TH) {
                wrist_tracker_push(&eng.right_wrist_tracker, rw_x, rw_y);
            }
        }

        /* ================= Mouth point from RetinaFace ================= */
        if (result->face_detected && result->face_count > 0) {
            retinaface_object_t *face = &face_res.object[0];

            float raw_mouth_x = ((float)face->point[3].x + (float)face->point[4].x) * 0.5f;
            float raw_mouth_y = ((float)face->point[3].y + (float)face->point[4].y) * 0.5f;

            if (!eng.mouth_init) {
                eng.smoothed_mouth_x = raw_mouth_x;
                eng.smoothed_mouth_y = raw_mouth_y;
                eng.mouth_init = 1;
            } else {
                eng.smoothed_mouth_x =
                    SMOOTH_ALPHA * raw_mouth_x +
                    (1.0f - SMOOTH_ALPHA) * eng.smoothed_mouth_x;

                eng.smoothed_mouth_y =
                    SMOOTH_ALPHA * raw_mouth_y +
                    (1.0f - SMOOTH_ALPHA) * eng.smoothed_mouth_y;
            }

            mouth_x = eng.smoothed_mouth_x;
            mouth_y = eng.smoothed_mouth_y;
            mouth_valid = 1;
        } else {
            mouth_valid = 0;
            eng.mouth_init = 0;
        }

        float shoulder_width = distance2d(shoulder_left_x,
                                          shoulder_left_y,
                                          shoulder_right_x,
                                          shoulder_right_y);

        float body_scale = compute_body_scale(shoulder_width, &face_res, src_w);

        float left_stability = wrist_stability_score(&eng.left_wrist_tracker, body_scale);
        float right_stability = wrist_stability_score(&eng.right_wrist_tracker, body_scale);

        /* ================= Head down ================= */
        float pose_head_score = 0.0f;
        float face_head_score = 0.0f;

        if (pose_detected && nose_conf >= KP_CONF_TH &&
            leye_conf >= KP_CONF_TH && reye_conf >= KP_CONF_TH) {

            float eye_center_y = (left_eye_y + right_eye_y) * 0.5f;
            float head_down_ratio = (nose_y - eye_center_y) / body_scale;

            eng.smoothed_pose_pitch =
                0.70f * eng.smoothed_pose_pitch +
                0.30f * head_down_ratio;

            pose_head_score = score_by_value_high(eng.smoothed_pose_pitch,
                                                  0.10f,
                                                  0.22f);
        }

        if (result->face_detected && result->face_count > 0) {
            float face_h = (float)(result->faces[0].bottom - result->faces[0].top);

            if (face_h > 10.0f) {
                float eye_y = (result->faces[0].points[0][1] +
                               result->faces[0].points[1][1]) * 0.5f;

                float nose_face_y = result->faces[0].points[2][1];

                float face_pitch = (nose_face_y - eye_y) / face_h;

                eng.smoothed_face_pitch =
                    0.75f * eng.smoothed_face_pitch +
                    0.25f * face_pitch;

                face_head_score = score_by_value_high(eng.smoothed_face_pitch,
                                                      0.18f,
                                                      0.32f);
            }
        }

        float head_score = (pose_head_score > face_head_score) ?
                           pose_head_score : face_head_score;

        /* ================= Phone / Mouth Risk ================= */
        float phone_score = 0.0f;
        float mouth_score = 0.0f;

        int face_valid_for_behavior = 0;
        float face_left = 0.0f;
        float face_top = 0.0f;
        float face_right = 0.0f;
        float face_bottom = 0.0f;

        if (result->face_detected && result->face_count > 0) {
            face_valid_for_behavior = 1;
            face_left = result->faces[0].left;
            face_top = result->faces[0].top;
            face_right = result->faces[0].right;
            face_bottom = result->faces[0].bottom;
        }

        compute_phone_mouth_scores(
        lw_x, lw_y, lw_conf,
        rw_x, rw_y, rw_conf,

        lelb_x, lelb_y, lelb_conf,
        relb_x, relb_y, relb_conf,

        le_x, le_y, le_conf,
        re_x, re_y, re_conf,

        mouth_x, mouth_y, mouth_valid,
        face_valid_for_behavior,
        face_left, face_top, face_right, face_bottom,
        body_scale,
        left_stability,
        right_stability,
        &phone_score,
        &mouth_score
    );

        /* ================= Yaw / Eye Risk / Hands Off ================= */
        float yaw_score = 0.0f;
        int yaw_state = 0;
        float eye_risk_score = 0.0f;

        compute_face_yaw_eye_risk(result,
                                  &eng,
                                  head_score,
                                  &yaw_score,
                                  &yaw_state,
                                  &eye_risk_score);

        float hands_score = compute_hands_off_score(
            pose_detected,
            lw_x, lw_y, lw_conf,
            rw_x, rw_y, rw_conf,
            src_w,
            src_h
        );

        /* ================= Windows ================= */
        eng.phone_window[eng.window_idx] = phone_score;
        eng.mouth_window[eng.window_idx] = mouth_score;
        eng.head_window[eng.window_idx] = head_score;
        eng.yaw_window[eng.window_idx] = yaw_score;
        eng.hands_window[eng.window_idx] = hands_score;
        eng.eye_window[eng.window_idx] = eye_risk_score;

        eng.window_idx = (eng.window_idx + 1) % WINDOW_SIZE;
        if (eng.window_filled < WINDOW_SIZE) {
            eng.window_filled++;
        }

        float phone_avg = avg_window(eng.phone_window, WINDOW_SIZE);
        float mouth_avg = avg_window(eng.mouth_window, WINDOW_SIZE);
        float head_avg = avg_window(eng.head_window, WINDOW_SIZE);
        float yaw_avg = avg_window(eng.yaw_window, WINDOW_SIZE);
        float hands_avg = avg_window(eng.hands_window, WINDOW_SIZE);
        float eye_avg = avg_window(eng.eye_window, WINDOW_SIZE);

        int phone_high_count = count_window_above(eng.phone_window, WINDOW_SIZE, 0.30f);
        int mouth_high_count = count_window_above(eng.mouth_window, WINDOW_SIZE, 0.32f);
        int head_high_count = count_window_above(eng.head_window, WINDOW_SIZE, 0.50f);
        int yaw_high_count = count_window_above(eng.yaw_window, WINDOW_SIZE, 0.55f);
        int hands_high_count = count_window_above(eng.hands_window, WINDOW_SIZE, 0.60f);

        /*
        * Time delta for Phone / Mouth accumulation.
        * This prevents one-frame touch from immediately triggering behavior.
        */
        float dt_sec = 0.10f;

        if (eng.last_logic_ms > 1.0) {
            dt_sec = (float)((ai_t0 - eng.last_logic_ms) / 1000.0);
            dt_sec = clamp_float(dt_sec, 0.03f, 0.20f);
        }

        eng.last_logic_ms = ai_t0;

        /* ================= State machines ================= */
        /*
        * Phone state machine with time accumulation.
        * 短暂碰到耳朵不会立刻触发，持续保持约 0.8~1.0 秒才触发。
        */
        int phone_evidence =
            (phone_score >= 0.58f) ||
            (phone_avg >= 0.42f && phone_high_count >= 3);

        eng.phone_accum = update_behavior_accum(
            eng.phone_accum,
            phone_evidence,
            dt_sec,
            0.55f,   /* rise: about 2.2s to trigger */
            1.20f,   /* fall: about 0.8s to exit after leaving */
            3.00f
        );

        if (!eng.phone_state) {
            if (eng.phone_accum >= 1.20f) {
                eng.phone_state = 1;
            }
        } else {
            if (eng.phone_accum <= 0.25f && phone_avg < 0.18f) {
                eng.phone_state = 0;
            }
        }

        /*
        * Mouth Risk state machine with time accumulation.
        * 短暂摸嘴/擦脸不会立刻触发，持续在人脸框正下方约 1 秒才触发。
        */
        int mouth_evidence =
            (mouth_score >= 0.58f) ||
            (mouth_avg >= 0.38f && mouth_high_count >= 3);

        eng.mouth_accum = update_behavior_accum(
            eng.mouth_accum,
            mouth_evidence,
            dt_sec,
            0.60f,   /* rise: about 2.0s to trigger */
            1.20f,   /* fall: about 0.8s to exit after leaving */
            3.00f
        );

        if (!eng.mouth_state) {
            if (eng.mouth_accum >= 1.20f) {
                eng.mouth_state = 1;
            }
        } else {
            if (eng.mouth_accum <= 0.25f && mouth_avg < 0.16f) {
                eng.mouth_state = 0;
            }
        }

        if (!eng.head_state) {
            if (head_avg > 0.42f && head_high_count >= 4) {
                eng.head_state = 1;
            }
        } else {
            if (head_avg < 0.25f || head_high_count <= 1) {
                eng.head_state = 0;
            }
        }

        if (!eng.yaw_state) {
            if (yaw_avg > 0.55f && yaw_high_count >= 4) {
                eng.yaw_state = yaw_state ? yaw_state : 1;
            }
        } else {
            if (yaw_avg < 0.25f || yaw_high_count <= 1) {
                eng.yaw_state = 0;
            } else if (yaw_state != 0) {
                eng.yaw_state = yaw_state;
            }
        }

        /*
        * Wheel On / Off 状态机
        *
        * hands_score 越高，表示越可能双手离开方向盘区域。
        *
        * 关键改进：
        * 1. 当前帧已经是高风险 hands_score >= 0.90f 时，立即进入 Wheel Off。
        *    这样单手、无手、手靠嘴/耳不会被滑动窗口拖住。
        *
        * 2. 从 Wheel Off 回到 Wheel On 时更严格：
        *    必须当前帧 score 很低，并且平均值也低，避免单手误回 On。
        */
        if (!eng.hands_off_state) {
            /*
            * 当前是 Wheel On。
            * 只要当前帧明确不是双手握方向盘，立即切到 Off。
            */
            if (hands_score >= 0.90f || (hands_avg > 0.35f && hands_high_count >= 3)) {
                eng.hands_off_state = 1;
            }
        } else {
            /*
            * 当前是 Wheel Off。
            * 只有连续稳定检测到双手都在方向盘区域，才切回 On。
            */
            if (hands_score <= 0.10f && hands_avg < 0.20f && hands_high_count <= 1) {
                eng.hands_off_state = 0;
            }
        }

        /*
        * Strict phone vs mouth exclusivity.
        *
        * 演示模式下，Phone 和 Mouth 不应该同时存在。
        * 如果同时触发，分数明显更高的一方保留；
        * 如果分数接近，默认保留 Phone，避免打电话动作被误抢成 Mouth Risk。
        */
        if (eng.phone_state && eng.mouth_state) {
            if (mouth_avg > phone_avg + 0.12f) {
                eng.phone_state = 0;
                eng.phone_accum = 0.0f;
            } else {
                eng.mouth_state = 0;
                eng.mouth_accum = 0.0f;
            }
        }

        /* ================= Write results ================= */
        result->phone_call = eng.phone_state;
        result->smoking = eng.mouth_state;       /* compatibility: UI should rename it to Mouth Risk */
        result->mouth_risk = eng.mouth_state;

        result->head_down = eng.head_state;
        result->pose_detected = pose_detected;

        result->head_yaw_score = yaw_avg;
        result->head_yaw_state = eng.yaw_state;
        result->eye_risk_score = eye_avg;
        result->hands_off = eng.hands_off_state;

        result->phone_score = phone_avg;
        result->mouth_risk_score = mouth_avg;
        result->head_down_score = head_avg;

        /*
        * 写入姿态关键点到共享内存。
        *
        * 注意：
        * 只有当前帧关键点置信度足够高，才写入坐标。
        * 如果置信度低，就写 0。
        *
        * 这样 UI 在画骨架线时会自动隐藏无效点，
        * 避免手离开画面后还残留旧骨架线。
        */
        if (pose_detected && det) {
            for (int i = 0; i < 17; i++) {
                float conf = det->keypoints[i][2];

                if (conf >= 0.20f) {
                    result->pose_keypoints[i][0] = eng.prev_keypoints[i][0];
                    result->pose_keypoints[i][1] = eng.prev_keypoints[i][1];
                } else {
                    result->pose_keypoints[i][0] = 0.0f;
                    result->pose_keypoints[i][1] = 0.0f;
                }
            }
        } else {
            for (int i = 0; i < 17; i++) {
                result->pose_keypoints[i][0] = 0.0f;
                result->pose_keypoints[i][1] = 0.0f;
            }
        }

                /*
        * ================= Final Risk Score =================
        *
        * 设计目标：
        * 1. 正常驾驶：0 ~ 20
        * 2. Mouth Risk：约 45 ~ 55
        * 3. Phone Risk：约 65 ~ 78
        * 4. Head Down：约 72 ~ 88
        * 5. Hands Off：约 55 ~ 68
        * 6. 多风险叠加：80 ~ 100
        *
        * 说明：
        * fatigue_score 不是单一疲劳分，而是综合驾驶风险分。
        * 这里采用“状态基础分 + 连续分数补偿 + 多风险叠加”的方式，
        * 比单纯加权平均更适合 UI 展示和答辩演示。
        */
        float risk_score = 0.0f;

        /* 弱风险连续分：即使状态机还没触发，UI 分数也会有轻微响应 */
        risk_score = fmaxf(risk_score, head_avg  * 42.0f);
        risk_score = fmaxf(risk_score, phone_avg * 38.0f);
        risk_score = fmaxf(risk_score, mouth_avg * 32.0f);
        risk_score = fmaxf(risk_score, hands_avg * 35.0f);
        risk_score = fmaxf(risk_score, yaw_avg   * 30.0f);
        risk_score = fmaxf(risk_score, eye_avg   * 28.0f);

        /*
        * 状态触发后的基础风险分。
        * 这样可以避免“明明检测到打电话，状态分只有 30 多”的问题。
        */
        if (eng.head_state) {
            risk_score = fmaxf(risk_score, 72.0f);
        }

        if (eng.phone_state) {
            risk_score = fmaxf(risk_score, 64.0f);
        }

        if (eng.mouth_state) {
            risk_score = fmaxf(risk_score, 46.0f);
        }

        if (eng.hands_off_state) {
            risk_score = fmaxf(risk_score, 55.0f);
        }

        if (eng.yaw_state != 0) {
            risk_score = fmaxf(risk_score, 38.0f);
        }

        /*
        * 连续强度补偿：
        * 动作越明显、持续越稳定，分数越高。
        */
        if (eng.head_state) {
            risk_score += head_avg * 16.0f;
            risk_score += eye_avg  * 6.0f;
        }

        if (eng.phone_state) {
            risk_score += phone_avg * 16.0f;
        }

        if (eng.mouth_state) {
            risk_score += mouth_avg * 8.0f;
        }

        if (eng.hands_off_state) {
            risk_score += hands_avg * 10.0f;
        }

        if (eng.yaw_state != 0) {
            risk_score += yaw_avg * 8.0f;
        }

        /*
        * 多行为叠加奖励：
        * 同时出现多个风险时，综合风险明显提高。
        */
        int risk_count = 0;

        if (eng.head_state)      risk_count++;
        if (eng.phone_state)     risk_count++;
        if (eng.mouth_state)     risk_count++;
        if (eng.hands_off_state) risk_count++;
        if (eng.yaw_state != 0)  risk_count++;

        /*
        * Mouth Risk 单独存在时，限制最高分。
        * 因为它代表嘴部风险动作，不等同于明确危险行为叠加。
        */
        if (eng.mouth_state &&
            !eng.phone_state &&
            !eng.head_state &&
            !eng.hands_off_state &&
            eng.yaw_state == 0) {
            if (risk_score > 65.0f) {
                risk_score = 65.0f;
            }
        }

        if (risk_count >= 2) {
            risk_score += 10.0f;
        }

        if (risk_count >= 3) {
            risk_score += 8.0f;
        }

        if (risk_count >= 4) {
            risk_score += 6.0f;
        }

        /*
        * 限幅。
        */
        risk_score = clamp_float(risk_score, 0.0f, 100.0f);

        result->fatigue_score = (int)(risk_score + 0.5f);

        result->frame_seq = frame->frame_count;
        result->timestamp = (uint64_t)time(NULL) * 1000000LL;
        result->heartbeat = result->timestamp;

        double ai_t1 = get_time_ms();
        float ai_ms = (float)(ai_t1 - ai_t0);
        float ai_fps = (ai_ms > 0.01f) ? (1000.0f / ai_ms) : 0.0f;

        result->pose_infer_ms = pose_ms;
        result->pose_infer_fps = pose_fps;

        result->face_infer_ms = face_ms;
        result->face_infer_fps = face_fps;

        result->ai_infer_ms = ai_ms;
        result->ai_infer_fps = ai_fps;

        if (debug_frame++ % 30 == 0) {
            printf("DBG: head=%.2f phone=%.2f mouth=%.2f yaw=%.2f eye=%.2f hands=%.2f | final H/P/M/Y/Hands=%d/%d/%d/%d/%d\n",
                   head_avg,
                   phone_avg,
                   mouth_avg,
                   yaw_avg,
                   eye_avg,
                   hands_avg,
                   result->head_down,
                   result->phone_call,
                   result->mouth_risk,
                   result->head_yaw_state,
                   result->hands_off);

            printf("PERF: AI %.2fms %.2fFPS | Pose %.2fms %.2fFPS | Face %.2fms %.2fFPS\n",
                   result->ai_infer_ms,
                   result->ai_infer_fps,
                   result->pose_infer_ms,
                   result->pose_infer_fps,
                   result->face_infer_ms,
                   result->face_infer_fps);
        }

        if (retina_input.virt_addr) {
            free(retina_input.virt_addr);
            retina_input.virt_addr = NULL;
        }

        usleep(10000);
    }

out:
    printf("[behavior_engine final] stopping...\n");

    if (rgb_buf) {
        free(rgb_buf);
        rgb_buf = NULL;
    }

    if (result) {
        shmdt(result);

        /*
         * Keep this behavior consistent with your current scripts.
         * If UI exits later than behavior_engine and has shm issues,
         * comment out this shmctl line.
         */
        if (res_shm >= 0) {
            shmctl(res_shm, IPC_RMID, NULL);
        }
    }

    if (frame) {
        shmdt(frame);
    }

    release_retinaface_model(&face_ctx);
    release_yolov8_pose_model(&pose_ctx);
    deinit_post_process();

    return 0;
}
