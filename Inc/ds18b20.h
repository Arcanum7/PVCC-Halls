#ifndef __DS18B20_H__
#define __DS18B20_H__

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_i2c.h"
#include "stm32f10x_iwdg.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "one_wire.h"
#include "timer.h"

// Structure in which temperature is stored
typedef struct {
    int8_t integer;
    uint16_t fractional;
    bool is_valid;
} simple_float;

// Structure for returning list of devices on one wire
typedef struct {
    uint8_t size;
    one_wire_device *devices;
} ds18b20_devices;

void ds18b20_init(GPIO_TypeDef *gpio, uint16_t port);
void ds18b20_set_precision(uint8_t precision);
ds18b20_devices ds18b20_get_devices(bool scan);

void ds18b20_convert_temperature_simple(void);
simple_float ds18b20_read_temperature_simple(void);

void ds18b20_convert_temperature_all(void);
simple_float* ds18b20_read_temperature_all(void);

void ds18b20_wait_for_conversion(void);
simple_float ds18b20_decode_temperature(void);

simple_float ds18b20_get_temperature_simple(void);

#endif // __DS18B20_H__
