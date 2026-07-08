# rk3588-driver-monitor

> 基于 RK3588 的端侧实时感知 + PC 端认知分析闭环监测系统。

## 项目简介

本作品面向智能座舱安全需求，针对传统 DMS「检测即报警、无法持续干预、缺乏行为理解」的痛点，设计了一套具备**可解释性**与**时序认知能力**的智能监测方案。

- **端侧（ELF2/RK3588）**：融合 YOLOv8-Pose 与 RetinaFace，实现低头、离盘、打电话等多行为实时监测；独创**30秒滑动窗口风险累积机制**，将单帧误报转化为 L0-L3 四级动态风险。
- **PC 端（DeepSeek Agent）**：部署 DeepSeek 大模型，对风险事件序列进行时序认知分析，生成可解释的干预策略与驾驶员画像。

## 核心创新点

1. **认知风险累积机制**：通过滑动窗口融合多行为时序特征，引入车辆模式修正与重复加权，解决传统 DMS 单帧误报问题。
2. **端云混合决策架构**：端侧负责 40ms 级硬规则保底与座舱干预（红屏/语音/风扇），PC 端承担非实时策略解释，二者解耦协同且端侧始终掌握安全兜底权。
3. **全链路可视化闭环**：从感知、决策、执行到反馈，通过 LVGL 端侧界面与 PC Dashboard 实现状态同步与一键故障恢复。

## 技术栈

| 模块 | 技术选型 |
|------|----------|
| **硬件平台** | Rockchip RK3588 (ELF2), OV13855, MPU6050, AHT20 |
| **视觉算法** | YOLOv8-Pose, RetinaFace, RKNN 量化加速 |
| **图形界面** | LVGL (Linux Framebuffer) |
| **云端认知** | DeepSeek API, Python Flask |
| **交互控制** | ASRPRO 离线语音, PWM 风扇驱动 |

## 快速开始

### 环境要求

- **端侧**：RK3588 开发板（Linux），Python 3.8+，RKNN Toolkit
- **PC 端**：Python 3.8+，Flask

### 端侧部署

```bash
# 克隆仓库
git clone https://github.com/JH3234501237-UI/rk3588-driver-monitor.git
cd rk3588-driver-monitor

# 配置 API Key（重要！）
# 请编辑 deepseek_proxy/start_edge_agent.bat 或对应的 .sh 脚本

# 启动演示（含一键清场与保护期设置）
./stop.sh
./start.sh
./start_demo.sh
