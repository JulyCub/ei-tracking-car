/**
	*@file		bspCSIS.c
	*@data		2022.04.30
	*@version	4.0
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
	*			-	Switch					���ܿ���
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
	*			-	bsp_ADCValue_PoorCmpAnd()				��ȺͲ��Ȩ�㷨
	*			-	bsp_ADC_Operate()								ADC���ݴ�����
	*			-	bsp_Motor_PwmSet()							���PWM���ú���(����)[δʹ��]
	*			-	bsp_SteMot_PwmSet()							���PWM���ú���(ת��)
	*			-	bsp_PID_Init()									PID��ʼ��
	*			-	bsp_PID_Core()									PID�����㷨(λ��ʽ)
	*			-	bsp_PID_Control()								PID���ƺ���
	*			-	bsp_CycleIn()										�����ж�
	*			-	fputc()													��������ض���
	*			-	bsp_Esp8266_ValueCallBack()			WiFi���ݷ��ͺ���
	*			-	bsp_Usart_Operate()							�������ݴ�����
	*			-	bsp_Usart_ADCValueCallBack()		ADC����ֵ���ڻط�����
	*			-	bsp_Usart_Receive()							���ڽ������ݺ���
	*			-	bsp_SteMot_MakeZero()						�����ʼ�����㣨���ֶ��ר�ã�[δʹ��]
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
	_Bool				IN1;
	_Bool 			IN2;
	float				pwmrate;
	
}MotorDriver;

/**
	*@name		MotDiff
	*@type		struct
	*@about 	������ٿ���
	*@param		
	*		--
	*		-	Param				����ϵ��
	*		-	pwmSwitch		������ٸ���ת����ƿ���
	*/
struct{
	
	_Bool			pwmSwitch;
	uint16_t	basepwmvalue;
	float 		Param;
	
}MotDiff;

/**
	*@name		SteMotDriver
	*@type		struct
	*@about 	�������
	*@param
	*		-	pwmrateTemp		pwm���ƹ���ֵ
	*		- pwmrateFnal		pwm��������ֵ
	*		-	Min						�����ֵ
	*		- angle					���Ŀǰ�Ƕ�
	*/
typedef struct{
	
	uint8_t			angle;
	float				pwmrateTemp;	
	float   	  pwmrateFnal;
	float				Min;

	
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
	__IO	float				IDUC_Ex;
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
	
	uint8_t	flag;			//�����㷨ת��
	float paramA;
	float paramB;
	float paramC;
	float paramP;
	float paramL;
	
}PoorCmpAnd;


/**
	*@name		Switch
	*@type		struct
	*@about 	��������
	*@param
	*		- 		
	*		- 		
	*/
struct{
	
	_Bool	ONOF1;			//����
	_Bool	ONOF2;			//ʮ��
	_Bool ONOF3;			//Y��
	_Bool ONOF4;			//��־ֵ����
	uint8_t ONOF5;		//�˲�
	
}Switch = {1, 0, 1, 0, 1};

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
	*		-	Integra_Max				�����޷�ֵ
	*/
typedef struct{
	
	int 	Proportion;
	int 	Integral;
	int 	Differential;
	float Err;
	float	ErrLastValue[3];
	float	ErrLastSum;
	float	Integral_Max;
	float k;
	float b;
	
}TyPID;

/**
	*@breif	ȫ�ֱ�־λ
	*/
struct Flag{
	uint16_t 	A;		//����
	uint16_t 	B;		//����
	uint16_t	C;		//Y��
	uint16_t	D;		//����sign
	uint16_t	G;		//�ɻɹ�
	uint16_t	S;		//���������
	uint16_t	T;		//����
	uint16_t	K;		//Y������
	uint16_t	W;		//�������
}Flag = {0, 0, 0, 0, 0, 0, 0, 0};

/**
	*@breif	�ж϶���
	*/
struct	ITReadTimes{
	int	Tim1;		//����
	int	Tim2;		//���
	int Tim3;		//Y��
	int	Tim4;
	int Tim5;
	int	Tim6;
	int	Tim7;
	int Tim8;
}ITRT = {270, 45, 100, 35, 30, 30, 30, 30};

/**
	*@breif	���������ж�ֵ
	*/
struct JudgeValue{
	uint16_t	jm1;		//����
	uint16_t	jm2;		//����
	uint16_t	jm3;		//ʮ��
	uint16_t	jm4;		//ʮ��
	uint16_t	jm5;		//ʮ��
	uint16_t	jm6;		//Y��
	uint16_t	jm7;		//Y��
	uint16_t	jm8;		//����
}JDVL = {3800, 500, 2000, 100, 100, 10, 100, 2000};

/*------------------------ ��ر������� --------------------------- */

Kalman iducKalman;			//�������˲�
PoorCmpAnd PCA;					//��ȺͲ��Ȩ�㷨ϵ��
MotorDriver Motor[2];		//���
SteMotDriver SteMot;		//���
ADCData adcData;				//ADC����
TyPID PID;							//���PID
TyPID MotorPID;					//���PID�����ã�

uint8_t	huandaoK = 20;	//������ֵ�������ѹ��ֵ



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
	HAL_ADC_Start(&hadc2);
	
	
	//���PWM����
	HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_1);
	
	
	//���PWM����
	HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_2);
	
	
	//��ʱ��3����
	HAL_TIM_Base_Start_IT(&htim3); 
	HAL_TIM_Base_Start_IT(&htim4);
	HAL_TIM_Base_Start(&htim3);
 
	
	//PCA��ʼ��
	bsp_PCA_Init();
	
	//PID��ʼ��
	bsp_PID_Init();
	
	//�������
	bsp_Emac_Operate();
	
	//�����ֵ��ʼ��
	SteMot.Min = 950;
	
	//������س�ʼ��
	Motor[0].OnOff = 1;
	
	//������ٿ��ƿ��س�ʼ��
	MotDiff.pwmSwitch = 0;
	
	//������ٸ����������ϵ��
	MotDiff.Param = 100;
	
	//����ٶ�pwm���ƻ�ֵ��ʼ��
	MotDiff.basepwmvalue = 3000;
	Motor[0].pwmrate = MotDiff.basepwmvalue;
	Motor[1].pwmrate = MotDiff.basepwmvalue;
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
void bsp_PCA_Init(void)
{
	PCA.paramA = 1.90;
	PCA.paramB = 6.75;
	PCA.paramC = 9.65;
	PCA.paramP = 1.18;
	PCA.paramL = 1;
}


/**
	*@funcname		bsp_PID_Init()
	*@brief 			PID��ʼ��
	*@param
	*			--
	*				- P			1
	*				- I			1
	*				- D			1
	*/
void bsp_PID_Init(void)
{
	PID.Proportion = 30;
	PID.Integral = 0;
	PID.Differential = 80;
	PID.Integral_Max = 0;
	PID.b = 55;
	PID.k = 178;
	
	PID.ErrLastSum = 0;
	PID.ErrLastValue[0] = 0.0;
	PID.ErrLastValue[1] = 0.0;
	PID.ErrLastValue[2] = 0.0;
}


/**
	*@funcname		bsp_OutAndInbound()
	*@brief 			���������
	*/
void bsp_OutAndInbound(void)
{
	/* ��� */
	if (Flag.G == 2)
	{
		Flag.S = 1;
		MotDiff.basepwmvalue = 2000;
	}
	if (Flag.S == 1 && ITRT.Tim2 <= 0)
		Motor->OnOff = 0;

}

/**
	*@funcname		bsp_StartAndStop_Detection()
	*@brief 			�ɻɹ���ʼ��ֹͣ���
	*/
void bsp_StartAndStop_Detection(void)
{
	/*
	if(HAL_GPIO_ReadPin(GPIOB, GanHuang_Pin) == GPIO_PIN_SET)
	{
		Flag.G++;
	}
	bsp_OutAndInbound();
	*/
}




/**
	*@funcname		bsp_LED_FeedBack()
	*@brief 			LED���������˸��������
	*/
void bsp_LED_FeedBack(void)
{
	HAL_GPIO_TogglePin(GPIOC, LED1_Pin);
	HAL_Delay(100);
	HAL_GPIO_TogglePin(GPIOC, LED1_Pin);
}



/**
	*@funcname		bsp_ArBi_Filter()
	*@brief 			һ��(����)�˲�
	*/
float bsp_ArBi_Filter(uint16_t value, uint8_t i)
{
	static uint16_t ArBi_lastValue[5] = {0, 0, 0, 0, 0};
	float result;
	
	result = 0.80 * value + (1 - 0.80) * ArBi_lastValue[i];
	ArBi_lastValue[i] = result;
	
	return result;
}


/**
	*@funcname		bsp_ADCValue_PoorCmpAnd()
	*@brief 			��ȺͲ��Ȩ�㷨
	*/
float bsp_ADCValue_PoorCmpAnd(ADCData value)
{
	float Err;
	
	/* ��ȺͲ��Ȩ */
	if(PCA.flag == 0)
	{
		Err = (
						(	PCA.paramA * (value.IDUC_L - value.IDUC_R) +
							PCA.paramB * (value.IDUC_LM - value.IDUC_RM)) /
						(
							(	PCA.paramA * (value.IDUC_L + value.IDUC_R)) +
							(	PCA.paramC * (fabs((double)(value.IDUC_LM - value.IDUC_RM))))
						)
					) * PCA.paramP;
						
	}
	
	return Err;
}



/**
	*@funcname		bsp_ADC_Operate()
	*@brief 			ADC���ݴ�����
	*@logic
	*		--
	*		-	ͨ���˲� -> ��Ⱥͼ�Ȩ�㷨 -> ���Err
	*/
void bsp_ADC_Operate(void)
{
	uint8_t time;

		//һ��(����)�˲�
		for(time = 0; time < 5; time++)
		{
			adcData.filterData[time] = bsp_ArBi_Filter(adcData.orignalData[time], time);
		}
		
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
	*@brief 			���PWM���ƺ���(����)
	*/
void bsp_Motor_PwmSet(MotorDriver *MD)
{
	char K = 0;
	
	if(MotDiff.pwmSwitch==1)
	{
		K = adcData.Error > 0 ? 1 : -1;
		MD[0].pwmrate += K * (MotDiff.Param * (SteMot.pwmrateFnal - SteMot.Min) /
																					(STEPWMMAX - SteMot.Min));
		MD[1].pwmrate -= K * (MotDiff.Param * (SteMot.pwmrateFnal - SteMot.Min) /
																					(STEPWMMAX - SteMot.Min));
	}
	
	Motor[0].pwmrate = MotDiff.basepwmvalue;
	Motor[1].pwmrate = MotDiff.basepwmvalue;
	__HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_1, (uint16_t)MD[0].pwmrate);
	__HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_2, (uint16_t)MD[1].pwmrate);
}




/**
	*@funcname		bsp_SteMot_PwmSet()
	*@brief 			���PWM���ú���(ת��)
	*/
void bsp_SteMot_PwmSet(float value)
{
	float pwmrate = value;
	
	/* ��ֵ���� */
	pwmrate += SteMot.Min;
	
	/* �����ж� */
	pwmrate = bsp_CycleIn(pwmrate);
	
	/* ʮ���ж� */
	pwmrate = bsp_Cross(pwmrate);
	
	/* Y���ж� */
	pwmrate = bsp_Yshape(pwmrate);
	
	/* �����ж� */
	if (Flag.T == 0)
	{
		pwmrate += 220;
	}
	/* ��� */
		if (Flag.S == 1 && ITRT.Tim2 != 0)
	{
		pwmrate += 230;
	}
	
	/* ��־ֵ���� */
	bsp_SignJudge();
	
	if (Flag.S == 1 && ITRT.Tim2 > 0)
		pwmrate += 150;
	if (Flag.S == 1 && ITRT.Tim2 <= 0)
		Motor->OnOff = 0;
	
	/* �޷� */
	if(pwmrate < STEPWMMIN)
		pwmrate = STEPWMMIN;
	else if (pwmrate > STEPWMMAX)
		pwmrate = STEPWMMAX;
	
	/* ��ֵ���� */
	SteMot.pwmrateFnal=pwmrate;
	
	__HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_1, (uint16_t)SteMot.pwmrateFnal);
}



/**************************************************************************************************
***************************************************************************************************/

/**
	*@funcname		bsp_PID_Core()
	*@brief 			PID�����㷨(λ��ʽ)
	*/
float bsp_PID_Core(float error)
{
	PID.Err = error;
	float PwmRate;		
	
	PID.ErrLastValue[2] = PID.ErrLastValue[1];
	PID.ErrLastValue[1] = PID.ErrLastValue[0];
	PID.ErrLastValue[0] = error;
	
	/* �����޷� */
	if (
			((PID.ErrLastSum + error) < PID.Integral_Max) &&
			((PID.ErrLastSum + error) > -PID.Integral_Max)
		 )
	{
		/* errֵ�ۼ� */
		PID.ErrLastSum += error;			
	}
	else if (PID.ErrLastSum > 0)		
	{
		/* �����޷� */
		PID.ErrLastSum = PID.Integral_Max;
	}
	else if (PID.ErrLastSum < 0)		
	{
		/* �����޷� */
		PID.ErrLastSum = -PID.Integral_Max;
	}
	
	PID.Proportion = PID.k * fabs(adcData.Error)+ PID.b;
	/* Core */
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
	SteMot.pwmrateTemp = bsp_PID_Core(adcData.Error);
	bsp_SteMot_PwmSet(SteMot.pwmrateTemp);

}


/**
	*@funcname		bsp_CycleIn()
	*@brief 			�����ж�
	*/
float bsp_CycleIn(float value)
{
	float result = value;
	
	if (Switch.ONOF1 == 1)
	{
		if((adcData.IDUC_LM + adcData.IDUC_RM) > 5000 && adcData.IDUC_M > 3000)
		{
					Flag.A = 1;
		}
	}
	
	if (Flag.A == 1)
	{
		result -= adcData.IDUC_Ex/15;
	}
		return result;
}



/**
	*@funcname		bsp_Cross()
	*@brief 			ʮ�֣�����
	*/
float bsp_Cross(float	value)
{	
	float result = value;
	
	if (Switch.ONOF2 == 1)
	{
		if (adcData.IDUC_M > JDVL.jm3)
		{
			if (adcData.IDUC_L - adcData.IDUC_R < JDVL.jm4)
			{
				if (adcData.IDUC_LM - adcData.IDUC_RM < JDVL.jm5)
				{
					Flag.B++;
				}
			}
		}
	}
	return result;
}

/**
	*@funcname		bsp_Yshape()
	*@brief 			Y��
	*/
float bsp_Yshape(float value)
{
	float result = value;
	
	if (Switch.ONOF3 == 1)
	{
		if (adcData.IDUC_M < 10)
		{
			if (adcData.IDUC_Ex < 120 && adcData.IDUC_M < 120 && ITRT.Tim3 == 100)
			{				
					Flag.C++;
					Flag.K = 1;			
			}
		}
	}
	
	if(Flag.C == 1)
	{
		result -= 120;
	}
	if(Flag.C == 3)
	{
		result += 170;
	}
	return result;
}

/**
	*@funcname		bsp_SignJudge()
	*@brief 			��־ֵ����
	*/
void bsp_SignJudge(void)
{
	if (Switch.ONOF4 == 1)
	{
		if (adcData.IDUC_R - adcData.IDUC_L > JDVL.jm8)
			Flag.D++;
	}
}

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
	*@funcname		bsp_Usart_Operate()
	*@brief 			����ָ��
	*@oder
	*		--
	*		-		1			P:value				PID_P = value
	*		-		2			I:value				PID_I = value
	*		-		3			D:value				PID_D = value
	*		-		4			A:value				PCA_A = value
	*		-		5			B:value				PCA_B = value
	*		-		6			C:value				PCA_C = value
	*		-		7			p:value				PCA_P = value
	*		-		24		L:value				PCA_L = value
	*		-		8			M:value				SteMot.Min = value
	*		-		9			--	
	*							-	G:1					�������
	*							-	G:0					����ر�
	*		-		10		--
	*							-	S:1					������ٸ����������
	*							-	S:0					������ٸ�����ƹر�
	*		-		11		M:value				���ռ�ձȣ��ٶȣ�	
	*		-		15		Pb:value			PID_Pb = value
	*		-		16		Pk:value			PID_Pk = value
	*		-		19		s1:value			���ϻ����ж�
	*		-		20		s2:value			����ʮ���ж�
	*		-		21		s3:value			����Y���ж�
	*		-		22		s4:value			���ϱ�ʶֵ����
	*		-		23		s5:value			�л��˲��㷨
	*		-		17		W:value				�л���һ���㷨
	*		-		18		h:value				������ֵ�������ֵ������
	*		-		25		U:Any					���ڷ��ز���ֵ
	*		--
	*/
void bsp_Usart_Operate(uint8_t *str)
{
	float value;
	uint8_t oder = 0;
	
	if			((*str == 'P') && (sscanf((const char *)str, "P:%f", &value) == 1))		oder = 1;
	else if ((*str == 'I') && (sscanf((const char *)str, "I:%f", &value) == 1))		oder = 2;
	else if ((*str == 'D') && (sscanf((const char *)str, "D:%f", &value) == 1))		oder = 3;
	else if ((*str == 'A') && (sscanf((const char *)str, "A:%f", &value) == 1))		oder = 4;
	else if ((*str == 'B') && (sscanf((const char *)str, "B:%f", &value) == 1))		oder = 5;
	else if ((*str == 'C') && (sscanf((const char *)str, "C:%f", &value) == 1))		oder = 6;
	else if ((*str == 'L') && (sscanf((const char *)str, "L:%f", &value) == 1))		oder = 24;
	else if ((*str == 'p') && (sscanf((const char *)str, "p:%f", &value) == 1))		oder = 7;
	else if ((*str == 'F') && (sscanf((const char *)str, "F:%f", &value) == 1))		oder = 8;
	else if ((*str == 'G') && (sscanf((const char *)str, "G:%f", &value) == 1))		oder = 9;
	else if ((*str == 'S') && (sscanf((const char *)str, "S:%f", &value) == 1))		oder = 10;
	else if ((*str == 'M') && (sscanf((const char *)str, "M:%f", &value) == 1))		oder = 11;
	
	else if ((*str == 'P') && (*(str+1) == 'k') && (sscanf((const char *)str, "Pk:%f", &value) == 1))		oder = 15;
	else if ((*str == 'P') && (*(str+1) == 'b') && (sscanf((const char *)str, "Pb:%f", &value) == 1))		oder = 16;
	else if ((*str == 's') && (*(str+1) == '1') && (sscanf((const char *)str, "s1:%f", &value) == 1))		oder = 19;
	else if ((*str == 's') && (*(str+1) == '2') && (sscanf((const char *)str, "s2:%f", &value) == 1))		oder = 20;
	else if ((*str == 's') && (*(str+1) == '3') && (sscanf((const char *)str, "s3:%f", &value) == 1))		oder = 21;
	else if ((*str == 's') && (*(str+1) == '4') && (sscanf((const char *)str, "s4:%f", &value) == 1))		oder = 22;
	else if ((*str == 's') && (*(str+1) == '5') && (sscanf((const char *)str, "s5:%f", &value) == 1))		oder = 23;
		
	else if ((*str == 'W') && (sscanf((const char *)str, "W:%f", &value) == 1))		oder = 17;
	else if ((*str == 'h') && (sscanf((const char *)str, "h:%f", &value) == 1))		oder = 18;
	else if ((*str == 'U') && (sscanf((const char *)str, "U:%f", &value) == 1))		oder = 25;
	
	else if ((*str == 'J') && (*(str+1) == '1') && (sscanf((const char *)str, "J1:%f", &value) == 1))		oder = 26;
	else if ((*str == 'J') && (*(str+1) == '2') && (sscanf((const char *)str, "J2:%f", &value) == 1))		oder = 27;
	else if ((*str == 'J') && (*(str+1) == '3') && (sscanf((const char *)str, "J3:%f", &value) == 1))		oder = 28;
	else if ((*str == 'J') && (*(str+1) == '4') && (sscanf((const char *)str, "J4:%f", &value) == 1))		oder = 29;
	else if ((*str == 'J') && (*(str+1) == '5') && (sscanf((const char *)str, "J5:%f", &value) == 1))		oder = 30;
	else if ((*str == 'J') && (*(str+1) == '6') && (sscanf((const char *)str, "J6:%f", &value) == 1))		oder = 31;
	else if ((*str == 'J') && (*(str+1) == '7') && (sscanf((const char *)str, "J7:%f", &value) == 1))		oder = 32;
	else if ((*str == 'J') && (*(str+1) == '8') && (sscanf((const char *)str, "J8:%f", &value) == 1))		oder = 33;
	
	else		printf("\nvalue set fail!");
	
	switch(oder)
	{
		case 1: 
						PID.Proportion = (int)value;	
						printf("\nSuccessful set the value of P: %d", PID.Proportion);
						break;
		case 2: 
						PID.Integral = (int)value;	
						printf("\nSuccessful set the value of I: %d", PID.Integral); 
						break;
		case 3:
						PID.Differential = (int)value;	
						printf("\nSuccessful set the value of D: %d", PID.Differential); 
						break;
		case 4:
						PCA.paramA = value;
						printf("\nSuccessful set the value of PCA-A: %f", PCA.paramA); 
						break;
		case 5:
						PCA.paramB = value;
						printf("\nSuccessful set the value of PCA-B: %f", PCA.paramB); 
						break;
		case 6:
						PCA.paramC = value;
						printf("\nSuccessful set the value of PCA-C: %f", PCA.paramC); 
						break;
		case 7:
						PCA.paramP = value;
						printf("\nSuccessful set the value of PCA-P: %f", PCA.paramP); 
						break;
		case 24:
						PCA.paramL = value;
						printf("\nSuccessful set the value of PCA-L: %f", PCA.paramL); 
						break;
		case 8:
						MotDiff.Param = value;
						printf("\nSuccessful set the value of SteMin: %f", MotDiff.Param); 
						break;
		case 9:
						if(value == 1)
						{
							Motor[0].OnOff = 1;
							printf("\nCar Motor ON."); 
						}
						else if (value == 0)
						{
							Motor[0].OnOff = 0;
							printf("\nCar Motor OFF.");
						}
						else
							printf("Err!!");
						break;
		case 10:
						if(value == 1)
						{
								MotDiff.pwmSwitch = 1;
								printf("\nCar Motor Diff Foller ON."); 
						}
						else if (value == 0)
						{
								MotDiff.pwmSwitch = 0;
								printf("\nCar Motor Diff Follor OFF.");
						}
						else
							printf("Err!!");
						break;

		case 11:
						MotDiff.basepwmvalue = (uint16_t)value;
						printf("\nMotor Speed: %.2f%%", (MotDiff.basepwmvalue/100.0)); 
						break;
		case 15:
						PID.k = value;
						printf("\nPID Pk: %.2f", PID.k); 
						break;
		case 16:
						PID.b = value;
						printf("\nPID Pb: %.2f", PID.b); 
						break;
		case 17:
						PCA.flag = value;
						printf("\nSuccessful set the value of pac flag: %f", (float)PCA.flag); 
						break;
		case 18:
						huandaoK = (uint8_t)value;
						printf("\nSuccess! %d", huandaoK);
						break;
		case 19:
						Switch.ONOF1 = (_Bool)value;
						printf("\nSuccess! ONOF1: %d", Switch.ONOF1);
						break;
		case 20:
						Switch.ONOF2 = (_Bool)value;
						printf("\nSuccess! ONOF2: %d", Switch.ONOF2);
						break;
		case 21:
						Switch.ONOF3 = (_Bool)value;
						printf("\nSuccess! ONOF3: %d", Switch.ONOF3);
						break;
		case 22:
						Switch.ONOF4 = (_Bool)value;
						printf("\nSuccess! ONOF4: %d", Switch.ONOF4);
						break;
		case 23:
						Switch.ONOF5 = (uint8_t)value;
						printf("\nSuccess! ONOF5: %d", Switch.ONOF5);
						break;
		case 25:
						bsp_Usart_CallBack();
						break;
		
		case 26:
						JDVL.jm1 = (uint16_t)value;
						printf("\nJ1: %d", JDVL.jm1);
						break;
		case 27:
						JDVL.jm2 = (uint16_t)value;
						printf("\nJ2: %d", JDVL.jm2);
						break;
		case 28:
						JDVL.jm3 = (uint16_t)value;
						printf("\nJ3: %d", JDVL.jm3);
						break;
		case 29:
						JDVL.jm4 = (uint16_t)value;
						printf("\nJ4: %d", JDVL.jm4);
						break;
		case 30:
						JDVL.jm5 = (uint16_t)value;
						printf("\nJ5: %d", JDVL.jm5);
						break;
		case 31:
						JDVL.jm6 = (uint16_t)value;
						printf("\nJ6: %d", JDVL.jm6);
						break;
		case 32:
						JDVL.jm7 = (uint16_t)value;
						printf("\nJ7: %d", JDVL.jm7);
						break;
		case 33:
						JDVL.jm8 = (uint16_t)value;
						printf("\nJ8: %d", JDVL.jm8);
						break;
		default:	break;
	}

}

/**
	*@funcname		bsp_Usart_Receive()
	*@brief 			���ڽ������ݺ���
	*/
void bsp_Usart_Receive(void)
{
	if(recv_end_flag ==1)			
		{	
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
	*@funcname		bsp_Emac_Operate()
	*@brief 			���IN����
	*/
void bsp_Emac_Operate(void)
{
	/* �ɻɹܼ�� */
	bsp_StartAndStop_Detection();

	//�������
	if(Motor[0].OnOff == 1)
	{
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET);

		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, GPIO_PIN_SET);
	}
	//���ֹͣ
	else
	{
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET);

		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, GPIO_PIN_RESET);
	}
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
	bsp_SteMot_PwmSet(SteMot.pwmrateTemp);
	
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
	OLED_ShowNum(100, 4, adcData.IDUC_Ex, 4, 12);
	
	OLED_ShowString(0, 5, (uint8_t *)"PwmRate:", 12);
	OLED_ShowUnFloat(60, 5, SteMot.pwmrateFnal, 7, 2, 12);
	
	OLED_ShowString(0, 6, (uint8_t *)"A:", 12);
	OLED_ShowNum(30, 6, Flag.A, 4, 12);
	OLED_ShowString(60, 6, (uint8_t *)"B:", 12);
	OLED_ShowNum(100, 6, Flag.B, 4, 12);
	OLED_ShowString(0, 7, (uint8_t *)"C:", 12);
	OLED_ShowNum(30, 7, Flag.C, 4, 12);
	OLED_ShowString(60, 7, (uint8_t *)"D:", 12);
	OLED_ShowNum(100, 7, Flag.D, 4, 12);
	
}


/**
	*@brief 	��ʱ���жϻص�����
	*/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim == &htim3)
	{
		HAL_ADC_Start(&hadc2);
		adcData.IDUC_Ex = HAL_ADC_GetValue(&hadc2);
		bsp_PID_Control();
		bsp_Motor_PwmSet(Motor);
	}
	if(htim == &htim4)
	{
		if (Flag.S == 1)
		{
			ITRT.Tim2--;
		}
		if (Flag.K == 1)
		{
			ITRT.Tim3--;
			if(ITRT.Tim3 <= 0)
			{
				Flag.K = 0;
				ITRT.Tim3 = 100;
			}
		}
		if (Flag.A == 1)
		{
			ITRT.Tim4--;
			if (ITRT.Tim4 <= 0)
			{
				Flag.A = 0;
				ITRT.Tim4 = 35;
			}
		}
		if (Flag.T == 0)
		{
			ITRT.Tim5--;
			if (ITRT.Tim5 <= 0)
			{
				Flag.T = 1;
			}
		}
		if (Flag.C == 1)
		{
			ITRT.Tim6--;
			if (ITRT.Tim6 <= 0)
			{
				Flag.C = 2;
				ITRT.Tim6= 30;
			}
		}		
		if(Flag.C == 3)
		{
			ITRT.Tim7--;
			if(ITRT.Tim7<=0)
			{
				Flag.C = 0;
				ITRT.Tim7 = 30;
			}
		}
		if (Flag.W == 1)
		{
			ITRT.Tim8--;
			if (ITRT.Tim8 <= 0)
			{
				Flag.W = 0;
				ITRT.Tim8 = 30;
			}
		}
	}
}

/**
	*@funcname		bsp_Usart_CallBack()
	*@brief 			��������
	*/
void bsp_Usart_CallBack(void)
{
	printf("\n");
	printf("\nPID - P: %d", PID.Proportion);
	printf("\nPID - I: %d", PID.Integral);
	printf("\nPID - D: %d", PID.Differential);
	printf("\nPID - Pb: %.2f", PID.b);
	printf("\nPID - Pk: %.2f", PID.k);
	printf("\n");
	printf("\nPCA - A: %.2f", PCA.paramA);
	printf("\nPCA - B: %.2f", PCA.paramB);
	printf("\nPCA - C: %.2f", PCA.paramC);
	printf("\nPCA - P: %.2f", PCA.paramP);
	printf("\nPCA - L: %.2f", PCA.paramL);
	printf("\n");
	printf("\nSpeed: %.2f%%", MotDiff.basepwmvalue/100.0);
	if (MotDiff.pwmSwitch == 1)
	{
		printf("\nMotDiff: ON");
		printf("\nMotDiff Param: %.2f", MotDiff.Param);
	}
	else
		printf("\nMotDiff: OFF");
	if (Switch.ONOF1 == 1)
	{
		printf("\nhuandaoK; %d", huandaoK);
		printf("\nJ1: %d", JDVL.jm1);
		printf("\nJ2: %d", JDVL.jm2);
	}
	if (Switch.ONOF2 == 1)
	{
		printf("\nJ3: %d", JDVL.jm3);
		printf("\nJ4: %d", JDVL.jm4);
		printf("\nJ5: %d", JDVL.jm5);
	}
	if (Switch.ONOF3 == 1)
	{
		printf("\nJ6: %d", JDVL.jm6);
		printf("\nJ7: %d", JDVL.jm7);
	}
	if (Switch.ONOF4 == 1)
		printf("\nJ8: %d", JDVL.jm8);
	
	printf("\nFlag G:%d", Flag.G);
	printf("\nITRT Tim2: %d", ITRT.Tim2);
	printf("\nITRT Tim1: %d", ITRT.Tim1);
	printf("\nITRT Tim3: %d", ITRT.Tim3);
	printf("\nErr: %.3f", adcData.Error);
	
	printf("\n");
}





/**
  * @brief  �ⲿ�жϻص�����
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	
	if (GPIO_Pin == GPIO_PIN_1 && Flag.W == 0)
	{
		Flag.G++;
		Flag.W = 1;
		bsp_OutAndInbound();
		while(!HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1));
	}
	else if (GPIO_Pin == Key1_Pin)
	{
		Motor->OnOff = !(Motor->OnOff);
		while(!HAL_GPIO_ReadPin(GPIOA, Key1_Pin));
	}
	
}









