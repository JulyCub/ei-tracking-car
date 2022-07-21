/**
	*@file		bspCSIS.c
	*@data		2022.03.28
	*@author	CSIS��Car Soared Into Sky��
	*@brief		Onboard integration support package
	*@logic		
	*		--
	*			
	*@InvoStru
	*			--
	*			-	MotorDriver			�������
	*			-	SteMotDriver		�������	
	*			-	ADCData					ADC����
	*			-	PoorCmpAnd			��Ⱥͼ�Ȩ�㷨ϵ��
	*			-	Key							��������[δʹ��]
	*			-	Kalmam					�������˲��㷨����
	*			-	TyPID						PID�㷨����
	*			--
	*@InvoFunc
	*			--
	*			-	bsp_GloableInit()								����֧�ְ�����ȫ�ֳ�ʼ������
	*			-	bsp_iducKalman_Init()						iducKalman��ʼ��
	*			-	bsp_PCA_Init()									PCA(��Ⱥͼ�Ȩ�㷨ϵ��)��ʼ��
	*			-	bsp_StartAndStop_Detection()		�ɻɹ���ʼ��ֹͣ���
	*			-	bsp_LED_FeedBack()							LED���������˸��������
	*			-	bsp_Kalman_Filter()							�������˲���һά��
	*			-	bsp_ADCvalue_Unifor()						ADC���ݹ�һ������[δʹ��]
	*			-	bsp_ADCValue_PoorCmpAnd()				��Ⱥͼ�Ȩ�㷨
	*			-	bsp_ADC_Operate()								ADC���ݴ�����
	*			-	bsp_Motor_PwmSet()							���PWM���ú���(����)[δʹ��]
	*			-	bsp_SteMot_PwmSet()							���PWM���ú���(ת��)
	*			-	bsp_PID_Init()									PID��ʼ��
	*			-	bsp_PID_Core()									PID�����㷨(λ��ʽ)
	*			-	bsp_PID_Control()								PID���ƺ���
	*			-	fputc()													��������ض���
	*			-	bsp_Esp8266_ValueCallBack()			WiFi���ݷ��ͺ���
	*			-	bsp_Usart_Operate()							�������ݴ�����
	*			-	bsp_Usart_ADCValueCallBack()		ADC����ֵ���ڻط�����
	*			-	bsp_Usart_Receive()							���ڽ������ݺ���
	*			-	bsp_SteMot_MakeZero()						�����ʼ������
	*			-	bsp_Emac_Operate()							���IN����
	*			-	bsp_OLED_Display()							OLED���Ժ���
	*			-	HAL_TIM_PeriodElapsedCallback()	��ʱ���жϻص�����
	*			-	HAL_GPIO_EXTI_Callback()				�ⲿ�жϻص�����
	*			--
	*/
	

/*--------------------- Includeͷ�ļ����� ----------------------- */
#include "bspCSIS.h"


/*---------------------- ��ؽṹ�嶨�� -------------------------- */
/**
	*@name		MotorDriver
	*@type		struct array
	*@about 	�������
	*@param
	*		- MotorDriver[0]		���1
	*		- MotorDriver[1]		���2
	*		--
	*		-	Onoff			���������ֹͣ��־λ
	*		- pwmrate		����Tim -> CRR��ֵ���ɸı�PWMռ�ձ�
	*		- IN1				L298n IN1�߼���������
	*		- IN2				L298n IN2�߼���������
	*/
typedef struct{
	
	_Bool				OnOff;
	uint16_t		pwmrate;
	_Bool				IN1;
	_Bool 			IN2;
	
}MotorDriver;


/**
	*@name		SteMotDriver
	*@type		struct
	*@about 	�������
	*@param
	*		- pwmrate		����Tim -> CRR��ֵ���ɸı�PWMռ�ձ�
	*		- angle			���Ŀǰ�Ƕ�
	*/
typedef struct{
	
	uint16_t		pwmrate;	
	uint8_t			angle;
	
}SteMotDriver;


/**
	*@name	ADCData
	*@type	struct
	*@about 	ADC����
	*@param
	*		- origanlData[]		ADC�ɼ�����ԭʼ����
	*		- filterData[]		�˲�����������
	*		- IDUC_L					����
	*		-	IDUC_R					�ҵ��
	*		- IDUC_M					�е��
	*		- IDUC_LM					���е��
	*		- IDUC_RM					���е��
	*		-	Error						��ֵ
	*/
typedef struct{
	
	__IO	uint16_t		orignalData[5];
	__IO	float				filterData[5];
	__IO	float				IDUC_L;
	__IO	float				IDUC_R;
	__IO	float				IDUC_M;
	__IO	float				IDUC_LM;
	__IO	float				IDUC_RM;
	__IO	float				Error;
	
}ADCData;


/**
	*@name	PoorCmpAnd
	*@type	struct
	*@about 	��Ⱥͼ�Ȩ�㷨ϵ��
	*@param
	*		--
	*		- paramA			����ϵ��A
	*		- paramB			����ϵ��B
	*		- paramC			����ϵ��C
	*		- paramP			����ϵ��P
 	*/
typedef struct{
	
	float paramA;
	float paramB;
	float paramC;
	float paramP;
	
}PoorCmpAnd;


/**
	*@name		Key
	*@type		struct
	*@about 	����������δʹ�ã�
	*@param
	*		- key1		��������1
	*		- key2		��������2
	*/
struct{
	
	_Bool	key1;
	_Bool	key2;
	
}Key;

/**
	*@name	Kalmam
	*@type	struct
	*@about Kalman Filter 
	*@param
	*		- symStateNow					ϵͳʵʱ״̬						X(k)
	* 	- symStatePostFore		ϵͳ�ϴ�Ԥ��״̬				X(k|k-1)
	* 	- symStatePostBest		ϵͳ�ϴ�����״̬				X(k-1|k-1)
	* 	- covNow							����ϵͳ״̬Э����			P(k|k)
	* 	- covPostFore					�ϴ�Ԥ��״̬Э����			P(k|k-1)
	*		- covPostBest					�ϴ�����״̬Э����			P(k-1|k-1)
	*		- symControl					ϵͳ������							U(k)
	*		- symParmA						ϵͳ����A							A
	*		- symParmB						ϵͳ����B							B
	* 	- errorMes						kʱ�̲���ֵ						Z(k)
	* 	- mesParm							����ϵͳ�Ĳ���					H
	* 	- pcesNoise						��������								W(k)
	* 	- mesNoise						��������								V(k)
	*		- transposeA					A��ת�þ���						A'
	*		- transposeQ					W(k)��ת�þ���					Q	
	*		- transposeR					V(k)��ת�þ���					R
	*		- transposeH					H��ת�þ���						H'
	*		- gain								����������							Kg
	*/
typedef struct{
	__IO	float symStateNow[5];
	__IO	float symStatePostFore[5];
	__IO	float	symStatePostBest[5];
	__IO	float covNow[5];
	__IO	float	covPostFore[5];
	__IO	float covPostBest[5];
				float symControl;
				float symParmA;
				float symParmB;
	__IO	float errorMes;
				float	mesParm;
				float pcesNoise;
				float mesNoise;	
				float transposeA;	
				float	transposeQ;
				float transposeR;
				float transposeH;
	__IO	float gain[5];
}Kalman;


/**
	*@name		TyPID
	*@type		struct
	*@about 	PID����ϵ��
	*@param
	*		--
	*		-	Err								�Ա���
	*		-	ErrLastValue[]		ǰ����Errֵ
	*		-	ErrLastSum				Err�ۼ�ֵ
	*		- Proportion				����
	*		- Integral					����
	*		- Differential			΢��
	*/
typedef struct{
	
	float Err;
	float	ErrLastValue[3];
	float	ErrLastSum;
	int 	Proportion;
	int 	Integral;
	int 	Differential;
	int		Integral_Max;
	
}TyPID;

/*------------------------ ��ر������� --------------------------- */

Kalman iducKalman;			//�������˲�
PoorCmpAnd PCA;					//��Ⱥͼ�Ȩ�㷨ϵ��
MotorDriver Motor[2];		//���
SteMotDriver SteMot;		//���
ADCData adcData;				//ADC����
TyPID PID;							//���PID
TyPID MotorPID;					//���PID�����ã�



/*-------------------------- �������� ----------------------------- */

/**
	*@funcname		bsp_GloableInit()
	*@brief 			����֧�ְ�����ȫ�ֳ�ʼ��
	*/
void bsp_GloableInit(void)
{
	//OLED��ʼ��
	
	OLED_Init();
	OLED_On();
	OLED_Clear();
	
	
	//ADC DMA ģʽ����
	HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adcData.orignalData, 5);
	
	//���PWM����
	HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_1);
	
	
	//���PWM����
	HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_2);
	
	
	//��ʱ��3����
	HAL_TIM_Base_Start_IT(&htim3); 
	HAL_TIM_Base_Start(&htim3);
 
	
	//Kalman��ʼ��
	bsp_iducKalman_Init();
	
	//PCA��ʼ��
	bsp_PCA_Init();
	
	//PID��ʼ��
	bsp_PID_Init();
	
	//�������
	bsp_Emac_Operate();
	
	//�����ʼ������
	bsp_SteMot_MakeZero();
}



/**
	*@funcname		bsp_iducKalman_Init()
	*@brief 			iducKalman��ʼ��
	*@param
	*			--
	*				- X(k|k)	10.0
	*				- P(k|k)	10.0
	*				- A				1.0			
	*				- A'			1.0
	*				- U(k)		0.0
	*				- H				1.0
	*				- H'			1.0
	*				- Q				0.5
	*				- R				0.5
	*				-	Kg			1.0
	*/
/*************************** ������ *****************************/
void bsp_iducKalman_Init(void)
{
	uint8_t _i;
	
	for(_i = 0; _i < 5; _i++)
		iducKalman.symStateNow[_i] = 10;
	for(_i = 0; _i < 5; _i++)
		iducKalman.covNow[_i] = 10;
	
	iducKalman.symParmA = 1;
	iducKalman.transposeA = 1;
	iducKalman.symControl = 0;
	iducKalman.mesParm = 1;
	iducKalman.transposeH = 1;
	iducKalman.transposeQ = 0.5;
	iducKalman.transposeR = 0.5;
	for(_i = 0; _i < 5; _i++)
		iducKalman.gain[_i] = 1.0;
} 


/**
	*@funcname		bsp_PCA_Init()
	*@brief 			PCA(��Ⱥͼ�Ȩ�㷨ϵ��)��ʼ��
	*@param
	*			--
	*				- A			1
	*				- B			1
	*				- C			1
	*				- P			0.5
	*/
/*************************** ������ *****************************/
void bsp_PCA_Init(void)
{
	PCA.paramA = 1;
	PCA.paramB = 1;
	PCA.paramC = 1;
	PCA.paramP = 0.5;
}



/**
	*@funcname		bsp_StartAndStop_Detection()
	*@brief 			�ɻɹ���ʼ��ֹͣ���
	*/
void bsp_StartAndStop_Detection(void)
{
	if(HAL_GPIO_ReadPin(GPIOB, GanHuang_Pin) == GPIO_PIN_SET)
			Motor->OnOff = 0;
	else
			Motor->OnOff = 1;
}




/**
	*@funcname		bsp_LED_FeedBack()
	*@brief 			LED���������˸��������
	*/
void bsp_LED_FeedBack(void)
{
	HAL_GPIO_TogglePin(GPIOC, LED1_Pin);
	HAL_Delay(200);
	HAL_GPIO_TogglePin(GPIOC, LED1_Pin);
}






/**
	*@funcname		bsp_Kalman_Filter()
	*@brief 			�������˲���һά��
	*/
/*************************** ���˶� *****************************/
float bsp_Kalman_Filter(uint16_t data, uint8_t _i)
{
	iducKalman.errorMes = (float)data;
	
	/*		X(k|k-1)=A X(k-1|k-1)+B U(k)	 */
	iducKalman.symStatePostFore[_i] = iducKalman.symParmA * iducKalman.symStatePostBest[_i] +
																iducKalman.symParmB * iducKalman.symControl;
	
	/* 		P(k|k-1)=A P(k-1|k-1) A��+Q		*/
	iducKalman.covPostFore[_i] = 	iducKalman.symParmA * iducKalman.covPostBest[_i] * 
														iducKalman.transposeA +iducKalman.transposeQ;
	
	/* 		X(k|k)= X(k|k-1)+Kg(k) (Z(k)-H X(k|k-1))		*/
	iducKalman.symStateNow[_i] = 	iducKalman.symStatePostFore[_i] + iducKalman.gain[_i] * 
														(iducKalman.errorMes - iducKalman.mesParm * iducKalman.symStatePostFore[_i]);
	
	/* 		Kg(k)= P(k|k-1) H�� / (H P(k|k-1) H�� + R)		*/
	iducKalman.gain[_i] = (iducKalman.covPostFore[_i] * iducKalman.transposeH) / 
										(
											iducKalman.mesParm * iducKalman.covPostFore[_i] *
											iducKalman.transposeH + iducKalman.transposeR );
	
	/*		P(k|k)=��I-Kg(k) H��P(k|k-1)		*/
	iducKalman.covNow[_i] = (1 - iducKalman.gain[_i] * iducKalman.mesParm) * iducKalman.covPostFore[_i];
	
	/*		X(k-1|k-1) = X(k|k) 	*/
	iducKalman.symStatePostBest[_i] = iducKalman.symStateNow[_i];
	
	/*		P(k-1|k-1) = P(k|k)		*/
	iducKalman.covPostBest[_i] = iducKalman.covNow[_i];
	
	return iducKalman.symStateNow[_i];
}







/**
	*@funcname		bsp_ADCvalue_Unifor()
	*@brief 			ADC���ݹ�һ������(δʹ��)
	*/
float bsp_ADCvalue_Unifor(float value)
{
	/* �޷� */
	if 			(value > ADCVALUEMAX) value = ADCVALUEMAX;
	else if (value < ADCVALUEMIN)	value = ADCVALUEMIN;
	/* ��һ�� */
	value = 100 * ((value - ADCVALUEMIN)/(ADCVALUEMAX - ADCVALUEMIN));
	return value;
}

/**
	*@funcname		bsp_ADCValue_PoorCmpAnd()
	*@brief 			��Ⱥͼ�Ȩ�㷨
	*/
float bsp_ADCValue_PoorCmpAnd(ADCData value)
{
	float Err;
	
	Err = (
					(	PCA.paramA * (adcData.IDUC_L - adcData.IDUC_R) +
						PCA.paramB * (adcData.IDUC_LM - adcData.IDUC_RM)) /
					(
						(	PCA.paramA * (adcData.IDUC_L + adcData.IDUC_R)) +
						(	PCA.paramC * (fabs((double)(adcData.IDUC_LM - adcData.IDUC_RM))))
					)
				) * PCA.paramP;
						
	return Err;
}



/**
	*@funcname		bsp_ADC_Operate()
	*@brief 			ADC���ݴ�����
	*@logic
	*		--
	*		-	ͨ���˲� -> ��Ⱥͼ�Ȩ�㷨 -> Err
	*/
/*************************** ������ *****************************/
void bsp_ADC_Operate(void)
{
	uint8_t time;
	
	//ͨ���˲�
	for(time = 0; time < 5; time++)
	{
		adcData.filterData[time] = bsp_Kalman_Filter(adcData.orignalData[time], time);
	}
	
	//��ֵ
	adcData.IDUC_L 	= adcData.filterData[0];
	adcData.IDUC_LM = adcData.filterData[1];
	adcData.IDUC_M 	= adcData.filterData[2];
	adcData.IDUC_RM = adcData.filterData[3];
	adcData.IDUC_R 	= adcData.filterData[4];
	
	//����Err
	adcData.Error = bsp_ADCValue_PoorCmpAnd(adcData);
	
}


/**
	*@funcname		bsp_Motor_PwmSet()
	*@brief 			���PWM���ú���(����)
	*/
void bsp_Motor_PwmSet(MotorDriver *MD)
{
	__HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_1, (uint16_t)MD[0].pwmrate);
	__HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_2, (uint16_t)MD[1].pwmrate);
}




/**
	*@funcname		bsp_SteMot_PwmSet()
	*@brief 			���PWM���ú���(ת��)
	*/
void bsp_SteMot_PwmSet(void)
{
	__HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_1, (uint16_t)SteMot.pwmrate);
}



/**************************************************************************************************
***************************************************************************************************/
/**
	*@funcname		bsp_PID_Init()
	*@brief 			PID��ʼ��
	*@param
	*			--
	*				- P			1
	*				- I			1
	*				- D			1
	*/
/*************************** ������ *****************************/
void bsp_PID_Init(void)
{
	PID.Proportion = 10;
	PID.Integral = 15;
	PID.Differential = 30;
	
	PID.ErrLastSum = 0;
	PID.ErrLastValue[0] = 0.0;
	PID.ErrLastValue[1] = 0.0;
	PID.ErrLastValue[2] = 0.0;
}


/**
	*@funcname		bsp_PID_Core()
	*@brief 			PID�����㷨(λ��ʽ)
	*/
uint16_t bsp_PID_Core(float error)
{
	PID.Err = error;
	float PwmRate;		//PwmRateΪPID�㷨������õ���PWM����ֵ
	
	PID.ErrLastValue[2] = PID.ErrLastValue[1];
	PID.ErrLastValue[1] = PID.ErrLastValue[0];
	PID.ErrLastValue[0] = error;
	
	if ((PID.ErrLastSum + error) < PID.Integral_Max && (PID.ErrLastSum + error) > -PID.Integral_Max)//�޷��жϣ���abs()ȡ����ֵ���ܻᶪʧ���ȵ�������ĳ��޶�����
	{
		PID.ErrLastSum += error;			//�ڷ����ھͼ���
	}
	else if (PID.ErrLastSum > 0)		//�����˵���������
	{
		PID.ErrLastSum = PID.Integral_Max;
	}
	else if (PID.ErrLastSum < 0)		//�����˵����Ǹ���
	{
		PID.ErrLastSum = -PID.Integral_Max;
	}
	
	/***************************** ������ *****************************/
	PwmRate = PID.Proportion * (error) + PID.Integral * PID.ErrLastSum + 
						PID.Differential * ((PID.ErrLastValue[0] - PID.ErrLastValue[1]) -
					(	PID.ErrLastValue[1] - PID.ErrLastValue[2]));
	

	return PwmRate;
}


/**
	*@funcname		bsp_PID_Control()
	*@brief 			PID���ƺ���
	*/
void bsp_PID_Control(void)
{
	bsp_ADC_Operate();
	SteMot.pwmrate = bsp_PID_Core(adcData.Error);
	bsp_SteMot_PwmSet();
	/*
			Ԥ��������
			����Ƕȼ��� 
			SteMot->angle
	*/
}

/**************************************************************************************************
***************************************************************************************************/
/**************************************��־�쳣ӳ�����ѡ��****************************************/

/**
	*@funcname		
	*@brief 			����
	*/




/**
	*@funcname		
	*@brief 			ʮ��
	*/




/**
	*@funcname		
	*@brief 			����
	*/



/**************************************************************************************************
***************************************************************************************************/


/**
	*@funcname		fputc()
	*@brief 			��������ض���
	*/
int fputc(int ch, FILE *f)
{
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xffff);
  return ch;
}



/**
	*@funcname		bsp_Esp8266_ValueCallBack()
	*@brief 			WiFi���ݷ��ͺ���
	*		--
	*		-					����printf("%f",error);�Ϳ��԰�errorֵͨ�����ڴ���ȥ��
	*		-					����esp8266ת����vofa��������ʽѡ��firewater�����Ҫ
	*		-					�Ƚ��롱�趨ֵ����࣬����չʾ�������ݣ���һ���ڴ˴�
	*		-					д��printf���ʽΪprintf(ͨ��1,ͨ��2,ͨ��3,...);��
	*		-					��float����
	*/
void bsp_Esp8266_ValueCallBack(void)
{
	bsp_PID_Control();
	printf("Or:%.0f %.0f %.0f %.0f %f.0\n", (float)adcData.orignalData[0], (float)adcData.orignalData[1], 
																					(float)adcData.orignalData[2], (float)adcData.orignalData[3], 
																					(float)adcData.orignalData[4]);
	printf("Fi:%.0f %.0f %.0f %.0f %f.0\n", (float)adcData.filterData[0], (float)adcData.filterData[1], 
																					(float)adcData.filterData[2], (float)adcData.filterData[3], 
																					(float)adcData.filterData[4]);
}



/**
	*@funcname		bsp_Usart_Operate()
	*@brief 			�������ݴ�����
	*/
void bsp_Usart_Operate(uint8_t *str)
{
	int value;
	uint8_t oder = 0;
	
	if			((*str == 'P') && (sscanf((const char *)str, "P:%d", &value) == 1))		oder = 1;
	else if ((*str == 'I') && (sscanf((const char *)str, "I:%d", &value) == 1))		oder = 2;
	else if ((*str == 'D') && (sscanf((const char *)str, "D:%d", &value) == 1))		oder = 3;
	else		printf("\nPID value set fail!");
	
	switch(oder)
	{
		case 1: 
						PID.Proportion = value;	
						printf("\nSuccessful set the value of P: %d", PID.Proportion);
						break;
		case 2: 
						PID.Integral = value;	
						printf("\nSuccessful set the value of I: %d", PID.Integral); 
						break;
		case 3:
						PID.Differential = value;	
						printf("\nSuccessful set the value of D: %d", PID.Differential); 
						break;
		default:	break;
	}

}







/**
	*@funcname		bsp_Usart_ADCValueCallBack()
	*@brief 			ADC����ֵ���ڻط�����
	*/
void bsp_Usart_ADCValueCallBack(void)
{
	
	uint8_t times;
	/*
	bsp_PID_Control();
	printf("\n\nThe origanl data:");
	for(times = 0; times < 5; times++){
		printf("\nChannel %d: %d", times, adcData.orignalData[times]);
	}
	printf("\n\nThe filter data:");
	for(times = 0; times < 5; times++){
		printf("\nChannel %d: %.3f", times, adcData.filterData[times]);
	}
	printf("\n\nThe Error data:");
	printf("\nChannel %d: %.3f", times, adcData.Error);
	
	printf("\nSteMotor PWM Count: %d", SteMot.pwmrate);
	*/

	/* ������Ÿв���ר�� */
	
	printf("\nCollection value:\n");
	for(times = 0; times < 5; times++){
		printf("%6d   ", adcData.orignalData[times]);
	}
	printf("\n");
	for(times = 0; times < 5;  times++){
		printf("%6d   ", (int)adcData.filterData[times]);
	}
	printf("\n");
	printf("%.3f", adcData.Error);
	printf("\n\n");
	
} 
 






/**
	*@funcname		bsp_Usart_Receive()
	*@brief 			���ڽ������ݺ���
	*/
void bsp_Usart_Receive(void)
{
	if(recv_end_flag ==1)			
		{	
			//bsp_LED_FeedBack();
			bsp_Usart_Operate(rx_buffer);
			for(uint8_t i=0;i<rx_len;i++)
			{
				rx_buffer[i]=0;
			}
			rx_len=0;
			recv_end_flag=0;
		}
		HAL_UART_Receive_DMA(&huart1,rx_buffer,200);	
}


/**
	*@funcname		bsp_SteMot_MakeZero()
	*@brief 			�����ʼ������
	*/
void bsp_SteMot_MakeZero(void)
{
	__HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_1, 1000);
}



/**
	*@funcname		bsp_Emac_Operate()
	*@brief 			���IN����
	*/
/*************************** ������ *****************************/
void bsp_Emac_Operate(void)
{
	//bsp_StartAndStop_Detection();
	//if(Motor->OnOff == 1)
	//{
		//���1��ת
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET);
		//���2��ת
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, GPIO_PIN_SET);
	//}
	
	/*else
	{
		//���1ֹͣ
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET);
		//���2ֹͣ
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, GPIO_PIN_RESET);
	}*/
}







/**
	*@funcname		bsp_OLED_Display()
	*@brief 			OLED���Ժ���
	*@count
	*		--
	*		-					PID��ֵ
	*		-					ADCԭʼ�ɼ�ֵ
	*		-					Err
	*/
void bsp_OLED_Display(void)
{
	bsp_PID_Control();
	
	OLED_ShowString(0, 0, (uint8_t *)"P:", 12);
	OLED_ShowNum(15, 0, PID.Proportion, 3, 12);
	OLED_ShowString(40, 0, (uint8_t *)"I:", 12);
	OLED_ShowNum(55, 0, PID.Integral, 3, 12);
	OLED_ShowString(80, 0, (uint8_t *)"D:", 12);
	OLED_ShowNum(95, 0, PID.Differential, 3, 12);	
	
	OLED_ShowString(0, 2, (uint8_t *)"ADC OrValue:", 12);
	OLED_ShowNum(0, 3, adcData.orignalData[0], 4, 12);
	OLED_ShowNum(50, 3, adcData.orignalData[1], 4, 12);
	OLED_ShowNum(100, 3, adcData.orignalData[2], 4, 12);
	OLED_ShowNum(0, 4, adcData.orignalData[3], 4, 12);
	OLED_ShowNum(50, 4, adcData.orignalData[4], 4, 12);
	
	OLED_ShowString(0, 6, (uint8_t *)"Err:", 12);
	OLED_ShowUnFloat(30, 6, adcData.Error, 6, 3, 12);
	
	OLED_ShowString(0, 7, (uint8_t *)"PwmRate:", 12);
	OLED_ShowUnFloat(60, 7, SteMot.pwmrate, 6, 2, 12);
	
}







/**
	*@brief 	��ʱ���жϻص�����
	*/
/********************* ��ʱ��Ƶ�ʴ�ȷ�� ******************************/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim == &htim3)
	{
		bsp_PID_Control();
		bsp_Usart_ADCValueCallBack();
	}
}








/**
  * @brief  �ⲿ�жϻص�����
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	
	if (GPIO_Pin == Key1_Pin)
	{
		bsp_Usart_ADCValueCallBack();
		while(!HAL_GPIO_ReadPin(GPIOA, Key1_Pin));
	}
	else if (GPIO_Pin == Key2_Pin)
	{
		if(Motor->OnOff==0)
			Motor->OnOff = 1;
		else
			Motor->OnOff = 0;
		while(!HAL_GPIO_ReadPin(GPIOA, Key2_Pin));
	}
	
}

