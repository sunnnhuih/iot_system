import json
import os
import socket
import threading
import time
from collections import deque
from datetime import datetime

from dotenv import load_dotenv
from flask import Flask, jsonify, render_template, request

load_dotenv()
app = Flask(__name__)
GATEWAY_HOST = os.getenv("GATEWAY_HOST", "192.168.43.100")
GATEWAY_PORT = int(os.getenv("GATEWAY_PORT", "8080"))

lock = threading.Lock()
socket_lock = threading.Lock()
gateway_socket = None
state = {"temperature": None, "humidity": None, "smoke": False, "dark": False,
         "human": False, "automatic": True, "relay": False, "buzzer": False,
         "led": False, "updated_at": None, "connected": False}
events = deque(maxlen=20)


def stamp():
    return datetime.now().strftime("%H:%M:%S")


def add_event(level, message):
    with lock:
        events.appendleft({"time": stamp(), "level": level, "message": message})


def snapshot():
    with lock:
        return dict(state)


def parse_status(line):
    parts = line.strip().split(",")
    if len(parts) != 10 or parts[0] != "S":
        return
    try:
        values = [int(item) for item in parts[1:]]
    except ValueError:
        return
    with lock:
        previous_smoke = state["smoke"]
        previous_hot = state["temperature"] is not None and state["temperature"] >= 35
        previous_humid = state["humidity"] is not None and state["humidity"] >= 80
        state.update({"temperature": values[0], "humidity": values[1], "smoke": bool(values[2]),
                      "dark": bool(values[3]), "human": bool(values[4]), "automatic": bool(values[5]),
                      "relay": bool(values[6]), "buzzer": bool(values[7]), "led": bool(values[8]),
                      "updated_at": stamp(), "connected": True})
        smoke = state["smoke"]
        hot = state["temperature"] >= 35
        humid = state["humidity"] >= 80
    if smoke and not previous_smoke:
        add_event("danger", "检测到烟雾，硬件已进入告警状态")
    if hot and not previous_hot:
        add_event("warning", "温度达到或超过 35°C")
    if humid and not previous_humid:
        add_event("warning", "湿度达到或超过 80%")


def gateway_loop():
    global gateway_socket
    while True:
        sock = None
        try:
            sock = socket.create_connection((GATEWAY_HOST, GATEWAY_PORT), timeout=5)
            sock.settimeout(1)
            with socket_lock:
                gateway_socket = sock
            with lock:
                state["connected"] = True
            add_event("info", "已连接 CC2530 Wi-Fi 网关")
            pending = ""
            while True:
                try:
                    chunk = sock.recv(1024)
                except socket.timeout:
                    continue
                if not chunk:
                    raise ConnectionError("gateway disconnected")
                pending += chunk.decode("utf-8", errors="ignore")
                while "\n" in pending:
                    line, pending = pending.split("\n", 1)
                    parse_status(line)
        except OSError:
            with lock:
                state["connected"] = False
            time.sleep(3)
        finally:
            with socket_lock:
                if gateway_socket is sock:
                    gateway_socket = None
            if sock:
                sock.close()


def send_command(target, value):
    if target not in {"MODE", "RELAY", "BUZZER", "LED"}:
        raise ValueError("未知控制目标")
    value = 1 if bool(value) else 0
    with socket_lock:
        if gateway_socket is None:
            raise ConnectionError("网关未连接")
        gateway_socket.sendall(f"C,{target},{value}\n".encode("ascii"))
    labels = {"MODE": "自动模式", "RELAY": "路灯继电器", "BUZZER": "蜂鸣器", "LED": "警示 LED"}
    add_event("control", f"{labels[target]}设置为{'开启' if value else '关闭'}")
    return {"target": target, "value": value, "message": "命令已发送到网关"}


TOOLS = [
    {"type": "function", "function": {"name": "get_current_status", "description": "读取当前硬件状态", "parameters": {"type": "object", "properties": {}}}},
    {"type": "function", "function": {"name": "set_control_mode", "description": "设置自动模式或手动模式", "parameters": {"type": "object", "properties": {"automatic": {"type": "boolean"}}, "required": ["automatic"]}}},
    {"type": "function", "function": {"name": "set_relay", "description": "在手动模式控制路灯继电器", "parameters": {"type": "object", "properties": {"on": {"type": "boolean"}}, "required": ["on"]}}},
    {"type": "function", "function": {"name": "set_buzzer", "description": "在手动模式控制蜂鸣器", "parameters": {"type": "object", "properties": {"on": {"type": "boolean"}}, "required": ["on"]}}},
    {"type": "function", "function": {"name": "set_indicator_led", "description": "在手动模式控制警示LED", "parameters": {"type": "object", "properties": {"on": {"type": "boolean"}}, "required": ["on"]}}},
]


def execute_tool(name, args):
    if name == "get_current_status":
        return snapshot()
    mapping = {"set_control_mode": ("MODE", "automatic"), "set_relay": ("RELAY", "on"),
               "set_buzzer": ("BUZZER", "on"), "set_indicator_led": ("LED", "on")}
    if name not in mapping:
        raise ValueError("不支持的工具")
    target, key = mapping[name]
    return send_command(target, args[key])


def run_agent(user_message):
    api_key = os.getenv("DEEPSEEK_API_KEY")
    if not api_key:
        raise RuntimeError("未配置 DEEPSEEK_API_KEY")
    from openai import OpenAI
    client = OpenAI(api_key=api_key, base_url="https://api.deepseek.com")
    messages = [
        {"role": "system", "content": "你是智慧城市设备控制助手。仅在用户发消息时响应，不能自行监测、轮询或决定策略。用户要求控制设备时，若当前是自动模式，先调用set_control_mode设为false。简洁说明真实的工具执行结果。"},
        {"role": "user", "content": user_message},
    ]
    call_log = []
    for _ in range(3):
        response = client.chat.completions.create(model="deepseek-flash", messages=messages, tools=TOOLS, tool_choice="auto")
        message = response.choices[0].message
        if not message.tool_calls:
            return {"reply": message.content or "操作完成。", "calls": call_log}
        serialized_calls = []
        for call in message.tool_calls:
            serialized_calls.append({"id": call.id, "type": "function", "function": {"name": call.function.name, "arguments": call.function.arguments}})
        messages.append({"role": "assistant", "content": message.content, "tool_calls": serialized_calls})
        for call in message.tool_calls:
            try:
                result = execute_tool(call.function.name, json.loads(call.function.arguments))
            except Exception as exc:
                result = {"error": str(exc)}
            call_log.append({"tool": call.function.name, "arguments": call.function.arguments, "result": result})
            messages.append({"role": "tool", "tool_call_id": call.id, "content": json.dumps(result, ensure_ascii=False)})
    return {"reply": "工具调用已完成。", "calls": call_log}


@app.get("/")
def index():
    return render_template("index.html")


@app.get("/api/status")
def get_status():
    return jsonify(snapshot())


@app.get("/api/events")
def get_events():
    with lock:
        return jsonify(list(events))


@app.post("/api/control")
def control():
    data = request.get_json(silent=True) or {}
    try:
        return jsonify(send_command(str(data["target"]), data["value"]))
    except (KeyError, ValueError, ConnectionError) as exc:
        return jsonify({"error": str(exc)}), 400


@app.post("/api/agent/chat")
def agent_chat():
    data = request.get_json(silent=True) or {}
    message = str(data.get("message", "")).strip()
    if not message:
        return jsonify({"error": "请输入消息"}), 400
    try:
        return jsonify(run_agent(message))
    except Exception as exc:
        return jsonify({"error": str(exc)}), 502


if __name__ == "__main__":
    threading.Thread(target=gateway_loop, daemon=True).start()
    app.run(host="0.0.0.0", port=int(os.getenv("WEB_PORT", "5000")), debug=False)
