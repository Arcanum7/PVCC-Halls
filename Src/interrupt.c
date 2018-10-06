#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_i2c.h"
#include "stm32f10x_iwdg.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint16_t CCR1_Interval = 258;
extern uint32_t Global_time;
extern uint8_t esc, pos;
extern unsigned char bufferu[100];
extern uint32_t last_measure, esc_time;

void setup_tim2(void)
{
    // TIMER2
    TIM_TimeBaseInitTypeDef TIMER_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

  	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

  	TIM_TimeBaseStructInit(&TIMER_InitStructure);
    TIMER_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIMER_InitStructure.TIM_Prescaler = 280;
    TIMER_InitStructure.TIM_Period = 65535;
    TIM_TimeBaseInit(TIM2, &TIMER_InitStructure);

    TIM_OCInitTypeDef  TIM_OCInitStructure;
    TIM_OCStructInit (&TIM_OCInitStructure);
    /* just use basic Output Compare Mode*/
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Active;
    /* set the initial match interval for CC1 */
    TIM_OCInitStructure.TIM_Pulse = CCR1_Interval;
    TIM_OC1Init (TIM2, &TIM_OCInitStructure);

    /*
      * It is important to clear any pending interrupt flags since the timer
      * has been free-running since we last used it and that may generate
      * interrupts on match even though the associated interrupt event has
      * not been enabled.
      */
     TIM_ClearITPendingBit (TIM2, TIM_IT_CC1);
     /* put the counter into a known state */
     TIM_SetCounter (TIM2, 0);
     /* enable the interrupt for CC1 only */
     TIM_ITConfig (TIM2, TIM_IT_CC1, ENABLE);
    TIM_Cmd(TIM2, ENABLE);

    /* NVIC Configuration */
    /* Enable the TIM4_IRQn Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void TIM2_IRQHandler(void)
{
	/* run through the interrupt sources looking for a hit */
	if (TIM_GetITStatus (TIM2, TIM_IT_CC1) != RESET)
	{
		uint16_t CCR1_Current = TIM_GetCapture1 (TIM2);
		TIM_SetCompare1 (TIM2, CCR1_Current + CCR1_Interval);

		TIM_ClearITPendingBit (TIM2, TIM_IT_CC1);
		Global_time++;  // increments by 1ms every time this interrupt is called
	}
}


void USART1_IRQHandler(void)
{
	uint8_t temp;
	if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{
		temp=USART_ReceiveData(USART1);
		if(temp==0x1b){
			esc=1;
			pos=0;
			esc_time = Global_time;
		}
		else if(esc){
			bufferu[pos]=temp;
			pos+=1;
		}
	}
}
