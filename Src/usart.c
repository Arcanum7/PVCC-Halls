#include "usart.h"

void USART_PutChar(uint8_t ch)
{
	while(!(USART1->SR & USART_SR_TXE));
	GPIO_SetBits(GPIOA, GPIO_Pin_12);
	USART1->DR = ch;
	while(!(USART1->SR & USART_FLAG_TC));
	GPIO_ResetBits(GPIOA, GPIO_Pin_12);
}
//=============================================================================
//
//=============================================================================
void USART_PutString(char * str)
{
	/* Set DE pin to high level for transmit */
	GPIO_SetBits(GPIOA, GPIO_Pin_12);
    while(*str != 0)
	{
		while(!(USART1->SR & USART_SR_TXE));
		USART1->DR = (uint8_t)*str;
		while(!(USART1->SR & USART_FLAG_TC));
		str++;
	}
    /* Set DE pin to low level when done transmitting */
	GPIO_ResetBits(GPIOA, GPIO_Pin_12);
}

void USARTConfig(void)
{
	USART_InitTypeDef USAR;
	GPIO_InitTypeDef GPIOStruc;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO | RCC_APB2Periph_USART1,ENABLE);

	/* GPIOA PIN9 alternative function Tx */
	GPIOStruc.GPIO_Mode=GPIO_Mode_AF_PP ;
	GPIOStruc.GPIO_Speed=GPIO_Speed_2MHz;
	GPIOStruc.GPIO_Pin=GPIO_Pin_9;
	GPIO_Init(GPIOA,&GPIOStruc);

	/* GPIOA PIN9 alternative function Rx */
	GPIOStruc.GPIO_Mode=GPIO_Mode_IN_FLOATING ;
	GPIOStruc.GPIO_Speed = GPIO_Speed_2MHz;
	GPIOStruc.GPIO_Pin=GPIO_Pin_10;
	GPIO_Init(GPIOA,&GPIOStruc);

	// GPIOA PIN12 is used for RS485 toggling between transmit and receive */
	GPIOStruc.GPIO_Pin = GPIO_Pin_12;
	GPIOStruc.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIOStruc.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOA, &GPIOStruc);
	GPIO_ResetBits(GPIOA, GPIO_Pin_12);

	// setup to use 9600 8-N-1
	USAR.USART_BaudRate=9600;
	USAR.USART_StopBits=USART_StopBits_1;
	USAR.USART_WordLength=USART_WordLength_8b;
	USAR.USART_Parity=USART_Parity_No ;
	USAR.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
	USAR.USART_Mode=USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1,&USAR);
	USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);
	NVIC_EnableIRQ(USART1_IRQn);
	USART_Cmd(USART1,ENABLE);

}

