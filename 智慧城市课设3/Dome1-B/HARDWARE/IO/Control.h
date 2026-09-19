#ifndef __CONTROL_H
#define __CONTROL_H
#include "stm32f10x.h"  

typedef struct{
	float Temp;
	float Temp_Th;
	uint8_t Humi;
	uint8_t Humi_Th;
	uint8_t Light;
	uint8_t Light_Th;
	uint8_t SoilHumi;
	uint8_t SoilHumi_Th;
	uint8_t Hand_Mode;
}Sensor_;

extern Sensor_ Sensor;

void Oled_Show(void);
void ESP_Send_Data(void);
void Get_Sensor(void);
	
#endif