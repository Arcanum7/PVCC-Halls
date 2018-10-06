// ----------------------------------------------------------------------------
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_i2c.h"
#include "stm32f10x_iwdg.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "interrupt.h"
#include "usart.h"
#include "timer.h"
#include "build_defs.h"
#include "ds18b20.h"

/*
 * this is the setup for a STM32F103C8T6 MCU on a small chinese made blue board
 * PA9 -> RS485 DI
 * PA10 -> RS485 RO
 * PA12 -> RS485 DE/RE
 * PB8 -> DS18B20 Data
 *
 */
// ----- main() ---------------------------------------------------------------
// this is the id used to select this device for all communications
#define MY_ID 'z'

// Sample pragmas to cope with warnings. Please note the related line at
// the end of this function, used to pop the compiler diagnostics status.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"

//USART_InitTypeDef USAR;

volatile uint32_t Global_time = 0L; // global time in ms
uint32_t last_measure, esc_time;
unsigned char pos;
volatile unsigned char bitpos;
volatile uint8_t stat;
volatile unsigned char bufferu[100];
volatile uint8_t temp;
uint8_t esc, address;
uint8_t version[12];

void setup_gpio(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |
						 RCC_APB2Periph_GPIOC|RCC_APB2Periph_AFIO, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;	// reduce power by setting unused pins to analog input
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	// Configure DS18B20 and 1-Wire on pin B8
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	// Configure pin in output push/pull mode for on-board LED
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

}

int main(int argc, char* argv[])
{
	char buf[40];
	uint16_t i;
	one_wire_device dev;
	simple_float *temperatures;
	int16_t temp, temp_f;

	esc = 0;
	pos = 0;
	// Configure all unused GPIO port pins in Analog Input mode
	setup_gpio();
	USARTConfig();
	GPIO_SetBits(GPIOC,GPIO_Pin_13);	// turn off the built in led
	GPIO_ResetBits(GPIOB, GPIO_Pin_8);	// this is the one-wire connection to the DS18B20s
	setup_tim2();	// setup timer2 interrupts.  These are used to update global_time
    setup_delay_timer();

    address = MY_ID;
	sprintf((char *)version,"%02d%02d%02d-%02d%02d", BUILD_YEAR - 2000, BUILD_MONTH, BUILD_DAY, BUILD_HOUR, BUILD_MIN);

	// Please remember about adding 4.7k pull-up resistor :)
	// if that isn't done, it won't find any ds18b20s
	ds18b20_init(GPIOB, GPIO_Pin_8);
	// Scan for DS18B20 devices on 1-Wire
	ds18b20_devices devices = ds18b20_get_devices(true);
	ds18b20_set_precision(2);	// just need 11-bit precision, not 12
	ds18b20_convert_temperature_all();		// start a temperature conversion so the first isn't 185
	ds18b20_wait_for_conversion();		// just a time-killer

	delay_ms(750);				// pause at startup
	sprintf((char *)buf,"%c:Running:%s:%02x\r\n",address,version,address);
	USART_PutString(buf);

	while (1)
	{
		if (esc && pos)	// is the escape flag set, and have we received an id?
		{
			if (bufferu[0] == address)  // address must match the switches read by mcp23017
			{
				GPIO_ResetBits(GPIOC,GPIO_Pin_13);	// turn LED on to show we are processing
				Global_time = 0L;					// reset the tick counter
				esc = 0;	// reset the escape character received flag
				devices = ds18b20_get_devices(true);	// run a scan to see what responds

				ds18b20_convert_temperature_all();		// start a temperature conversion
				ds18b20_wait_for_conversion();
				temperatures = ds18b20_read_temperature_all();
				for (i = 0; i < devices.size; ++i)	// loop through all of the DS18B20s
				{
					dev = devices.devices[i];
					temp = temperatures[i].integer;
					temp = temp * 1.8 + 32;	// convert to fahrenhiet from celsius
					temp_f = temperatures[i].fractional *1.8;	// get just the fractional part
					if (temp_f > 999)
					{
						temp++;
						temp_f -= 1000;
					}
					// only 6 bytes of the address are needed.  The first two will always be 28ff for ds18b20s
					sprintf(buf,"%c:[%02d] :%02x%02x%02x%02x%02x%02x:%3d.%02dF\r\n", address,i,
								dev.address[2], dev.address[3],	dev.address[4], dev.address[5], dev.address[6], dev.address[7],
								temp,temp_f/10);
					USART_PutString(buf);
				}
				free(temperatures);
				sprintf(buf,"%c:DONE :\r\n",address);
				USART_PutString(buf);
				GPIO_SetBits(GPIOC,GPIO_Pin_13);		// turn the on-board LED off when done processing
			}
			else  // not my id
			{
				esc = 0;
				pos = 0;
				for (i = 0; i < 10; i++)
					bufferu[i] = 0;
			}
		}
		if (esc && (Global_time - esc_time > 2000))  // give it 2 seconds to receive the id character
		{
			esc = 0;  // reset the esc flag, since it should have been followed by the id right away
			pos = 0;
		}
    }
  // Infinite loop, never return.
}

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
