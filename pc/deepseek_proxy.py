from flask import Flask, request, jsonify
from openai import OpenAI
import os

app = Flask(__name__)

client = OpenAI(
    api_key=os.environ.get("DEEPSEEK_API_KEY"),
    base_url="https://api.deepseek.com"
)

SYSTEM_PROMPT = """
你是车载智能座舱安全助手。
系统基于 RK3588,包含 YOLOv8-Pose 姿态检测、RetinaFace 人脸检测、
MPU6050 车辆运动状态判断、语音预警、环境监测、风扇控制和 CSV 历史记录。
请用中文简洁回答,回答不要超过100字。
"""

@app.route("/ask", methods=["POST"])
def ask():
    data = request.get_json(force=True)

    question = data.get("question", "")
    context = data.get("context", "")

    if not question:
        return jsonify({"answer": "没有收到问题。"})

    try:
        user_prompt = f"""
当前系统记录：
{context}

用户问题：
{question}
"""

        resp = client.chat.completions.create(
            model="deepseek-chat",
            messages=[
                {"role": "system", "content": SYSTEM_PROMPT},
                {"role": "user", "content": user_prompt}
            ],
            temperature=0.3,
            max_tokens=180,
            stream=False
        )

        answer = resp.choices[0].message.content.strip()
        return jsonify({"answer": answer})

    except Exception as e:
        return jsonify({
            "answer": "云端AI服务暂时不可用,请检查网络或API配置。",
            "error": str(e)
        }), 500


@app.route("/ping", methods=["GET"])
def ping():
    return jsonify({"status": "ok", "message": "deepseek proxy running"})


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000)