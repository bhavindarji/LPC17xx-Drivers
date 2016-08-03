/******************************************************************//**
* @file		lpc17xx_timer.c
* @brief	Contains all functions support for Timer firmware library on LPC17xx
* @version	1.0
* @date		27. Nov. 2013
* @author	Dwijay.Edutech Learning Solutions
**********************************************************************/

/* Peripheral group ----------------------------------------------------------- */
/** @addtogroup TIM
 * @{
 */

/* Includes ------------------------------------------------------------------- */
#include "lpc_system_init.h"
#include "lpc17xx_timer.h"

/* If this source file built with example, the LPC17xx FW library configuration
 * file in each example directory ("lpc17xx_libcfg.h") must be included,
 * otherwise the default FW library configuration file must be included instead
 */
/* Private Variables ----------------------------------------------------------- */
uint8_t tim_init=0;


/* Private Functions ---------------------------------------------------------- */

static uint32_t getPClock (uint32_t timernum);
static uint32_t converUSecToVal (uint32_t timernum, uint32_t usec);
static uint32_t converPtrToTimeNum (LPC_TIM_TypeDef *TIMx);


/*********************************************************************//**
 * @brief	TIM0 interrupt handler sub-routine
 * @param	None
 * @return	None
 **********************************************************************/
void TIMER0_IRQHandler(void)
{
	TIM_ClearIntPending(LPC_TIM0, TIM_MR1_INT);  // clear Interrupt
}


/*********************************************************************//**
 * @brief	TIM1 interrupt handler sub-routine
 * @param	None
 * @return	None
 **********************************************************************/
void TIMER1_IRQHandler(void)
{
	if (TIM_GetIntCaptureStatus(LPC_TIM1,0))
	{
		TIM_ClearIntCapturePending(LPC_TIM1,0);
		if(first_capture==TRUE)
		{
			TIM_Cmd(LPC_TIM1,DISABLE);
			TIM_ResetCounter(LPC_TIM1);
			TIM_Cmd(LPC_TIM1,ENABLE);
			count++;
			if(count==20)first_capture=FALSE; //stable
		}
		else
		{
			count=0; //reset count for next use
			done=TRUE;
			capture = TIM_GetCaptureValue(LPC_TIM1,0);
		}
	}
}


/*********************************************************************//**
 * @brief	TIM2 interrupt handler sub-routine
 * @param	None
 * @return	None
 **********************************************************************/
void TIMER2_IRQHandler(void)
{
	TIM_ClearIntPending(LPC_TIM2, TIM_MR0_INT);  // clear Interrupt
}


/*********************************************************************//**
 * @brief	TIM3 interrupt handler sub-routine
 * @param	None
 * @return	None
 **********************************************************************/
void TIMER3_IRQHandler(void)
{
	if (TIM_GetIntStatus(LPC_TIM3, TIM_MR0_INT)== SET)
	{
		TIM_Cmd(LPC_TIM3,DISABLE);                 // Disable Timer
		TIM_ResetCounter(LPC_TIM3);
		if(toggle_tim3 == TRUE)
		{
			TIM_UpdateMatchValue(LPC_TIM3,0,T1*10);//MAT3.0
			toggle_tim3=FALSE;
		}
		else
		{
			TIM_UpdateMatchValue(LPC_TIM3,0,T2*10);
			toggle_tim3=TRUE;
		}
		TIM_Cmd(LPC_TIM3,ENABLE);                // Start Timer
	}
	TIM_ClearIntPending(LPC_TIM3, TIM_MR0_INT);  // clear Interrupt
}


/*********************************************************************//**
 * @brief 		Get peripheral clock of each timer controller
 * @param[in]	timernum Timer number
 * @return 		Peripheral clock of timer
 **********************************************************************/
static uint32_t getPClock (uint32_t timernum)
{
	uint32_t clkdlycnt;
	switch (timernum)
	{
	case 0:
		clkdlycnt = CLKPWR_GetPCLK (CLKPWR_PCLKSEL_TIMER0);
		break;

	case 1:
		clkdlycnt = CLKPWR_GetPCLK (CLKPWR_PCLKSEL_TIMER1);
		break;

	case 2:
		clkdlycnt = CLKPWR_GetPCLK (CLKPWR_PCLKSEL_TIMER2);
		break;

	case 3:
		clkdlycnt = CLKPWR_GetPCLK (CLKPWR_PCLKSEL_TIMER3);
		break;
	}
	return clkdlycnt;
}


/*********************************************************************//**
 * @brief 		Convert a time to a timer count value
 * @param[in]	timernum Timer number
 * @param[in]	usec Time in microseconds
 * @return 		The number of required clock ticks to give the time delay
 **********************************************************************/
uint32_t converUSecToVal (uint32_t timernum, uint32_t usec)
{
	uint64_t clkdlycnt;

	// Get Pclock of timer
	clkdlycnt = (uint64_t) getPClock(timernum);

	clkdlycnt = (clkdlycnt * usec) / 1000000;
	return (uint32_t) clkdlycnt;
}


/*********************************************************************//**
 * @brief 		Convert a timer register pointer to a timer number
 * @param[in]	TIMx Pointer to LPC_TIM_TypeDef, should be:
 * 				- LPC_TIM0: TIMER0 peripheral
 * 				- LPC_TIM1: TIMER1 peripheral
 * 				- LPC_TIM2: TIMER2 peripheral
 * 				- LPC_TIM3: TIMER3 peripheral
 * @return 		The timer number (0 to 3) or -1 if register pointer is bad
 **********************************************************************/
uint32_t converPtrToTimeNum (LPC_TIM_TypeDef *TIMx)
{
	uint32_t tnum = -1;

	if (TIMx == LPC_TIM0)
	{
		tnum = 0;
	}
	else if (TIMx == LPC_TIM1)
	{
		tnum = 1;
	}
	else if (TIMx == LPC_TIM2)
	{
		tnum = 2;
	}
	else if (TIMx == LPC_TIM3)
	{
		tnum = 3;
	}

	return tnum;
}

/* End of Private Functions ---------------------------------------------------- */


/* Public Functions ----------------------------------------------------------- */
/** @addtogroup TIM_Public_Functions
 * @{
 */

/* TIM Initialization Config function ---------------------------------*/

/********************************************************************//**
* @brief		Configures the TIM0 peripheral according to the specified
*               parameters.
* @param[in]	None
* @return 		None
*********************************************************************/
void TIM0_Config (void)
{
	// TIM Configuration structure variable
	TIM_TIMERCFG_Type TIM_ConfigStruct;
	// TIM Match configuration Structure variable
	TIM_MATCHCFG_Type TIM_MatchConfigStruct;

	// Initialize timer, prescale count time of 100uS
	TIM_ConfigStruct.PrescaleOption = TIM_PRESCALE_USVAL;
	TIM_ConfigStruct.PrescaleValue	= 100;

	// Use channel PCfg
	TIM_MatchConfigStruct.MatchChannel = 1;

	// Disable interrupt when MR0 matches the value in TC register
	TIM_MatchConfigStruct.IntOnMatch   = TRUE;
	// Enable reset on MR0: TIMER will reset if MR0 matches it
	TIM_MatchConfigStruct.ResetOnMatch = TRUE;
	// Stop on MR0 if MR0 matches it
	TIM_MatchConfigStruct.StopOnMatch  = FALSE;
	// Toggle MR0 pin if MR0 matches it
	TIM_MatchConfigStruct.ExtMatchOutputType =TIM_EXTMATCH_TOGGLE;
	// Set Match value, count value of 1000 (1000 * 100uS = 100mS --> 10Hz)
	TIM_MatchConfigStruct.MatchValue   = 1000;

	// Set configuration for Tim_config and Tim_MatchConfig
	TIM_Init(LPC_TIM0, TIM_TIMER_MODE,&TIM_ConfigStruct);
	TIM_ConfigMatch(LPC_TIM0, &TIM_MatchConfigStruct);

	/* preemption = 1, sub-priority = 1 */
	NVIC_SetPriority(TIMER0_IRQn, 1);
	/* Enable interrupt for timer 1 */
	NVIC_EnableIRQ(TIMER0_IRQn);

	TIM_Cmd(LPC_TIM0, ENABLE);
}


/********************************************************************//**
* @brief		Configures the TIM1 peripheral according to the specified
*               parameters.
* @param[in]	None
* @return 		None
*********************************************************************/
void TIM1_Config (void)
{
	// TIM Configuration structure variable
	TIM_TIMERCFG_Type TIM_ConfigStruct;
	// TIM Match configuration Structure variable
	TIM_MATCHCFG_Type TIM_MatchConfigStruct;
	// TIM Capture configuration Structure variable
	TIM_CAPTURECFG_Type TIM_CaptureConfigStruct;

	// Initialize timer, prescale count time of 1mS
	TIM_ConfigStruct.PrescaleOption = TIM_PRESCALE_USVAL;
	TIM_ConfigStruct.PrescaleValue	= 100;

	// use channel 0, CAPn.0
	TIM_CaptureConfigStruct.CaptureChannel = 0;
	// Enable capture on CAPn.0 rising edge
	TIM_CaptureConfigStruct.RisingEdge = ENABLE;
	// Enable capture on CAPn.0 falling edge
	TIM_CaptureConfigStruct.FallingEdge = DISABLE;
	// Generate capture interrupt
	TIM_CaptureConfigStruct.IntOnCaption = ENABLE;


	// Use channel PCfg
	TIM_MatchConfigStruct.MatchChannel = 0;
	// Disable interrupt when MR0 matches the value in TC register
	TIM_MatchConfigStruct.IntOnMatch   = TRUE;
	// Enable reset on MR0: TIMER will reset if MR0 matches it
	TIM_MatchConfigStruct.ResetOnMatch = TRUE;
	// Stop on MR0 if MR0 matches it
	TIM_MatchConfigStruct.StopOnMatch  = FALSE;
	// Toggle MR0 pin if MR0 matches it
	TIM_MatchConfigStruct.ExtMatchOutputType =TIM_EXTMATCH_TOGGLE;
	// Set Match value, count value of 1000 (1000 * 100uS = 100mS --> 10Hz)
	TIM_MatchConfigStruct.MatchValue   = 1000;


	// Set configuration for Tim_config and Tim_MatchConfig
	TIM_Init(LPC_TIM1, TIM_TIMER_MODE,&TIM_ConfigStruct);
//	TIM_ConfigMatch(LPC_TIM1, &TIM_MatchConfigStruct);
	TIM_ConfigCapture(LPC_TIM1, &TIM_CaptureConfigStruct);
	TIM_ResetCounter(LPC_TIM1);

	/* preemption = 1, sub-priority = 1 */
	NVIC_SetPriority(TIMER1_IRQn, 1);
	/* Enable interrupt for timer 1 */
	NVIC_EnableIRQ(TIMER1_IRQn);

	TIM_Cmd(LPC_TIM1, ENABLE);
}


/********************************************************************//**
* @brief		Configures the TIM2 peripheral according to the specified
*               parameters.
* @param[in]	None
* @return 		None
*********************************************************************/
void TIM2_Config (void)
{
	// TIM Configuration structure variable
	TIM_TIMERCFG_Type TIM_ConfigStruct;
	// TIM Match configuration Structure variable
	TIM_MATCHCFG_Type TIM_MatchConfigStruct;

	// Initialize timer, prescale count time of 100uS
	TIM_ConfigStruct.PrescaleOption = TIM_PRESCALE_USVAL;
	TIM_ConfigStruct.PrescaleValue	= 1;

	// Use channel PCfg
	TIM_MatchConfigStruct.MatchChannel = 0;

	// Disable interrupt when MR0 matches the value in TC register
	TIM_MatchConfigStruct.IntOnMatch   = FALSE;
	// Enable reset on MR0: TIMER will reset if MR0 matches it
	TIM_MatchConfigStruct.ResetOnMatch = TRUE;
	// Stop on MR0 if MR0 matches it
	TIM_MatchConfigStruct.StopOnMatch  = FALSE;
	// Toggle MR0 pin if MR0 matches it
	TIM_MatchConfigStruct.ExtMatchOutputType =TIM_EXTMATCH_TOGGLE;
	// Set Match value, count value of 1000 (1000 * 100uS = 100mS --> 10Hz)
	TIM_MatchConfigStruct.MatchValue   = (500000/820);

	// Set configuration for Tim_config and Tim_MatchConfig
	TIM_Init(LPC_TIM2, TIM_TIMER_MODE,&TIM_ConfigStruct);
	TIM_ConfigMatch(LPC_TIM2, &TIM_MatchConfigStruct);

	/* preemption = 1, sub-priority = 1 */
//	NVIC_SetPriority(TIMER2_IRQn, 1);
	/* Enable interrupt for timer 2 */
//	NVIC_EnableIRQ(TIMER2_IRQn);

	TIM_Cmd(LPC_TIM2, ENABLE);
}


/********************************************************************//**
* @brief		Configures the TIM3 peripheral according to the specified
*               parameters.
* @param[in]	None
* @return 		None
*********************************************************************/
void TIM3_Config (void)
{
	// TIM Configuration structure variable
	TIM_TIMERCFG_Type TIM_ConfigStruct;
	// TIM Match configuration Structure variable
	TIM_MATCHCFG_Type TIM_MatchConfigStruct;

	// Initialize timer, prescale count time of 100uS
	TIM_ConfigStruct.PrescaleOption = TIM_PRESCALE_USVAL;
	TIM_ConfigStruct.PrescaleValue	= 100;

	// Use channel PCfg
	TIM_MatchConfigStruct.MatchChannel = 0;

	// Disable interrupt when MR0 matches the value in TC register
	TIM_MatchConfigStruct.IntOnMatch   = TRUE;
	// Enable reset on MR0: TIMER will reset if MR0 matches it
	TIM_MatchConfigStruct.ResetOnMatch = TRUE;
	// Stop on MR0 if MR0 matches it
	TIM_MatchConfigStruct.StopOnMatch  = FALSE;
	// Toggle MR0 pin if MR0 matches it
	TIM_MatchConfigStruct.ExtMatchOutputType =TIM_EXTMATCH_TOGGLE;
	// Set Match value, count value of 1000 (1000 * 100uS = 100mS --> 10Hz)
	TIM_MatchConfigStruct.MatchValue   = T1*10;

	// Set configuration for Tim_config and Tim_MatchConfig
	TIM_Init(LPC_TIM3, TIM_TIMER_MODE,&TIM_ConfigStruct);
	TIM_ConfigMatch(LPC_TIM3, &TIM_MatchConfigStruct);

	/* preemption = 1, sub-priority = 1 */
	NVIC_SetPriority(TIMER3_IRQn, 1);
	/* Enable interrupt for timer 3 */
	NVIC_EnableIRQ(TIMER3_IRQn);

	TIM_Cmd(LPC_TIM3, ENABLE);
}


/********************************************************************//**
* @brief		Initializes the TIMx peripheral according to the specified
*               parameters.
* @param[in]	TIMx	Timer peripheral selected, should be:
*   			- LPC_TIM0: TIMER0 peripheral
* 				- LPC_TIM1: TIMER1 peripheral
* 				- LPC_TIM2: TIMER2 peripheral
* 				- LPC_TIM3: TIMER3 peripheral
* @param[in]	IntFlag: interrupt type, should be:
* 				- None   : No Pin Configuration
* 				- TIM_MR0: Configure for Ext Match channel 0
* 				- TIM_MR1: Configure for Ext Match channel 1
* 				- TIM_MR2: Configure for Ext Match channel 2 for only Timer2
* 				- TIM_MR3: Configure for Ext Match channel 3 for only Timer2
* 				- TIM_CR0: Configure for Capture channel 0
* 				- TIM_CR1: Configure for Capture channel 1
* @return 		None
*********************************************************************/
void TIM_Config(LPC_TIM_TypeDef *TIMx, TIM_PCFG_TYPE PCfg)
{
	// Pin configuration for TIM
	PINSEL_CFG_Type PinCfg;

	if (TIMx == LPC_TIM0)
	{
		switch (PCfg)
		{
		 case TIM_MR1:
			 // Configure P3.26 as MAT0.1
			 PinCfg.Funcnum = 2;
			 PinCfg.OpenDrain = 0;
			 PinCfg.Pinmode = 0;
			 PinCfg.Portnum = 3;
			 PinCfg.Pinnum = 26;
			 PINSEL_ConfigPin(&PinCfg);
			 break;

		 case None:
			 break;

		 default:
		 		//Error match value
		 		//Error loop
		 		while(1);
		}

		// Pin Configuration
		TIM0_Config();    // Timer0 Configuration
	}
	else if (TIMx == LPC_TIM1)
	{
		switch (PCfg)
		{
		 case TIM_MR0:
			 // Configure P1.22 as MAT1.0
			 PinCfg.Funcnum = 3;
			 PinCfg.OpenDrain = 0;
			 PinCfg.Pinmode = 0;
			 PinCfg.Portnum = 1;
			 PinCfg.Pinnum = 22;
			 PINSEL_ConfigPin(&PinCfg);
			 break;

		 case TIM_MR1:
			 // Configure P1.25 as MAT1.1
			 PinCfg.Funcnum = 3;
			 PinCfg.OpenDrain = 0;
			 PinCfg.Pinmode = 0;
			 PinCfg.Portnum = 1;
			 PinCfg.Pinnum = 25;
			 PINSEL_ConfigPin(&PinCfg);
			 break;

		 case TIM_CR0:
			 // Configure P1.18 as CAP1.0
			 PinCfg.Funcnum = 3;
			 PinCfg.OpenDrain = 0;
			 PinCfg.Pinmode = 0;
			 PinCfg.Portnum = 1;
			 PinCfg.Pinnum = 18;
			 PINSEL_ConfigPin(&PinCfg);
			 break;

		 case TIM_CR1:
			 // Configure P1.19 as CAP1.1
			 PinCfg.Funcnum = 3;
			 PinCfg.OpenDrain = 0;
			 PinCfg.Pinmode = 0;
			 PinCfg.Portnum = 1;
			 PinCfg.Pinnum = 19;
			 PINSEL_ConfigPin(&PinCfg);
			 break;

		 case None:
			 break;

		 default:
		 		//Error match value
		 		//Error loop
		 		while(1);
		}

		TIM1_Config();   // Timer1 Configuration
	}
	else if (TIMx == LPC_TIM2)
	{
		switch (PCfg)
		{
		 case TIM_MR0:
			 // Configure P4.28 as MAT2.0
			 PinCfg.Funcnum = 2;
			 PinCfg.OpenDrain = 0;
			 PinCfg.Pinmode = 0;
			 PinCfg.Portnum = 4;
			 PinCfg.Pinnum = 28;
			 PINSEL_ConfigPin(&PinCfg);
			 break;

		 case None:
			 break;

		 default:
		 		//Error match value
		 		//Error loop
		 		while(1);
		}

		// Pin Configuration
		TIM2_Config();    // Timer2 Configuration
	}
	else if (TIMx == LPC_TIM3)
	{
		switch (PCfg)
		{
		 case TIM_MR0:
			 // Configure P0.10 as MAT3.0
			 PinCfg.Funcnum = 3;
			 PinCfg.OpenDrain = 0;
			 PinCfg.Pinmode = 0;
			 PinCfg.Portnum = 0;
			 PinCfg.Pinnum = 10;
			 PINSEL_ConfigPin(&PinCfg);
			 break;

		 case TIM_MR1:
			 // Configure P0.11 as MAT3.1
			 PinCfg.Funcnum = 3;
			 PinCfg.OpenDrain = 0;
			 PinCfg.Pinmode = 0;
			 PinCfg.Portnum = 0;
			 PinCfg.Pinnum = 11;
			 PINSEL_ConfigPin(&PinCfg);
			 break;

		 case None:
			 break;

		 default:
		 		//Error match value
		 		//Error loop
		 		while(1);
		}

		// Pin Configuration
		TIM3_Config();    // Timer3 Configuration
	}
}


/*********************************************************************//**
 * @brief 		Get Interrupt Status
 * @param[in]	TIMx Timer selection, should be:
 *   			- LPC_TIM0: TIMER0 peripheral
 * 				- LPC_TIM1: TIMER1 peripheral
 * 				- LPC_TIM2: TIMER2 peripheral
 * 				- LPC_TIM3: TIMER3 peripheral
 * @param[in]	IntFlag: interrupt type, should be:
 * 				- TIM_MR0_INT: Interrupt for Match channel 0
 * 				- TIM_MR1_INT: Interrupt for Match channel 1
 * 				- TIM_MR2_INT: Interrupt for Match channel 2 for only Timer2
 * 				- TIM_MR3_INT: Interrupt for Match channel 3 for only Timer2
 * 				- TIM_CR0_INT: Interrupt for Capture channel 0
 * 				- TIM_CR1_INT: Interrupt for Capture channel 1
 * @return 		FlagStatus
 * 				- SET : interrupt
 * 				- RESET : no interrupt
 **********************************************************************/
FlagStatus TIM_GetIntStatus(LPC_TIM_TypeDef *TIMx, TIM_INT_TYPE IntFlag)
{
	uint8_t temp;
	CHECK_PARAM(PARAM_TIMx(TIMx));
	CHECK_PARAM(PARAM_TIM_INT_TYPE(IntFlag));
	temp = (TIMx->IR)& TIM_IR_CLR(IntFlag);
	if (temp)
		return SET;

	return RESET;

}
/*********************************************************************//**
 * @brief 		Get Capture Interrupt Status
 * @param[in]	TIMx Timer selection, should be:
 *  	   		- LPC_TIM0: TIMER0 peripheral
 * 				- LPC_TIM1: TIMER1 peripheral
 * 				- LPC_TIM2: TIMER2 peripheral
 * 				- LPC_TIM3: TIMER3 peripheral
 * @param[in]	IntFlag: interrupt type, should be:
 * 				- TIM_MR0_INT: Interrupt for Match channel 0
 * 				- TIM_MR1_INT: Interrupt for Match channel 1
 * 				- TIM_MR2_INT: Interrupt for Match channel 2 for only Timer2
 * 				- TIM_MR3_INT: Interrupt for Match channel 3 for only Timer2
 * 				- TIM_CR0_INT: Interrupt for Capture channel 0
 * 				- TIM_CR1_INT: Interrupt for Capture channel 1
 * @return 		FlagStatus
 * 				- SET : interrupt
 * 				- RESET : no interrupt
 **********************************************************************/
FlagStatus TIM_GetIntCaptureStatus(LPC_TIM_TypeDef *TIMx, TIM_INT_TYPE IntFlag)
{
	uint8_t temp;
	CHECK_PARAM(PARAM_TIMx(TIMx));
	CHECK_PARAM(PARAM_TIM_INT_TYPE(IntFlag));
	temp = (TIMx->IR) & (1<<(4+IntFlag));
	if(temp)
		return SET;
	return RESET;
}
/*********************************************************************//**
 * @brief 		Clear Interrupt pending
 * @param[in]	TIMx Timer selection, should be:
 *    			- LPC_TIM0: TIMER0 peripheral
 * 				- LPC_TIM1: TIMER1 peripheral
 * 				- LPC_TIM2: TIMER2 peripheral
 * 				- LPC_TIM3: TIMER3 peripheral
 * @param[in]	IntFlag: interrupt type, should be:
 * 				- TIM_MR0_INT: Interrupt for Match channel 0
 * 				- TIM_MR1_INT: Interrupt for Match channel 1
 * 				- TIM_MR2_INT: Interrupt for Match channel 2 for only Timer2
 * 				- TIM_MR3_INT: Interrupt for Match channel 3 for only Timer2
 * 				- TIM_CR0_INT: Interrupt for Capture channel 0
 * 				- TIM_CR1_INT: Interrupt for Capture channel 1
 * @return 		None
 **********************************************************************/
void TIM_ClearIntPending(LPC_TIM_TypeDef *TIMx, TIM_INT_TYPE IntFlag)
{
	CHECK_PARAM(PARAM_TIMx(TIMx));
	CHECK_PARAM(PARAM_TIM_INT_TYPE(IntFlag));
	TIMx->IR = TIM_IR_CLR(IntFlag);
}

/*********************************************************************//**
 * @brief 		Clear Capture Interrupt pending
 * @param[in]	TIMx Timer selection, should be
 *    			- LPC_TIM0: TIMER0 peripheral
 * 				- LPC_TIM1: TIMER1 peripheral
 * 				- LPC_TIM2: TIMER2 peripheral
 * 				- LPC_TIM3: TIMER3 peripheral
 * @param[in]	IntFlag interrupt type, should be:
 *				- TIM_MR0_INT: Interrupt for Match channel 0
 * 				- TIM_MR1_INT: Interrupt for Match channel 1
 * 				- TIM_MR2_INT: Interrupt for Match channel 2
 * 				- TIM_MR3_INT: Interrupt for Match channel 3
 * 				- TIM_CR0_INT: Interrupt for Capture channel 0
 * 				- TIM_CR1_INT: Interrupt for Capture channel 1
 * @return 		None
 **********************************************************************/
void TIM_ClearIntCapturePending(LPC_TIM_TypeDef *TIMx, TIM_INT_TYPE IntFlag)
{
	CHECK_PARAM(PARAM_TIMx(TIMx));
	CHECK_PARAM(PARAM_TIM_INT_TYPE(IntFlag));
	TIMx->IR = (1<<(4+IntFlag));
}

/*********************************************************************//**
 * @brief 		Configuration for Timer at initial time
 * @param[in] 	TimerCounterMode timer counter mode, should be:
 * 				- TIM_TIMER_MODE: Timer mode
 * 				- TIM_COUNTER_RISING_MODE: Counter rising mode
 * 				- TIM_COUNTER_FALLING_MODE: Counter falling mode
 * 				- TIM_COUNTER_ANY_MODE:Counter on both edges
 * @param[in] 	TIM_ConfigStruct pointer to TIM_TIMERCFG_Type or
 * 				TIM_COUNTERCFG_Type
 * @return 		None
 **********************************************************************/
void TIM_ConfigStructInit(TIM_MODE_OPT TimerCounterMode, void *TIM_ConfigStruct)
{
	if (TimerCounterMode == TIM_TIMER_MODE )
	{
		TIM_TIMERCFG_Type * pTimeCfg = (TIM_TIMERCFG_Type *)TIM_ConfigStruct;
		pTimeCfg->PrescaleOption = TIM_PRESCALE_USVAL;
		pTimeCfg->PrescaleValue = 1;
	}
	else
	{
		TIM_COUNTERCFG_Type * pCounterCfg = (TIM_COUNTERCFG_Type *)TIM_ConfigStruct;
		pCounterCfg->CountInputSelect = TIM_COUNTER_INCAP0;
	}
}

/*********************************************************************//**
 * @brief 		Initial Timer/Counter device
 * 				 	Set Clock frequency for Timer
 * 					Set initial configuration for Timer
 * @param[in]	TIMx  Timer selection, should be:
 * 				- LPC_TIM0: TIMER0 peripheral
 * 				- LPC_TIM1: TIMER1 peripheral
 * 				- LPC_TIM2: TIMER2 peripheral
 * 				- LPC_TIM3: TIMER3 peripheral
 * @param[in]	TimerCounterMode Timer counter mode, should be:
 * 				- TIM_TIMER_MODE: Timer mode
 * 				- TIM_COUNTER_RISING_MODE: Counter rising mode
 * 				- TIM_COUNTER_FALLING_MODE: Counter falling mode
 * 				- TIM_COUNTER_ANY_MODE:Counter on both edges
 * @param[in]	TIM_ConfigStruct pointer to TIM_TIMERCFG_Type
 * 				that contains the configuration information for the
 *                    specified Timer peripheral.
 * @return 		None
 **********************************************************************/
void TIM_Init(LPC_TIM_TypeDef *TIMx, TIM_MODE_OPT TimerCounterMode, void *TIM_ConfigStruct)
{
	TIM_TIMERCFG_Type *pTimeCfg;
	TIM_COUNTERCFG_Type *pCounterCfg;

	CHECK_PARAM(PARAM_TIMx(TIMx));
	CHECK_PARAM(PARAM_TIM_MODE_OPT(TimerCounterMode));

	//set power

	if (TIMx== LPC_TIM0)
	{
		CLKPWR_ConfigPPWR (CLKPWR_PCONP_PCTIM0, ENABLE);
		//PCLK_Timer0 = CCLK/4
		CLKPWR_SetPCLKDiv (CLKPWR_PCLKSEL_TIMER0, CLKPWR_PCLKSEL_CCLK_DIV_4);
	}
	else if (TIMx== LPC_TIM1)
	{
		CLKPWR_ConfigPPWR (CLKPWR_PCONP_PCTIM1, ENABLE);
		//PCLK_Timer1 = CCLK/4
		CLKPWR_SetPCLKDiv (CLKPWR_PCLKSEL_TIMER1, CLKPWR_PCLKSEL_CCLK_DIV_4);

	}

	else if (TIMx== LPC_TIM2)
	{
		CLKPWR_ConfigPPWR (CLKPWR_PCONP_PCTIM2, ENABLE);
		//PCLK_Timer2= CCLK/4
		CLKPWR_SetPCLKDiv (CLKPWR_PCLKSEL_TIMER2, CLKPWR_PCLKSEL_CCLK_DIV_4);
	}
	else if (TIMx== LPC_TIM3)
	{
		CLKPWR_ConfigPPWR (CLKPWR_PCONP_PCTIM3, ENABLE);
		//PCLK_Timer3= CCLK/4
		CLKPWR_SetPCLKDiv (CLKPWR_PCLKSEL_TIMER3, CLKPWR_PCLKSEL_CCLK_DIV_4);

	}

	TIMx->CCR &= ~TIM_CTCR_MODE_MASK;
	TIMx->CCR |= TIM_TIMER_MODE;

	TIMx->TC =0;
	TIMx->PC =0;
	TIMx->PR =0;
	TIMx->TCR |= (1<<1); //Reset Counter
	TIMx->TCR &= ~(1<<1); //release reset
	if (TimerCounterMode == TIM_TIMER_MODE )
	{
		pTimeCfg = (TIM_TIMERCFG_Type *)TIM_ConfigStruct;
		if (pTimeCfg->PrescaleOption  == TIM_PRESCALE_TICKVAL)
		{
			TIMx->PR   = pTimeCfg->PrescaleValue -1  ;
		}
		else
		{
			TIMx->PR   = converUSecToVal (converPtrToTimeNum(TIMx),pTimeCfg->PrescaleValue)-1;
		}
	}
	else
	{

		pCounterCfg = (TIM_COUNTERCFG_Type *)TIM_ConfigStruct;
		TIMx->CCR  &= ~TIM_CTCR_INPUT_MASK;
		if (pCounterCfg->CountInputSelect == TIM_COUNTER_INCAP1)
			TIMx->CCR |= _BIT(2);
	}

	// Clear interrupt pending
	TIMx->IR = 0xFFFFFFFF;

}

/*********************************************************************//**
 * @brief 		Close Timer/Counter device
 * @param[in]	TIMx  Pointer to timer device, should be:
 * 				- LPC_TIM0: TIMER0 peripheral
 * 				- LPC_TIM1: TIMER1 peripheral
 * 				- LPC_TIM2: TIMER2 peripheral
 * 				- LPC_TIM3: TIMER3 peripheral
 * @return 		None
 **********************************************************************/
void TIM_DeInit (LPC_TIM_TypeDef *TIMx)
{
	CHECK_PARAM(PARAM_TIMx(TIMx));
	// Disable timer/counter
	TIMx->TCR = 0x00;

	// Disable power
	if (TIMx== LPC_TIM0)
		CLKPWR_ConfigPPWR (CLKPWR_PCONP_PCTIM0, DISABLE);

	else if (TIMx== LPC_TIM1)
		CLKPWR_ConfigPPWR (CLKPWR_PCONP_PCTIM1, DISABLE);

	else if (TIMx== LPC_TIM2)
		CLKPWR_ConfigPPWR (CLKPWR_PCONP_PCTIM2, DISABLE);

	else if (TIMx== LPC_TIM3)
		CLKPWR_ConfigPPWR (CLKPWR_PCONP_PCTIM2, DISABLE);

}

/*********************************************************************//**
 * @brief	 	Start/Stop Timer/Counter device
 * @param[in]	TIMx Pointer to timer device, should be:
 *  			- LPC_TIM0: TIMER0 peripheral
 * 				- LPC_TIM1: TIMER1 peripheral
 * 				- LPC_TIM2: TIMER2 peripheral
 * 				- LPC_TIM3: TIMER3 peripheral
 * @param[in]	NewState
 * 				-	ENABLE  : set timer enable
 * 				-	DISABLE : disable timer
 * @return 		None
 **********************************************************************/
void TIM_Cmd(LPC_TIM_TypeDef *TIMx, FunctionalState NewState)
{
	CHECK_PARAM(PARAM_TIMx(TIMx));
	if (NewState == ENABLE)
	{
		TIMx->TCR	|=  TIM_ENABLE;
	}
	else
	{
		TIMx->TCR &= ~TIM_ENABLE;
	}
}

/*********************************************************************//**
 * @brief 		Reset Timer/Counter device,
 * 					Make TC and PC are synchronously reset on the next
 * 					positive edge of PCLK
 * @param[in]	TIMx Pointer to timer device, should be:
 *   			- LPC_TIM0: TIMER0 peripheral
 * 				- LPC_TIM1: TIMER1 peripheral
 * 				- LPC_TIM2: TIMER2 peripheral
 * 				- LPC_TIM3: TIMER3 peripheral
 * @return 		None
 **********************************************************************/
void TIM_ResetCounter(LPC_TIM_TypeDef *TIMx)
{
	CHECK_PARAM(PARAM_TIMx(TIMx));
	TIMx->TCR |= TIM_RESET;
	TIMx->TCR &= ~TIM_RESET;
}

/*********************************************************************//**
 * @brief 		Configuration for Match register
 * @param[in]	TIMx Pointer to timer device, should be:
 *   			- LPC_TIM0: TIMER0 peripheral
 * 				- LPC_TIM1: TIMER1 peripheral
 * 				- LPC_TIM2: TIMER2 peripheral
 * 				- LPC_TIM3: TIMER3 peripheral
 * @param[in]   TIM_MatchConfigStruct Pointer to TIM_MATCHCFG_Type
 * 					- MatchChannel : choose channel 0 or 1
 * 					- IntOnMatch	 : if SET, interrupt will be generated when MRxx match
 * 									the value in TC
 * 					- StopOnMatch	 : if SET, TC and PC will be stopped whenM Rxx match
 * 									the value in TC
 * 					- ResetOnMatch : if SET, Reset on MR0 when MRxx match
 * 									the value in TC
 * 					-ExtMatchOutputType: Select output for external match
 * 						 +	 0:	Do nothing for external output pin if match
 *						 +   1:	Force external output pin to low if match
 *						 + 	 2: Force external output pin to high if match
 *						 + 	 3: Toggle external output pin if match
 *					MatchValue: Set the value to be compared with TC value
 * @return 		None
 **********************************************************************/
void TIM_ConfigMatch(LPC_TIM_TypeDef *TIMx, TIM_MATCHCFG_Type *TIM_MatchConfigStruct)
{

	CHECK_PARAM(PARAM_TIMx(TIMx));
	CHECK_PARAM(PARAM_TIM_EXTMATCH_OPT(TIM_MatchConfigStruct->ExtMatchOutputType));

	switch(TIM_MatchConfigStruct->MatchChannel)
	{
	case 0:
		TIMx->MR0 = TIM_MatchConfigStruct->MatchValue;
		break;
	case 1:
		TIMx->MR1 = TIM_MatchConfigStruct->MatchValue;
		break;
	case 2:
		TIMx->MR2 = TIM_MatchConfigStruct->MatchValue;
		break;
	case 3:
		TIMx->MR3 = TIM_MatchConfigStruct->MatchValue;
		break;
	default:
		//Error match value
		//Error loop
		while(1);
	}
	//interrupt on MRn
	TIMx->MCR &=~TIM_MCR_CHANNEL_MASKBIT(TIM_MatchConfigStruct->MatchChannel);

	if (TIM_MatchConfigStruct->IntOnMatch)
		TIMx->MCR |= TIM_INT_ON_MATCH(TIM_MatchConfigStruct->MatchChannel);

	//reset on MRn
	if (TIM_MatchConfigStruct->ResetOnMatch)
		TIMx->MCR |= TIM_RESET_ON_MATCH(TIM_MatchConfigStruct->MatchChannel);

	//stop on MRn
	if (TIM_MatchConfigStruct->StopOnMatch)
		TIMx->MCR |= TIM_STOP_ON_MATCH(TIM_MatchConfigStruct->MatchChannel);

	// match output type

	TIMx->EMR 	&= ~TIM_EM_MASK(TIM_MatchConfigStruct->MatchChannel);
	TIMx->EMR   |= TIM_EM_SET(TIM_MatchConfigStruct->MatchChannel,TIM_MatchConfigStruct->ExtMatchOutputType);
}
/*********************************************************************//**
 * @brief 		Update Match value
 * @param[in]	TIMx Pointer to timer device, should be:
 *   			- LPC_TIM0: TIMER0 peripheral
 * 				- LPC_TIM1: TIMER1 peripheral
 * 				- LPC_TIM2: TIMER2 peripheral
 * 				- LPC_TIM3: TIMER3 peripheral
 * @param[in]	MatchChannel	Match channel, should be: 0..3
 * @param[in]	MatchValue		updated match value
 * @return 		None
 **********************************************************************/
void TIM_UpdateMatchValue(LPC_TIM_TypeDef *TIMx,uint8_t MatchChannel, uint32_t MatchValue)
{
	CHECK_PARAM(PARAM_TIMx(TIMx));
	switch(MatchChannel)
	{
	case 0:
		TIMx->MR0 = MatchValue;
		break;
	case 1:
		TIMx->MR1 = MatchValue;
		break;
	case 2:
		TIMx->MR2 = MatchValue;
		break;
	case 3:
		TIMx->MR3 = MatchValue;
		break;
	default:
		//Error Loop
		while(1);
	}

}
/*********************************************************************//**
 * @brief 		Configuration for Capture register
 * @param[in]	TIMx Pointer to timer device, should be:
 *   			- LPC_TIM0: TIMER0 peripheral
 * 				- LPC_TIM1: TIMER1 peripheral
 * 				- LPC_TIM2: TIMER2 peripheral
 * 				- LPC_TIM3: TIMER3 peripheral
 * 					- CaptureChannel: set the channel to capture data
 * 					- RisingEdge    : if SET, Capture at rising edge
 * 					- FallingEdge	: if SET, Capture at falling edge
 * 					- IntOnCaption  : if SET, Capture generate interrupt
 * @param[in]   TIM_CaptureConfigStruct	Pointer to TIM_CAPTURECFG_Type
 * @return 		None
 **********************************************************************/
void TIM_ConfigCapture(LPC_TIM_TypeDef *TIMx, TIM_CAPTURECFG_Type *TIM_CaptureConfigStruct)
{

	CHECK_PARAM(PARAM_TIMx(TIMx));
	TIMx->CCR &= ~TIM_CCR_CHANNEL_MASKBIT(TIM_CaptureConfigStruct->CaptureChannel);

	if (TIM_CaptureConfigStruct->RisingEdge)
		TIMx->CCR |= TIM_CAP_RISING(TIM_CaptureConfigStruct->CaptureChannel);

	if (TIM_CaptureConfigStruct->FallingEdge)
		TIMx->CCR |= TIM_CAP_FALLING(TIM_CaptureConfigStruct->CaptureChannel);

	if (TIM_CaptureConfigStruct->IntOnCaption)
		TIMx->CCR |= TIM_INT_ON_CAP(TIM_CaptureConfigStruct->CaptureChannel);
}

/*********************************************************************//**
 * @brief 		Read value of capture register in timer/counter device
 * @param[in]	TIMx Pointer to timer/counter device, should be:
 *  			- LPC_TIM0: TIMER0 peripheral
 * 				- LPC_TIM1: TIMER1 peripheral
 * 				- LPC_TIM2: TIMER2 peripheral
 * 				- LPC_TIM3: TIMER3 peripheral
 * @param[in]	CaptureChannel: capture channel number, should be:
 * 				- TIM_COUNTER_INCAP0: CAPn.0 input pin for TIMERn
 * 				- TIM_COUNTER_INCAP1: CAPn.1 input pin for TIMERn
 * @return 		Value of capture register
 **********************************************************************/
uint32_t TIM_GetCaptureValue(LPC_TIM_TypeDef *TIMx, TIM_COUNTER_INPUT_OPT CaptureChannel)
{
	CHECK_PARAM(PARAM_TIMx(TIMx));
	CHECK_PARAM(PARAM_TIM_COUNTER_INPUT_OPT(CaptureChannel));

	if(CaptureChannel==0)
		return TIMx->CR0;
	else
		return TIMx->CR1;
}

/********************************************************************//**
* @brief Micro Sec delay function using Timer 3(Polling Mode)
**********************************************************************/

/** @defgroup micro_sec_Public_Functions micro_sec Public Functions
 * @{
 */

/*********************************************************************//**
 * @brief	Initialization of the timer3(Polling Mode)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void US_TimerInit(void)
{
	if(tim_init)		
	{
		return;
	}
	tim_init=1;

	LPC_SC->PCONP |= 1<<23;	/* Power on timer 3 */
	LPC_TIM3->CTCR = 0; /* Timer mode  */
	LPC_TIM3->TCR = 2;  /* Reset timer */
	LPC_TIM3->PR = ((SystemCoreClock/4)/1000000)-1; /* Set the prescaler value 
													   for 1us */
	LPC_TIM3->TCR = 1;	/* Start the counter */
}

/*********************************************************************//**
 * @brief	stops the timer
 * @param[in]	None
 * @return 		value
 **********************************************************************/
inline void US_TimerStop(void)
{
	LPC_TIM3->TCR = 0;		/* Disable the counter */
}

/*********************************************************************//**
 * @brief	read the timer counter value
 * @param[in]	None
 * @return 		value
 **********************************************************************/
uint32_t US_TimerRead()
{
	if (!tim_init)
	{
		US_TimerInit();
	}
	return LPC_TIM3->TC;	// retuen timer counter value
}


/*********************************************************************//**
 * @brief	micro sec delay function
 * @param[in]	value in micro sec
 * @return 		None
 **********************************************************************/
void delay_us(uint32_t us)
{
	uint32_t start  = US_TimerRead();		/* Read timer counter value */
	while ((US_TimerRead() - start) < us);	/* delay loop */
	LPC_SC->PCONP |= 0<<23;		/* disable the timer 3 */
	LPC_TIM3->TCR = 0;
	tim_init=0;
}


/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

/* --------------------------------- End Of File ------------------------------ */
