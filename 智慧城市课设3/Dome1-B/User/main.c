#include "stm32f10x.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "delay.h"
#include "bsp_usart.h"
#include "oled.h"
#include "cs_lora.h"
#include "bsp_esp8266.h"
#include "bsp_io.h"

#define CITY_WIFI_SSID       "CITY_LIGHT"
#define CITY_WIFI_PASSWORD   "12345678"
#define CITY_WIFI_IP         "192.168.4.2"
#define CITY_WIFI_PORT       "5000"
#define CITY_WIFI_TIMEOUT    "1800"
#define SMOKE_ALARM_LEVEL    1800
#define LORA_TIMEOUT_MS      6000

static uint8_t g_wifi_ready = 0;
static ENUM_ID_NO_TypeDef g_wifi_id = Multiple_ID_0;
static uint8_t g_lora_online = 0;
static uint8_t g_auto_mode = 1;
static uint8_t g_relay_on = 0;
static uint8_t g_pir = 0;
static uint8_t g_smoke_alarm = 0;
static uint16_t g_smoke = 0;
static uint16_t g_light = 0;
static int g_temp_int = 0;
static int g_temp_deci = 0;
static int g_humi_int = 0;
static int g_humi_deci = 0;
static u32 g_lora_silence_ms = 0;
static char g_lora_tx[128];
static char g_wifi_tx[192];
static char g_field[32];
static char g_line[32];
static char g_lora_frame[160];
static char g_wifi_frame[512];

static void Clear_Esp8266_Rx(void)
{
	NVIC_DisableIRQ(USART3_IRQn);
	Esp8266_rx_length = 0;
	Esp8266_rx_flag = 0;
	Esp8266_rx_buf[0] = '\0';
	NVIC_EnableIRQ(USART3_IRQn);
}

static uint8_t Take_Esp8266_Frame(char *dst, uint16_t dst_size)
{
	uint16_t i;
	uint16_t len;
	uint8_t ready = 0;

	NVIC_DisableIRQ(USART3_IRQn);
	if (Esp8266_rx_flag != 0)
	{
		len = (uint16_t)Esp8266_rx_length;
		if (len >= dst_size)
		{
			len = dst_size - 1;
		}
		for (i = 0; i < len; i++)
		{
			dst[i] = Esp8266_rx_buf[i];
		}
		dst[len] = '\0';
		ready = (len != 0) ? 1 : 0;
		Esp8266_rx_length = 0;
		Esp8266_rx_flag = 0;
		Esp8266_rx_buf[0] = '\0';
	}
	NVIC_EnableIRQ(USART3_IRQn);
	return ready;
}

static uint8_t Take_Lora_Frame(char *dst, uint16_t dst_size)
{
	uint16_t i;
	uint16_t len;
	uint8_t ready = 0;

	NVIC_DisableIRQ(USART2_IRQn);
	if ((buf_lora.rx_flag != 0) && (buf_lora.index != 0))
	{
		len = (uint16_t)buf_lora.index;
		if (len >= dst_size)
		{
			len = dst_size - 1;
		}
		for (i = 0; i < len; i++)
		{
			dst[i] = buf_lora.buf[i];
		}
		dst[len] = '\0';
		ready = 1;
		/* Clear only after a complete frame has been copied.  Clearing the
		 * buffer on every poll would erase a frame while it is still arriving
		 * at 9600 baud (the USART IDLE flag is raised only after the frame). */
		buf_lora.index = 0;
		buf_lora.rx_flag = 0;
		buf_lora.buf[0] = '\0';
	}
	NVIC_EnableIRQ(USART2_IRQn);
	return ready;
}

static uint8_t Get_Field(const char *message, const char *key,
	char *value, uint16_t value_size)
{
	const char *start;
	uint16_t n = 0;

	start = strstr(message, key);
	if (start == 0)
	{
		return 0;
	}
	start += strlen(key);
	while ((*start != '\0') && (*start != ',') && (*start != '\r') &&
		(*start != '\n') && (n < value_size - 1))
	{
		value[n++] = *start++;
	}
	value[n] = '\0';
	return (n != 0) ? 1 : 0;
}

static void Parse_Fixed_1(const char *value, int *whole, int *deci)
{
	const char *dot;
	*whole = atoi(value);
	*deci = 0;
	dot = strchr(value, '.');
	if ((dot != 0) && (dot[1] >= '0') && (dot[1] <= '9'))
	{
		*deci = dot[1] - '0';
	}
}

static void Update_Alarm(void)
{
	g_smoke_alarm = (g_smoke >= SMOKE_ALARM_LEVEL) ? 1 : 0;
	if (g_smoke_alarm)
	{
		BEEP_ON;
	}
	else
	{
		BEEP_OFF;
	}
}

static void Draw_Gateway_Display(void)
{
	OLED_ShowString(0, 0, "CITY GATEWAY B ");
	if (!g_lora_online)
	{
		OLED_ShowString(0, 2, "LORA LOST      ");
	}
	else
	{
		OLED_ShowString(0, 2, "                ");
		sprintf(g_line, "T:%d.%d H:%d.%d", g_temp_int, g_temp_deci,
			g_humi_int, g_humi_deci);
		OLED_ShowString(0, 2, g_line);
	}
	OLED_ShowString(0, 4, "                ");
	sprintf(g_line, "S:%u L:%u", g_smoke, g_light);
	OLED_ShowString(0, 4, g_line);
	/* The last row keeps the mode and the alarm state visible at a glance. */
	OLED_ShowString(0, 6, "                ");
	sprintf(g_line, "P:%u R:%s %c AL:%u", g_pir,
		g_relay_on ? "ON" : "OFF", g_auto_mode ? 'A' : 'M', g_smoke_alarm);
	OLED_ShowString(0, 6, g_line);
}

static void Send_Phone_Status(void)
{
	if (!g_wifi_ready)
	{
		return;
	}
	sprintf(g_wifi_tx,
		"DATA,T=%d.%d,H=%d.%d,SMOKE=%u,LIGHT=%u,PIR=%u,RELAY=%u,MODE=%s,ALARM=%u",
		g_temp_int, g_temp_deci, g_humi_int, g_humi_deci,
		g_smoke, g_light, g_pir, g_relay_on,
		g_auto_mode ? "AUTO" : "MANUAL", g_smoke_alarm);
	if (!ESP8266_SendString(DISABLE, g_wifi_tx, strlen(g_wifi_tx),
		g_wifi_id))
	{
		g_wifi_ready = 0;
	}
}

static void Send_Phone_Text(const char *text)
{
	if (g_wifi_ready)
	{
		if (!ESP8266_SendString(DISABLE, (char *)text, strlen(text),
			g_wifi_id))
		{
			g_wifi_ready = 0;
		}
	}
}

static void Forward_To_Node(const char *command)
{
	sprintf(g_lora_tx, "B,CMD=%s\r\n", command);
	CSTX_Lora_Send_String(g_lora_tx);
}

static void Send_Gateway_Ping(void)
{
	/* Sent only after a complete A frame, so it does not contend with the
	 * periodic A transmission.  A uses it as a gateway liveness watchdog. */
	CSTX_Lora_Send_String("B,CMD=PING\r\n");
}

static void Handle_Phone_Command(const char *packet)
{
	const char *payload;
	char command[40];

	payload = strchr(packet, ':');
	if (payload != 0)
	{
		payload++;
	}
	else
	{
		payload = packet;
	}

	if (strstr(payload, "MODE=AUTO") != 0 || strstr(payload, "AUTO") != 0)
	{
		strcpy(command, "MODE=AUTO");
		Forward_To_Node(command);
		Send_Phone_Text("ACK,FORWARDED,MODE=AUTO");
	}
	else if (strstr(payload, "MODE=MANUAL") != 0 || strstr(payload, "MANUAL") != 0)
	{
		strcpy(command, "MODE=MANUAL");
		Forward_To_Node(command);
		Send_Phone_Text("ACK,FORWARDED,MODE=MANUAL");
	}
	else if (strstr(payload, "LIGHT=ON") != 0 || strstr(payload, "LIGHT_ON") != 0)
	{
		strcpy(command, "LIGHT=ON");
		Forward_To_Node(command);
		Send_Phone_Text("ACK,FORWARDED,LIGHT=ON");
	}
	else if (strstr(payload, "LIGHT=OFF") != 0 || strstr(payload, "LIGHT_OFF") != 0)
	{
		strcpy(command, "LIGHT=OFF");
		Forward_To_Node(command);
		Send_Phone_Text("ACK,FORWARDED,LIGHT=OFF");
	}
	else if (strstr(payload, "STATUS") != 0)
	{
		Forward_To_Node("STATUS");
		Send_Phone_Text("ACK,FORWARDED,STATUS");
	}
}

static void Update_Wifi_Id(const char *frame)
{
	const char *p;

	/* Multiple-connection ESP-AT frames are either +IPD,<id>,... or
	 * <id>,CONNECT.  Remember the client that most recently sent data so
	 * replies do not get hard-coded to connection 0 after reconnects. */
	p = strstr(frame, "+IPD,");
	if ((p != 0) && (p[5] >= '0') && (p[5] <= '4') && (p[6] == ','))
	{
		g_wifi_id = (ENUM_ID_NO_TypeDef)(p[5] - '0');
		return;
	}
	p = strstr(frame, ",CONNECT");
	if ((p != 0) && (p > frame) && (p[-1] >= '0') && (p[-1] <= '4'))
	{
		g_wifi_id = (ENUM_ID_NO_TypeDef)(p[-1] - '0');
	}
}

static void Handle_Wifi_Frame(const char *frame)
{
	/* Only a socket line is a client connection.  "WIFI CONNECTED" is an
	 * AP status message and must not trigger CIPSEND. */
	if (strstr(frame, ",CONNECT") != 0)
	{
		Update_Wifi_Id(frame);
		g_wifi_ready = 1;
		Send_Phone_Text("CITY_GATEWAY_READY");
	}
	if (strstr(frame, "CLOSED") != 0)
	{
		g_wifi_ready = 0;
	}
	if (strstr(frame, "+IPD") != 0)
	{
		Update_Wifi_Id(frame);
		g_wifi_ready = 1;
		Handle_Phone_Command(frame);
	}
}

static void Handle_Lora_Frame(const char *frame)
{
	if (strncmp(frame, "A,", 2) != 0)
	{
		return;
	}

	g_lora_online = 1;
	g_lora_silence_ms = 0;
	if (Get_Field(frame, "T=", g_field, sizeof(g_field)))
	{
		Parse_Fixed_1(g_field, &g_temp_int, &g_temp_deci);
	}
	if (Get_Field(frame, "H=", g_field, sizeof(g_field)))
	{
		Parse_Fixed_1(g_field, &g_humi_int, &g_humi_deci);
	}
	if (Get_Field(frame, "SMOKE=", g_field, sizeof(g_field)))
	{
		g_smoke = (uint16_t)atoi(g_field);
	}
	if (Get_Field(frame, "LIGHT=", g_field, sizeof(g_field)))
	{
		g_light = (uint16_t)atoi(g_field);
	}
	if (Get_Field(frame, "PIR=", g_field, sizeof(g_field)))
	{
		g_pir = (uint8_t)atoi(g_field);
	}
	if (Get_Field(frame, "RELAY=", g_field, sizeof(g_field)))
	{
		g_relay_on = (uint8_t)atoi(g_field);
	}
	if (Get_Field(frame, "MODE=", g_field, sizeof(g_field)))
	{
		g_auto_mode = (strstr(g_field, "AUTO") != 0) ? 1 : 0;
	}
	Update_Alarm();
	Draw_Gateway_Display();
	Send_Gateway_Ping();
	if (Get_Field(frame, "ACK=", g_field, sizeof(g_field)))
	{
		sprintf(g_wifi_tx, "ACK,EXECUTED,%s,RELAY=%u,MODE=%s",
			g_field, g_relay_on, g_auto_mode ? "AUTO" : "MANUAL");
		Send_Phone_Text(g_wifi_tx);
	}
	Send_Phone_Status();
}

static uint8_t Start_City_Wifi(void)
{
	char ip[24];

	macESP8266_CH_ENABLE();
	if (!ESP8266_AT_Test())
	{
		return 0;
	}
	/* Disable command echo so incoming TCP payloads are easier to recognize. */
	if (!ESP8266_Cmd("ATE0", "OK", 0, 500))
	{
		return 0;
	}
	if (!ESP8266_Net_Mode_Choose(AP))
	{
		return 0;
	}
	if (!ESP8266_CIPAP(CITY_WIFI_IP))
	{
		return 0;
	}
	if (!ESP8266_BuildAP(CITY_WIFI_SSID, CITY_WIFI_PASSWORD, WPA2_PSK))
	{
		return 0;
	}
	/* ESP-AT requires multiple-connection mode for a TCP server.  The
	 * firmware still serves one phone in normal use and remembers its ID. */
	if (!ESP8266_Enable_MultipleId(ENABLE))
	{
		return 0;
	}
	if (!ESP8266_StartOrShutServer(ENABLE, CITY_WIFI_PORT, CITY_WIFI_TIMEOUT))
	{
		return 0;
	}
	if (ESP8266_Inquire_ApIp(ip, sizeof(ip)))
	{
		printf("CITY WIFI SSID=%s IP=%s PORT=%s\r\n", CITY_WIFI_SSID,
			ip, CITY_WIFI_PORT);
	}
	Clear_Esp8266_Rx();
	/* Discard telemetry that may have accumulated while the ESP AT setup
	 * was blocking, then start normal LoRa frame polling with a clean buffer. */
	CSTX_Lora_Receive_Buf_Clear();
	return 1;
}

int main(void)
{
	uint8_t wifi_ok;

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	SysTick_Init(72);
	usart1_init(115200);
	usart2_init(9600);
	CS_OLED_Init();
	OLED_Clear();
	CSTX_Lora_M0_M1_Init();
	Set_Lora_Transmit_Mode();
	CSTX_Lora_Receive_Buf_Clear();
	delay_ms(100);
	Beep_Init();
	Draw_Gateway_Display();

	printf("CITY GATEWAY B START\r\n");
	printf("ESP12F: EN=PB8 RST=PB1 TX=PB10 RX=PB11\r\n");
	ESP8266_Init();
	wifi_ok = Start_City_Wifi();
	if (wifi_ok)
	{
		g_wifi_ready = 0;
		OLED_ShowString(0, 0, "WIFI AP READY   ");
	}
	else
	{
		OLED_ShowString(0, 0, "WIFI INIT FAIL  ");
		printf("CITY WIFI INIT FAILED\r\n");
	}

	while (1)
	{
		if (Take_Lora_Frame(g_lora_frame, sizeof(g_lora_frame)))
		{
			Handle_Lora_Frame(g_lora_frame);
		}
		if (Take_Esp8266_Frame(g_wifi_frame, sizeof(g_wifi_frame)))
		{
			Handle_Wifi_Frame(g_wifi_frame);
		}
		if (g_lora_silence_ms >= LORA_TIMEOUT_MS)
		{
			g_lora_online = 0;
			Update_Alarm();
			Draw_Gateway_Display();
		}

		delay_ms(20);
		g_lora_silence_ms += 20;
	}
}
