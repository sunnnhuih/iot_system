#include "stm32f10x.h"
#include <stdio.h>
#include <string.h>
#include "delay.h"
#include "bsp_usart.h"
#include "oled.h"
#include "cs_lora.h"
#include "bsp_led.h"
#include "bsp_dht11.h"
#include "bsp_adc.h"
#include "bsp_io.h"

#define DARK_LEVEL_THRESHOLD    1200
#define SENSOR_PERIOD_MS        2000
#define GATEWAY_TIMEOUT_MS      8000

static uint8_t g_auto_mode = 1;
static uint8_t g_relay_on = 0;
static uint16_t g_smoke = 0;
static uint16_t g_light = 0;
static uint8_t g_pir = 0;
static u32 g_gateway_silence_ms = 0;
static char g_tx[160];
static char g_line[32];

static void Relay_Set(uint8_t on)
{
	if (on)
	{
		Relay_ON;
		g_relay_on = 1;
	}
	else
	{
		Relay_OFF;
		g_relay_on = 0;
	}
}

static void Apply_Automatic_Lighting(void)
{
	if (g_auto_mode)
	{
		if ((g_light < DARK_LEVEL_THRESHOLD) && (g_pir != 0))
		{
			Relay_Set(1);
		}
		else
		{
			Relay_Set(0);
		}
	}
}

static void Draw_Node_Display(void)
{
	OLED_ShowString(0, 0, "CITY NODE A    ");
	OLED_ShowString(0, 2, "                ");
	sprintf(g_line, "T:%d.%d H:%d.%d", DHT11_Data.temp_int,
		DHT11_Data.temp_deci, DHT11_Data.humi_int, DHT11_Data.humi_deci);
	OLED_ShowString(0, 2, g_line);
	OLED_ShowString(0, 4, "                ");
	sprintf(g_line, "S:%u L:%u", g_smoke, g_light);
	OLED_ShowString(0, 4, g_line);
	OLED_ShowString(0, 6, "                ");
	sprintf(g_line, "P:%u R:%s %s", g_pir, g_relay_on ? "ON" : "OFF",
		g_auto_mode ? "AUTO" : "MAN");
	OLED_ShowString(0, 6, g_line);
}

static void Send_Telemetry(void)
{
	sprintf(g_tx,
		/* Keep the periodic LoRa payload below the common 58-byte air-frame
		 * limit.  MODE is returned in the shorter ACK frame below; B derives
		 * ALARM from SMOKE. */
		"A,T=%d.%d,H=%d.%d,SMOKE=%u,LIGHT=%u,PIR=%u,RELAY=%u\r\n",
		DHT11_Data.temp_int, DHT11_Data.temp_deci,
		DHT11_Data.humi_int, DHT11_Data.humi_deci,
		g_smoke, g_light, g_pir, g_relay_on);
	CSTX_Lora_Send_String(g_tx);
}

static void Send_Ack(const char *text)
{
	sprintf(g_tx, "A,ACK=%s,RELAY=%u,MODE=%s\r\n", text, g_relay_on,
		g_auto_mode ? "AUTO" : "MANUAL");
	CSTX_Lora_Send_String(g_tx);
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

static void Handle_Lora_Command(const char *frame)
{
	if (strncmp(frame, "B,", 2) == 0)
	{
		/* B sends a ping immediately after each telemetry frame.  This lets
		 * A leave a manually forced relay in a safe local state if LoRa dies. */
		g_gateway_silence_ms = 0;
	}
	if (strstr(frame, "PING") != 0)
	{
		return;
	}
	if (strstr(frame, "MODE=AUTO") != 0)
	{
		g_auto_mode = 1;
		Apply_Automatic_Lighting();
		Send_Ack("MODE_AUTO");
	}
	else if (strstr(frame, "MODE=MANUAL") != 0)
	{
		g_auto_mode = 0;
		Send_Ack("MODE_MANUAL");
	}
	else if (strstr(frame, "LIGHT=ON") != 0)
	{
		g_auto_mode = 0;
		Relay_Set(1);
		Send_Ack("LIGHT_ON");
	}
	else if (strstr(frame, "LIGHT=OFF") != 0)
	{
		g_auto_mode = 0;
		Relay_Set(0);
		Send_Ack("LIGHT_OFF");
	}
	else if (strstr(frame, "STATUS") != 0)
	{
		Send_Telemetry();
	}
}

static void Read_Sensors(void)
{
	g_smoke = ADC_Read[0];
	g_light = ADC_Read[1];
	g_pir = (uint8_t)ReadInfrared();
	DHT11_Read_TempAndHumidity(&DHT11_Data);
	Apply_Automatic_Lighting();
	Draw_Node_Display();
}

int main(void)
{
	char command[128];
	u32 elapsed = 0;

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
	DHT11_Init();
	/* Allow the DHT11 and its pull-up line to settle before the first read. */
	delay_ms(1000);
	MYADC_Init();
	MYGPIO_Init();
	Relay_Set(0);
	Draw_Node_Display();

	printf("CITY NODE A READY\r\n");
	printf("DHT11=PA12 SMOKE=PA1 LIGHT=PA6 PIR=PA0 RELAY=PB2\r\n");

	while (1)
	{
		if (Take_Lora_Frame(command, sizeof(command)))
		{
			Handle_Lora_Command(command);
			Draw_Node_Display();
		}

		if (elapsed >= SENSOR_PERIOD_MS)
		{
			Read_Sensors();
			Send_Telemetry();
			elapsed = 0;
		}

		delay_ms(20);
		elapsed += 20;
		if (g_gateway_silence_ms < GATEWAY_TIMEOUT_MS)
		{
			g_gateway_silence_ms += 20;
		}
		if ((g_gateway_silence_ms >= GATEWAY_TIMEOUT_MS) && !g_auto_mode)
		{
			/* Lost gateway: leave manual mode and release the simulated lamp.
			 * The next sensor cycle resumes the local AUTO rule. */
			g_auto_mode = 1;
			Relay_Set(0);
		}
	}
}
