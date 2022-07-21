/**
	*@file		bspCSIS.h
	*@data		2022.03.28
	*@author	CSIS��Car Soared Into Sky��
	*@brief		Onboard support for packet header files
	*/

#ifndef __BSPCSIS_H__
#define __BSPCSIS_H__

/*------------------ Includeͷ�ļ����� ----------------------- */
#include "main.h"
#include "adc.h"
#include "i2c.h"
#include "tim.h"
#include "gpio.h"
#include "usart.h"
#include "stdio.h"
#include "OLED.h"
#include "math.h"

/*-------------------- define�궨���� ----------------------- */
#define ADCVALUEMAX			3800	//ADC�ɼ����ֵ
#define ADCVALUEMIN			0			//ADC�ɼ���Сֵ
#define BSP_OK					1			//�����쳣�ź�ֵ
#define	BSP_ERROR				0			//�����쳣�ź�ֵ
#define STEPWMMAX				1600	//���pwm�޷����ֵ
#define STEPWMMIN				300  		//���pwm�޷���Сֵ




/*------------------- ��ر����������� ---------------------- */
//Usart���
extern volatile uint8_t rx_len;
extern volatile uint8_t recv_end_flag; 
extern 					uint8_t rx_buffer[200];

/*---------------------- bsp�������� ------------------------ */
void bsp_GloableInit(void);
void bsp_Emac_Operate(void);
void bsp_PCA_Init(void);
void bsp_PID_Init(void);
void bsp_OLED_Display(void);
void bsp_Usart_Receive(void);
void bsp_LED_FeedBack(void);
_Bool bsp_SteMot_MakeZero(void);
float bsp_CycleIn(float value);
float bsp_Cross(float	value);
float bsp_Yshape(float value);
void bsp_SignJudge(void);
void bsp_Usart_CallBack(void);

#endif
