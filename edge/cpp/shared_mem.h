#ifndef SHARED_MEM_H
#define SHARED_MEM_H

#include <stdint.h>
#include <stddef.h>

#define SHM_KEY         0x1234      // 摄像头共享内存
#define RESULT_SHM_KEY  0x5678      // 行为检测结果共享内存
#define HEARTBEAT_KEY   0x9ABC      // 心跳共享内存（可选，可集成到RESULT）

#define CAM_WIDTH       1280
#define CAM_HEIGHT      720
#define FRAME_SIZE      (CAM_WIDTH * CAM_HEIGHT * 3)

#define MAX_PROCESSES   3   // producer, engine, ui
#define HEARTBEAT_TIMEOUT_MS 2000

// #define ENV_SHM_KEY  0xABCD
#define OBJECT_SHM_KEY 0x5679

// 人脸信息结构体
typedef struct {
    int left, top, right, bottom;   // 人脸框
    int points[5][2];               // 5个关键点：左眼、右眼、鼻尖、左嘴角、右嘴角
    float score;                    // 置信度
} FaceInfo;

typedef struct {
    int width;
    int height;
    int frame_count;
    uint64_t timestamp;
    unsigned char rgb_data[FRAME_SIZE];
} SharedFrame;

typedef struct {
    int has_phone;
    int has_bottle;
    int has_cup;
    float phone_box[4];
    float bottle_box[4];
    float cup_box[4];
    uint64_t timestamp;
} ObjectResult;

// 行为检测结果
typedef struct {
    int frame_seq;
    uint64_t timestamp;
    int phone_call;       // 打电话
    int smoking;          // 抽烟/手靠近嘴
    int head_down;        // 低头
    // 姿态关键点（用于绘制骨架）
    float pose_keypoints[17][2];
    int pose_detected;
    // 心跳（每个进程定期更新时间戳）
    uint64_t heartbeat;

    int eye_closed;
    // 新增：人脸检测结果
    int face_detected;              // 是否检测到人脸
    int face_count;                 // 实际检测到的人脸数量（最多3）
    FaceInfo faces[3];              // 最多保存3个人脸

     // ========== 时序状态引擎输出 ==========
    int fatigue_score;          // 0-100
    int driver_state;           // 0=正常,1=轻度风险,2=疲劳,3=危险
    int perclos;                // 闭眼比例 0-100
    int blink_freq;             // 眨眼频率 次/分钟
    int yawn_count;             // 预留

    /* 性能指标：由 behavior_engine 写入，UI 读取 */
    float ai_infer_ms;
    float ai_infer_fps;

    float pose_infer_ms;
    float pose_infer_fps;

    float face_infer_ms;
    float face_infer_fps;

    /* Extended behavior signals */
    float head_yaw_score;
    int head_yaw_state;       // 0 normal, 1 left, 2 right

    float eye_risk_score;     // 0.0 - 1.0, estimated, not real PERCLOS

    int hands_off;            // 0 on wheel / unknown, 1 hands off
    int mouth_risk;           // 0 normal, 1 mouth risk

    float phone_score;
    float mouth_risk_score;
    float head_down_score;

} BehaviorResult;

// 进程内部状态（用于平滑、历史）
// typedef struct {
//     float prev_keypoints[17][2];  // 上一帧关键点
//     float ear_dist_norm_left;     // 归一化距离
//     float eye_dist;               // 眼距（归一化分母）
//     int init;                     // 是否已初始化
// } EngineState;

// typedef struct {
//     float temperature;
//     float humidity;
//     int   air_quality;
//     int   fan_status;
//     float target_temp;
// } EnvData;

// 温湿度共享内存键值
#define ENV_SHM_KEY 0x567A

typedef struct {
    float temperature;
    float humidity;
    uint64_t timestamp;
} EnvData;

#define FAN_CTRL_SHM_KEY 0x567B
typedef struct {
    int mode;        // 0: 真实模式（芯片温度）, 1: 模拟模式（滑块输入）
    float sim_temp;  // 模拟温度值（仅 mode=1 时使用）
    int manual_speed;  // 手动风速 (0-100, 仅 mode=2 时使用)
    uint64_t timestamp;
    int current_speed;  // 当前实际风速 0-100（由 fan_control 计算）
    float chip_temp;    // 当前芯片温度（由 fan_control 写入）
} FanCtrl;

#endif