import json
import time

import paho.mqtt.client as mqtt

from MyClass.pymata4EX import Pymata4EX


TRIG_PIN = 7
ECHO_PIN = 9

MQTT_HOST = "127.0.0.1"
MQTT_PORT = 1883
DISTANCE_TOPIC = "ultrasonic/distance"
MQTT_CLIENT_ID = "ultrasonic_sensor"
POLL_INTERVAL = 1


def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("成功连接到MQTT服务器")
    else:
        print("连接MQTT服务器失败，错误代码:", rc)


def on_publish(client, userdata, mid):
    print("消息发布成功，消息ID:", mid)


def main():
    board = None
    client = mqtt.Client(client_id=MQTT_CLIENT_ID)
    client.on_connect = on_connect
    client.on_publish = on_publish

    try:
        client.connect(MQTT_HOST, MQTT_PORT, 60)
        client.loop_start()
    except Exception as exc:
        print("连接MQTT服务器时出错:", exc)
        return

    try:
        board = Pymata4EX()
        board.set_pin_mode_sonar(TRIG_PIN, ECHO_PIN)
        print("成功连接到Arduino，超声波传感器已设置")
    except Exception as exc:
        print("连接Arduino或设置超声波传感器时出错:", exc)
        client.loop_stop()
        client.disconnect()
        return

    print("程序运行中：正在发布 ultrasonic/distance")

    try:
        while True:
            distance, event_time = board.sonar_read(TRIG_PIN)

            if event_time != 0 and distance > 0:
                sensor_data = {
                    "distance": distance,
                    "unit": "cm"
                }
                payload = json.dumps(sensor_data)
                client.publish(DISTANCE_TOPIC, payload)
                print("已发布:", payload)
            else:
                print("等待超声波传感器数据...")

            time.sleep(POLL_INTERVAL)
    except KeyboardInterrupt:
        print("程序停止")
    finally:
        client.loop_stop()
        client.disconnect()
        if board is not None:
            board.shutdown()


if __name__ == "__main__":
    main()
