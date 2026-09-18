# CC2530 智慧城市课设

- `firmware_terminal`：小板环境终端；IAR 打开 `Project/rf_set.ewp`。
- `firmware_gateway`：大板网关；IAR 打开 `Project/rf_set.ewp`。
- `python_server`：手机网页、TCP 网关与 DeepSeek Agent。

## 接线

| 模块 | 引脚 | 状态 |
| --- | --- | --- |
| DHT11 数据 | P0.0 | 数字数据 |
| 人体红外 DO | P0.4 | 高电平有人 |
| 继电器 IN | P0.5 | 高电平吸合 |
| 蜂鸣器 | P0.7 | 低电平响 |
| 光敏 DO | P1.4 | 低电平环境暗 |
| MQ-2 DO | P1.5 | 低电平检测烟雾 |

所有模块与 CC2530 共地，继电器只接低压演示负载。

## 启动

1. 手机开热点，电脑和 Wi-Fi 网关接入热点；网关配置为 TCP Server，并把拨码切到 `ZigBee <-> Wi-Fi` 透传。
2. 复制 `python_server/.env.example` 为 `.env`，填写 DeepSeek Key 和网关 IP。
3. 在 `python_server` 运行 `python -m pip install -r requirements.txt`，再运行 `python app.py`。
4. 手机访问电脑热点 IP 的 `:5000` 端口，例如 `http://192.168.43.101:5000`。

硬件每秒上报状态。自动模式中：烟雾强制蜂鸣和警示 LED，夜间有人时接通路灯继电器，离开 15 秒后关灯。网页或 Agent 需先切换手动模式再控制设备。
