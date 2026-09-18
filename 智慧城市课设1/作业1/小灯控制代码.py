import json
import time

import paho.mqtt.client as mqtt

from MyClass import pymata4EX


LED_PINS = {
    "r": 5,
    "g": 10,
    "b": 6,
}

MQTT_HOST = "127.0.0.1"
MQTT_PORT = 1883
CONTROL_TOPIC = "rgb/control"
STATUS_TOPIC = "rgb/status"


board = pymata4EX.Pymata4EX()
client = mqtt.Client()
led_states = {
    "r": 0,
    "g": 0,
    "b": 0,
}


def publish_status():
    payload = json.dumps({
        "r": led_states["r"],
        "g": led_states["g"],
        "b": led_states["b"],
        "text": (
            f"红灯:{'开' if led_states['r'] else '关'} "
            f"绿灯:{'开' if led_states['g'] else '关'} "
            f"蓝灯:{'开' if led_states['b'] else '关'}"
        ),
    })
    client.publish(STATUS_TOPIC, payload)
    print("状态已发布:", payload)


def set_led(color, state):
    led_states[color] = state
    board.digital_pin_write(LED_PINS[color], state)
    publish_status()


def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("MQTT 已连接")
        client.subscribe(CONTROL_TOPIC)
        publish_status()
    else:
        print("MQTT 连接失败，错误码:", rc)


def on_message(client, userdata, msg):
    try:
        data = json.loads(msg.payload.decode("utf-8"))
        color = str(data.get("color", "")).lower()
        state = int(data.get("state", 0))
    except Exception as exc:
        print("命令解析失败:", exc)
        return

    if color not in LED_PINS:
        print("未知灯名:", color)
        return

    if state not in (0, 1):
        print("未知状态:", state)
        return

    print(f"收到控制命令: {color} -> {state}")
    set_led(color, state)


def main():
    for pin in LED_PINS.values():
        board.set_pin_mode_digital_output(pin)
        board.digital_pin_write(pin, 0)

    client.on_connect = on_connect
    client.on_message = on_message
    client.connect(MQTT_HOST, MQTT_PORT, 60)
    client.loop_start()

    print("程序运行中：Node-RED 发送 rgb/control，Python 返回 rgb/status")
    print("接线参考：R接D5，G接D10，B接D6")

    try:
        while True:
            publish_status()
            time.sleep(2)
    except KeyboardInterrupt:
        print("程序停止")
    finally:
        for pin in LED_PINS.values():
            board.digital_pin_write(pin, 0)
        client.loop_stop()
        client.disconnect()
        board.shutdown()


if __name__ == "__main__":
    main()
