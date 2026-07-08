# -*- coding: utf-8 -*-
import os
import re
import time
from pathlib import Path
from collections import Counter, deque

from flask import Flask, request, jsonify, Response
from openai import OpenAI


# =========================
# 基础配置
# =========================

app = Flask(__name__)

client = OpenAI(
    api_key=os.environ.get("DEEPSEEK_API_KEY"),
    base_url="https://api.deepseek.com"
)

MODEL_NAME = "deepseek-chat"


# =========================
# 内存状态
# =========================

vehicle_cache = deque(maxlen=80)

latest_context = "暂无边缘端上下文"
latest_answer = "暂无 AI 认知分析结果"
latest_latency = "--"
cloud_status = "OFFLINE"
mqtt_status = "OPTIONAL"

driver_profile = {
    "first_seen": "--",
    "last_seen": "--",
    "total_requests": 0,
    "high_risk_count": 0,
    "avg_latency": "--",
    "event_counter": Counter(),
    "last_intervention": "--",
    "last_recovery": "--",
    "current_policy": "DEFAULT_POLICY",
}

latest_state = {
    "vehicle_mode": "UNKNOWN",
    "risk_score": 0,
    "event": "NONE",
    "source": "RK3588",
    "cognitive_level": "0",
    "cognitive_score": "0",
    "action_status": "Action: --",
    "intervention_mode": "NORMAL",
    "fan_action": "NONE",
    "fan_state": "NORMAL",
    "updated_at": 0,
}

# 开发板实时上报状态。由 /edge/state 更新。
EDGE_LIVE_STATE = {
    "updated_at": 0,
    "event": "NONE",
    "risk_score": 0,
    "vehicle_mode": "UNKNOWN",
    "action_status": "",
    "source": "RK3588",
    "cognitive_level": "0",
    "cognitive_score": "0",
    "fan_action": "NONE",
    "fan_state": "NORMAL",
}


# =========================
# 工具函数：时间 / 数字
# =========================

def now_str():
    return time.strftime("%Y-%m-%d %H:%M:%S")


def safe_int(x, default=0):
    try:
        return int(float(x))
    except Exception:
        return default


# =========================
# 工具函数：事件 / 车辆状态统一口径
# =========================

def canonical_event(value):
    """
    把各种事件写法统一成固定枚举：
    HEAD_DOWN / WHEEL_OFF / PHONE_CALL / SMOKING / MOUTH_YAWN / NONE / UNKNOWN
    """
    s = str(value or "").strip().upper()

    if "WHEEL_OFF" in s or "双手离盘" in s or "离盘" in s:
        return "WHEEL_OFF"

    if "HEAD_DOWN" in s or "HEAD DOWN" in s or "低头" in s:
        return "HEAD_DOWN"

    if "PHONE_CALL" in s or "PHONE CALL" in s or "PHONE" in s or "电话" in s:
        return "PHONE_CALL"

    if (
        "MOUTH_RISK" in s
        or "MOUTH" in s
        or "YAWN" in s
        or "SMOKING" in s
        or "SMOKE" in s
        or "抽烟" in s
        or "疲劳" in s
        or "打哈欠" in s
    ):
        return "MOUTH_RISK"

    if s in ("NONE", "NORMAL", "NO_RISK", "0", "无", "--", ""):
        return "NONE"

    return s or "UNKNOWN"


def event_to_cn(event):
    table = {
        "HEAD_DOWN": "低头",
        "WHEEL_OFF": "双手离盘",
        "PHONE_CALL": "打电话",
        "MOUTH_RISK": "嘴部风险",
        "MOUTH_YAWN": "疲劳/打哈欠",
        "SMOKING": "抽烟",
        "NONE": "正常",
        "UNKNOWN": "--",
        "--": "--",
    }
    return table.get(canonical_event(event), str(event or "--"))


def extract_event_from_action_status(text):
    """
    从动作状态里解析 Event:WHEEL_OFF 这种字段。
    这一步非常关键：如果 action_status 是 WHEEL_OFF，
    那 Dashboard 当前事件、表格、图表都应该按 WHEEL_OFF 处理。
    """
    text = str(text or "")

    m = re.search(r"Event\s*[:=]\s*([A-Z_]+)", text, re.I)
    if m:
        return canonical_event(m.group(1))

    ev = canonical_event(text)
    if ev not in ("NONE", "UNKNOWN"):
        return ev

    return None


def normalize_vehicle_mode(value):
    """
    把车辆状态统一成 DRIVING / PARKED / AUTO / UNKNOWN。
    """
    s = str(value or "").strip().upper()

    if s in ("DRIVING", "DRIVE", "MOVING", "RUNNING", "1"):
        return "DRIVING"

    if s in ("PARKED", "PARK", "STOP", "STOPPED", "0"):
        return "PARKED"

    if s in ("AUTO", "AUTOMATIC"):
        return "AUTO"

    if s in ("--", "", "NONE", "UNKNOWN"):
        return "UNKNOWN"

    return s


def normalize_action_status(action_status):
    s = str(action_status or "").strip()
    return s if s else "Action: --"


def choose_event(raw_event, action_status, risk_score):
    """
    当前事件选择逻辑：
    1. 如果 risk_score <= 0 且 raw_event 是 NONE，则认为当前正常，不再用旧 action_status 误判；
    2. 如果 action_status 中有 Event:xxx 且当前有风险分数，优先使用 action_status；
    3. 否则使用 raw_event。
    """
    raw = canonical_event(raw_event)
    score = safe_int(risk_score, 0)
    action_event = extract_event_from_action_status(action_status)

    if score <= 0 and raw in ("NONE", "UNKNOWN"):
        return "NONE"

    if action_event:
        return action_event

    return raw


def parse_action_level(action_status, default="0"):
    text = str(action_status or "")
    m = re.search(r"Risk\s*Level\s*([0-9]+)", text, re.I)
    if m:
        return m.group(1)

    m = re.search(r"LEVEL[_\s:=]+([0-9]+)", text, re.I)
    if m:
        return m.group(1)

    return str(default)


def normalize_record(row):
    """
    统一缓存记录口径：
    - event 优先解析 action_status 里的 Event:xxx
    - vehicle_mode 统一枚举
    - risk_score 转数字
    """
    r = dict(row or {})

    action = normalize_action_status(r.get("action_status", ""))
    score = safe_int(r.get("risk_score", r.get("score", 0)), 0)

    r["action_status"] = action
    r["risk_score"] = score
    r["event"] = choose_event(r.get("event", "NONE"), action, score)
    r["vehicle_mode"] = normalize_vehicle_mode(r.get("vehicle_mode", "UNKNOWN"))
    r["cognitive_level"] = str(r.get("cognitive_level", parse_action_level(action, "0")))
    r["cognitive_score"] = str(r.get("cognitive_score", "0"))
    r["source"] = r.get("source", "RK3588")
    r["time"] = r.get("time", now_str())

    return r


def should_append_record(new_record):
    """
    避免 /edge/state 每秒上报导致表格刷满重复记录。
    - 如果事件/车辆/分数/动作发生变化，立即记录；
    - 如果没有变化，每 5 秒最多记录一次。
    """
    if not vehicle_cache:
        return True

    last = normalize_record(vehicle_cache[-1])
    cur = normalize_record(new_record)

    keys = ["vehicle_mode", "risk_score", "event", "cognitive_level", "action_status"]
    changed = any(str(last.get(k)) != str(cur.get(k)) for k in keys)

    if changed:
        return True

    try:
        last_ts = time.mktime(time.strptime(last.get("time", now_str()), "%Y-%m-%d %H:%M:%S"))
        return time.time() - last_ts >= 5
    except Exception:
        return True


# =========================
# 驾驶员画像
# =========================

def update_driver_profile(event, risk_score, latency=None):
    event = canonical_event(event)
    score = safe_int(risk_score)

    driver_profile["total_requests"] += 1
    driver_profile["last_seen"] = now_str()

    if driver_profile["first_seen"] == "--":
        driver_profile["first_seen"] = now_str()

    if event and event not in ("--", "NONE", "UNKNOWN"):
        driver_profile["event_counter"][event] += 1

    if score >= 60:
        driver_profile["high_risk_count"] += 1

    if latency:
        driver_profile["avg_latency"] = latency


def build_deep_driver_profile(normalized_cache):
    """
    给 Dashboard 使用的更深驾驶员画像。
    """
    if not normalized_cache:
        return {
            "first_seen": driver_profile["first_seen"],
            "last_seen": driver_profile["last_seen"],
            "total_requests": driver_profile["total_requests"],
            "high_risk_count": driver_profile["high_risk_count"],
            "high_risk_ratio": "0%",
            "avg_ai_latency": driver_profile["avg_latency"],
            "avg_latency": driver_profile["avg_latency"],
            "dominant_risk_type": "--",
            "peak_risk_hour": "--",
            "trend_label": "平稳",
            "response_rate": "0%",
            "stability_label": "稳定",
            "tags": ["暂无足够画像数据"],
            "top3_behaviors": [],
            "summary": "当前暂无足够历史数据，系统尚未形成稳定驾驶员风险画像。",
            "last_intervention": driver_profile["last_intervention"],
            "last_recovery": driver_profile["last_recovery"],
            "current_policy": driver_profile["current_policy"],
            "top_events": dict(driver_profile["event_counter"].most_common(6)),
        }

    counter = Counter()
    hours = Counter()
    scores = []
    feedback_active_count = 0

    first_seen = normalized_cache[-1].get("time", "--")
    last_seen = normalized_cache[0].get("time", "--")

    for r in normalized_cache:
        ev = canonical_event(r.get("event"))
        if ev not in ("NONE", "UNKNOWN", "--"):
            counter[ev] += 1

        score = safe_int(r.get("risk_score", 0))
        scores.append(score)

        t = str(r.get("time", ""))
        m = re.search(r"(\d{2}):\d{2}:\d{2}", t)
        if m:
            hours[m.group(1)] += 1

        if "ACTIVE" in str(r.get("action_status", "")).upper():
            feedback_active_count += 1

    total = len(normalized_cache)
    high = sum(1 for r in normalized_cache if safe_int(r.get("risk_score", 0)) >= 60)
    high_ratio = round(high * 100 / total, 1) if total else 0

    dominant_event = counter.most_common(1)[0][0] if counter else "NONE"
    dominant_cn = event_to_cn(dominant_event)

    peak_hour = hours.most_common(1)[0][0] if hours else None
    peak_text = f"{peak_hour}:00-{peak_hour}:59" if peak_hour else "--"

    trend = "平稳"
    if len(scores) >= 6:
        half = len(scores) // 2
        new_avg = sum(scores[:half]) / max(1, half)
        old_avg = sum(scores[half:]) / max(1, len(scores) - half)

        if new_avg - old_avg >= 8:
            trend = "上升"
        elif old_avg - new_avg >= 8:
            trend = "下降"

    response_rate = round(feedback_active_count * 100 / total, 1) if total else 0

    if high_ratio >= 60:
        stability = "波动大"
    elif high_ratio >= 30:
        stability = "中等"
    else:
        stability = "稳定"

    tags = []
    if dominant_event not in ("NONE", "UNKNOWN"):
        tags.append(f"主导风险：{dominant_cn}")

    tags.append("高风险驾驶倾向" if high_ratio >= 60 else "中风险驾驶倾向" if high_ratio >= 30 else "低风险驾驶倾向")
    tags.append("近期风险上升" if trend == "上升" else "近期风险下降" if trend == "下降" else "风险相对平稳")

    if counter.get("HEAD_DOWN", 0) > 0:
        tags.append("注意力分散倾向")
    if counter.get("WHEEL_OFF", 0) > 0:
        tags.append("手离盘倾向")
    if counter.get("PHONE_CALL", 0) > 0:
        tags.append("通讯干扰风险")
    if counter.get("MOUTH_YAWN", 0) > 0:
        tags.append("疲劳驾驶风险")
    if counter.get("SMOKING", 0) > 0:
        tags.append("抽烟干扰风险")

    tags.append("反馈链路活跃" if response_rate >= 50 else "需增强闭环干预")

    top3 = [
        {"name": event_to_cn(name), "count": count}
        for name, count in counter.most_common(3)
    ]

    summary = (
        f"该驾驶员当前主导风险类型为“{dominant_cn}”，高风险事件占比约 {high_ratio}%，"
        f"主要风险高发时段集中在 {peak_text}。系统判断其近期风险趋势为“{trend}”，"
        f"反馈链路响应率约为 {response_rate}%，整体行为稳定性为“{stability}”。"
        f"建议保持分级预警策略，并优先对“{dominant_cn}”相关行为进行持续干预。"
    )

    return {
        "first_seen": first_seen,
        "last_seen": last_seen,
        "total_requests": total,
        "high_risk_count": high,
        "high_risk_ratio": f"{high_ratio}%",
        "avg_ai_latency": driver_profile["avg_latency"],
        "avg_latency": driver_profile["avg_latency"],
        "dominant_risk_type": dominant_cn,
        "peak_risk_hour": peak_text,
        "trend_label": trend,
        "response_rate": f"{response_rate}%",
        "stability_label": stability,
        "tags": tags,
        "top3_behaviors": top3,
        "summary": summary,
        "last_intervention": driver_profile["last_intervention"],
        "last_recovery": driver_profile["last_recovery"],
        "current_policy": driver_profile["current_policy"],
        "top_events": {event_to_cn(k): v for k, v in counter.most_common(6)},
    }


# =========================
# DeepSeek
# =========================

def build_prompt(question, context):
    return f"""
你是智能座舱安全系统的云端认知大脑 DeepSeek。
你不是普通聊天助手，而是负责根据端侧风险事件、车辆状态、驾驶员画像和反馈动作做认知分析。

请遵守：
1. 不要声称直接控制刹车、油门、转向。
2. 只能建议或确认座舱级动作：语音提醒、红屏警告、风扇增强、驾驶员画像更新、云端记录。
3. 强调端侧硬规则始终保底。
4. 输出尽量简洁，适合车载屏幕和远程 Dashboard 展示。

用户任务：
{question}

端侧上下文：
{context}
"""


def ask_deepseek(question, context):
    prompt = build_prompt(question, context)
    start = time.time()

    resp = client.chat.completions.create(
        model=MODEL_NAME,
        messages=[
            {"role": "system", "content": "You are a cloud cognitive safety agent for an intelligent cockpit."},
            {"role": "user", "content": prompt}
        ],
        temperature=0.3,
        max_tokens=650,
    )

    elapsed = time.time() - start
    answer = resp.choices[0].message.content.strip()
    latency = f"{elapsed:.2f}s"
    return answer, latency


# =========================
# 上下文解析
# =========================

def parse_context_to_state(context):
    """
    从开发板传来的 context 文本中提取风险信息。
    已修复：优先解析 ACTION_STATUS 里的 Event:xxx。
    """
    text = str(context or "")

    action_status = "Action: --"
    m = re.search(r"ACTION_STATUS=([^\n]+)", text)
    if m:
        action_status = m.group(1).strip()

    # 兼容 latest edge action status 文本
    m = re.search(r"Last Edge Action Status[:：]\s*([^\n]+)", text, re.I)
    if m and action_status == "Action: --":
        action_status = m.group(1).strip()

    score = 0
    patterns = [
        r"Risk Score[:： ]+(\d+)",
        r"risk score[:： ]+(\d+)",
        r"RISK_SCORE=(\d+)",
        r"SCORE=(\d+)",
        r"score[:： ]+(\d+)",
        r"风险分数[:： ]*(\d+)",
    ]
    for p in patterns:
        m = re.search(p, text, re.I)
        if m:
            score = safe_int(m.group(1))
            break

    # 车辆状态：优先显式字段
    vehicle = "UNKNOWN"
    vehicle_patterns = [
        r"Vehicle Mode[:： ]+([A-Za-z_]+)",
        r"vehicle_mode[:= ]+([A-Za-z_]+)",
        r"VEHICLE[_ ]?MODE[:= ]+([A-Za-z_]+)",
        r"VEHICLE=([A-Za-z_]+)",
    ]
    for p in vehicle_patterns:
        matches = re.findall(p, text, re.I)
        if matches:
            vehicle = normalize_vehicle_mode(matches[-1])
            break

    if vehicle == "UNKNOWN":
        # 兜底：取文本里最后一次出现的状态，避免历史记录中的 DRIVING 抢占当前 PARKED
        matches = re.findall(r"\b(DRIVING|MOVING|PARKED|PARK|STOP|AUTO)\b", text, re.I)
        if matches:
            vehicle = normalize_vehicle_mode(matches[-1])

    level = "0"
    level_patterns = [
        r"RISK_LEVEL=(\d+)",
        r"Risk Level[: ]+(\d+)",
        r"Current Risk Level[:： ]+(\d+)",
    ]
    for p in level_patterns:
        m = re.search(p, text, re.I)
        if m:
            level = m.group(1)
            break

    if level == "0":
        level = parse_action_level(action_status, "0")

    cognitive_score = "0"
    m = re.search(r"COGNITIVE_SCORE=(\d+)", text, re.I)
    if m:
        cognitive_score = m.group(1)

    raw_event = "NONE"
    event_patterns = [
        r"EVENT=([A-Z_]+)",
        r"Latest Risk[:： ]+([A-Z_]+)",
        r"latest_event[:= ]+([A-Z_]+)",
    ]
    for p in event_patterns:
        matches = re.findall(p, text, re.I)
        if matches:
            raw_event = matches[-1]
            break

    if raw_event == "NONE":
        # 兜底：取最后一个风险关键词
        found = []
        for key in ["HEAD_DOWN", "WHEEL_OFF", "PHONE_CALL", "SMOKING", "SMOKE", "MOUTH_RISK", "MOUTH_YAWN"]:
            for _ in re.finditer(key, text, re.I):
                found.append(key)
        if found:
            raw_event = found[-1]

    event = choose_event(raw_event, action_status, score)

    return {
        "vehicle_mode": vehicle,
        "risk_score": score,
        "event": event,
        "source": "RK3588",
        "cognitive_level": str(level),
        "cognitive_score": str(cognitive_score),
        "action_status": normalize_action_status(action_status),
        "updated_at": time.time(),
    }


# =========================
# 统计 / Dashboard 数据
# =========================

def get_event_stats(normalized_cache):
    event_counter = Counter()
    level_counter = Counter()
    action_counter = Counter()

    for item in normalized_cache:
        event = canonical_event(item.get("event", "NONE"))
        if event not in ("NONE", "UNKNOWN", "--"):
            event_counter[event] += 1

        level = str(item.get("cognitive_level", "0"))
        level_counter[level] += 1

        action = str(item.get("action_status", ""))
        if "VOICE" in action or "语音" in action:
            action_counter["语音提醒"] += 1
        if "RED_FLASH" in action or "RED" in action or "红屏" in action:
            action_counter["红屏警告"] += 1
        if "FAN_BOOST" in action or "FAN" in action or "风扇" in action:
            action_counter["风扇增强"] += 1
        if "RECOVERED" in action:
            action_counter["恢复正常"] += 1

    return event_counter, level_counter, action_counter


def build_charts(normalized_cache):
    # 用统一后的缓存重新计算图表，避免 charts 还是 HEAD_DOWN、动作却是 WHEEL_OFF
    event_counter, level_counter, action_counter = get_event_stats(normalized_cache)

    latest_20 = list(reversed(normalized_cache[:20]))

    event_order = ["HEAD_DOWN", "WHEEL_OFF", "PHONE_CALL", "MOUTH_RISK", "HIGH_RISK"]

    return {
        "times": [x.get("time", "--")[-8:] for x in latest_20],
        "scores": [safe_int(x.get("risk_score", 0)) for x in latest_20],
        "levels": [safe_int(x.get("cognitive_level", 0)) for x in latest_20],
        "event_names": event_order,
        "event_values": [event_counter.get(e, 0) for e in event_order],
        "level_names": [f"Level {k}" for k in level_counter.keys()],
        "level_values": list(level_counter.values()),
        "action_names": list(action_counter.keys()),
        "action_values": list(action_counter.values()),
    }


def build_dashboard_data():
    # 最新缓存放前面
    normalized_cache = [normalize_record(x) for x in list(vehicle_cache)][::-1]

    live_valid = time.time() - EDGE_LIVE_STATE.get("updated_at", 0) <= 5

    if live_valid:
        action_status = normalize_action_status(EDGE_LIVE_STATE.get("action_status", latest_state.get("action_status", "")))
        latest_score = safe_int(EDGE_LIVE_STATE.get("risk_score", 0))
        latest_event = choose_event(EDGE_LIVE_STATE.get("event", "NONE"), action_status, latest_score)
        latest_vehicle_mode = normalize_vehicle_mode(EDGE_LIVE_STATE.get("vehicle_mode", "UNKNOWN"))
        latest_level = str(EDGE_LIVE_STATE.get("cognitive_level", parse_action_level(action_status, latest_state.get("cognitive_level", "0"))))
        latest_cognitive_score = str(EDGE_LIVE_STATE.get("cognitive_score", latest_state.get("cognitive_score", "0")))
        latest_fan_action = EDGE_LIVE_STATE.get("fan_action", latest_state.get("fan_action", "NONE"))
        latest_fan_state = EDGE_LIVE_STATE.get("fan_state", latest_state.get("fan_state", "NORMAL"))
    else:
        latest = normalized_cache[0] if normalized_cache else normalize_record(latest_state)
        action_status = normalize_action_status(latest.get("action_status", latest_state.get("action_status", "")))
        latest_score = safe_int(latest.get("risk_score", latest_state.get("risk_score", 0)))
        latest_event = choose_event(latest.get("event", latest_state.get("event", "NONE")), action_status, latest_score)
        latest_vehicle_mode = normalize_vehicle_mode(latest.get("vehicle_mode", latest_state.get("vehicle_mode", "UNKNOWN")))
        latest_level = str(latest.get("cognitive_level", latest_state.get("cognitive_level", "0")))
        latest_cognitive_score = str(latest.get("cognitive_score", latest_state.get("cognitive_score", "0")))
        latest_fan_action = latest.get("fan_action", latest_state.get("fan_action", "NONE"))
        latest_fan_state = latest.get("fan_state", latest_state.get("fan_state", "NORMAL"))

    # 同步 latest_state，方便调试
    latest_state.update({
        "vehicle_mode": latest_vehicle_mode,
        "risk_score": latest_score,
        "event": latest_event,
        "cognitive_level": latest_level,
        "cognitive_score": latest_cognitive_score,
        "action_status": action_status,
        "updated_at": time.time(),
        "fan_action": latest_fan_action,
        "fan_state": latest_fan_state,
    })

    charts = build_charts(normalized_cache)
    deep_profile = build_deep_driver_profile(normalized_cache)

    return {
        "cloud_status": cloud_status,
        "mqtt_status": mqtt_status,
        "latency": latest_latency,

        "vehicle_mode": latest_vehicle_mode,
        "risk_score": latest_score,
        "event": latest_event,
        "latest_event": latest_event,
        "latest_event_cn": event_to_cn(latest_event),
        "dominant_event_cn": deep_profile.get("dominant_risk_type", "--"),

        "cognitive_level": latest_level,
        "cognitive_score": latest_cognitive_score,
        "action_status": action_status,
        "intervention_mode": latest_state.get("intervention_mode", "NORMAL"),

        "latest_answer": latest_answer,
        "latest_context": latest_context,

        "vehicle_cache": normalized_cache[:20],

        "driver_profile": deep_profile,

        "charts": charts,

        "fan_action": latest_fan_action,
        "fan_state": latest_fan_state,

        # 调试字段：网页同步诊断会用到
        "debug_sync": {
            "live_valid": live_valid,
            "edge_live_state": EDGE_LIVE_STATE,
            "latest_state": latest_state,
            "cache_len": len(vehicle_cache),
        }
    }


# =========================
# API 路由
# =========================

@app.route("/ping", methods=["GET"])
def ping():
    return jsonify({
        "status": "ok",
        "message": "edge cognitive agent running"
    })


@app.route("/ask", methods=["POST"])
def ask():
    global latest_answer, latest_latency, cloud_status, latest_context, latest_state

    data = request.get_json(force=True, silent=True) or {}
    question = data.get("question", "")
    context = data.get("context", "")

    latest_context = context if context else "暂无边缘端上下文"

    parsed = parse_context_to_state(context)
    latest_state.update(parsed)

    record = normalize_record({
        "time": now_str(),
        "vehicle_mode": latest_state.get("vehicle_mode", "UNKNOWN"),
        "risk_score": latest_state.get("risk_score", 0),
        "event": latest_state.get("event", "NONE"),
        "source": "RK3588",
        "cognitive_level": latest_state.get("cognitive_level", "0"),
        "cognitive_score": latest_state.get("cognitive_score", "0"),
        "action_status": latest_state.get("action_status", "Action: --"),
    })

    if should_append_record(record):
        vehicle_cache.append(record)
        update_driver_profile(record["event"], record["risk_score"])

    try:
        answer, latency = ask_deepseek(question, context)
        latest_answer = answer
        latest_latency = latency
        cloud_status = "ONLINE"

        update_driver_profile(record["event"], record["risk_score"], latency)

        # 根据回答内容粗略更新策略状态
        answer_upper = answer.upper()

        if "AI_HIGH_SENSITIVITY" in answer_upper:
            driver_profile["current_policy"] = "AI_HIGH_SENSITIVITY"
        if "FAN_BOOST" in answer_upper or "FAN" in answer_upper:
            driver_profile["last_intervention"] = "FAN_BOOST"
        if "RECOVER" in answer_upper or "RECOVERED" in answer_upper:
            driver_profile["last_recovery"] = now_str()

        return jsonify({
            "answer": answer,
            "latency": latency,
            "cloud_status": "ONLINE"
        })

    except Exception as e:
        cloud_status = "OFFLINE"
        latest_latency = "--"
        latest_answer = (
            "[离线降级]\n"
            "云端 AI 暂不可用，端侧感知、分级预警和事件记录仍保持工作。\n"
            f"错误信息：{e}"
        )

        return jsonify({
            "answer": latest_answer,
            "latency": "--",
            "cloud_status": "OFFLINE"
        })


@app.route("/status", methods=["POST"])
def status():
    """
    兼容旧接口：开发板如果 POST 到 /status，也能同步。
    推荐新脚本 POST 到 /edge/state。
    """
    global latest_state, latest_context

    data = request.get_json(force=True, silent=True) or {}

    action_status = normalize_action_status(data.get("action_status", latest_state.get("action_status", "")))
    score = safe_int(data.get("risk_score", latest_state.get("risk_score", 0)))
    event = choose_event(data.get("event", latest_state.get("event", "NONE")), action_status, score)

    latest_state.update({
        "vehicle_mode": normalize_vehicle_mode(data.get("vehicle_mode", latest_state.get("vehicle_mode", "UNKNOWN"))),
        "risk_score": score,
        "event": event,
        "source": data.get("source", "RK3588"),
        "cognitive_level": str(data.get("cognitive_level", parse_action_level(action_status, latest_state.get("cognitive_level", "0")))),
        "cognitive_score": str(data.get("cognitive_score", latest_state.get("cognitive_score", "0"))),
        "action_status": action_status,
        "updated_at": time.time(),
    })

    record = normalize_record({
        "time": now_str(),
        "vehicle_mode": latest_state.get("vehicle_mode", "UNKNOWN"),
        "risk_score": latest_state.get("risk_score", 0),
        "event": latest_state.get("event", "NONE"),
        "source": latest_state.get("source", "RK3588"),
        "cognitive_level": latest_state.get("cognitive_level", "0"),
        "cognitive_score": latest_state.get("cognitive_score", "0"),
        "action_status": latest_state.get("action_status", "Action: --"),
    })

    if should_append_record(record):
        vehicle_cache.append(record)
        update_driver_profile(record["event"], record["risk_score"])

    return jsonify({"status": "ok", "state": latest_state})


@app.route("/edge/state", methods=["POST"])
def edge_state():
    """
    新增接口：开发板 push_state_to_pc.py 每秒 POST 到这里。
    用于解决开发板 UI 已变成 PARKED，但 PC Dashboard 仍显示 DRIVING 的问题。
    """
    data = request.get_json(force=True, silent=True) or {}

    action_status = normalize_action_status(data.get("action_status", ""))
    score = safe_int(data.get("risk_score", 0))
    event = choose_event(data.get("event", "NONE"), action_status, score)
    mode = normalize_vehicle_mode(data.get("vehicle_mode", "UNKNOWN"))

    EDGE_LIVE_STATE.update({
        "updated_at": time.time(),
        "event": event,
        "risk_score": score,
        "vehicle_mode": mode,
        "action_status": action_status,
        "source": data.get("source", "RK3588"),
        "cognitive_level": str(data.get("cognitive_level", parse_action_level(action_status, "0"))),
        "cognitive_score": str(data.get("cognitive_score", "0")),
        "fan_action": data.get("fan_action", EDGE_LIVE_STATE.get("fan_action", "NONE")),
        "fan_state": data.get("fan_state", EDGE_LIVE_STATE.get("fan_state", "NORMAL")),
    })

    latest_state.update({
        "vehicle_mode": mode,
        "risk_score": score,
        "event": event,
        "source": data.get("source", "RK3588"),
        "cognitive_level": EDGE_LIVE_STATE["cognitive_level"],
        "cognitive_score": EDGE_LIVE_STATE["cognitive_score"],
        "action_status": action_status,
        "updated_at": time.time(),
        "fan_action": EDGE_LIVE_STATE["fan_action"],
        "fan_state": EDGE_LIVE_STATE["fan_state"],
    })

    record = normalize_record({
        "time": data.get("timestamp", now_str()),
        "vehicle_mode": mode,
        "risk_score": score,
        "event": event,
        "source": data.get("source", "RK3588"),
        "cognitive_level": EDGE_LIVE_STATE["cognitive_level"],
        "cognitive_score": EDGE_LIVE_STATE["cognitive_score"],
        "action_status": action_status,
    })

    if should_append_record(record):
        vehicle_cache.append(record)
        update_driver_profile(record["event"], record["risk_score"])

    return jsonify({
        "ok": True,
        "state": EDGE_LIVE_STATE
    })


@app.route("/api/dashboard", methods=["GET"])
def api_dashboard():
    return jsonify(build_dashboard_data())


# =========================
# Dashboard 页面
# =========================

@app.route("/dashboard", methods=["GET"])
def dashboard():
    """
    优先读取 dashboard_neo.html。
    如果你还没改名，也兼容 v5 / v4 文件名。
    """
    candidates = [
        "dashboard_neo.html",
        "dashboard_guosai_sync_v5.html",
        "dashboard_guosai_final_v4.html",
    ]

    for name in candidates:
        html_path = Path(__file__).with_name(name)
        if html_path.exists():
            return Response(html_path.read_text(encoding="utf-8"), mimetype="text/html")

    return Response(
        "Dashboard HTML not found. Please put dashboard_neo.html in D:\\deepseek_proxy\\",
        mimetype="text/plain"
    )


if __name__ == "__main__":
    print("Starting Edge Cognitive Agent...")
    print("Dashboard: http://127.0.0.1:5000/dashboard")
    print("API:       http://127.0.0.1:5000/api/dashboard")
    print("Edge POST: http://127.0.0.1:5000/edge/state")
    app.run(host="0.0.0.0", port=5000, debug=False)
