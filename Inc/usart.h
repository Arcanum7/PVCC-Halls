#ifndef USART_H_
#define USART_H_

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_i2c.h"
#include "stm32f10x_iwdg.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ----------------------------------------------------------------------------
#define USART_RX_GPIO GPIOA
#define USART_RX_PIN  10
#define USART_TX_GPIO GPIOA
#define USART_TX_PIN  9


void USART_PutChar(uint8_t ch);
void USART_PutString(char * str);
void USARTConfig(void);
// ----------------------------------------------------------------------------

#endif // USART_H_
