/*
 * led.h
 *
 *  Created on: Oct 12, 2022
 *      Author: mathi
 */

#ifndef LED_ARRAY_LED_ARRAY_H_
#define LED_ARRAY_LED_ARRAY_H_

#include "stm32l1xx_hal.h"

#define CHECK_LED_PARAMS()     \
	do                         \
	{                          \
		if (led_array == NULL) \
		{                      \
			return HAL_ERROR;  \
		}                      \
	} while (0)

typedef struct
{
	GPIO_TypeDef *port;
	uint16_t pin;
} TypeDef_LED;

typedef struct
{
	TypeDef_LED *array;
	size_t array_sz;
} TypeDef_LED_Array;

HAL_StatusTypeDef led_array_init(TypeDef_LED_Array *_led_array);
HAL_StatusTypeDef write_array(int _led_index, GPIO_PinState _state);
HAL_StatusTypeDef clear_array(void);
HAL_StatusTypeDef set_array(void);

#endif /* LED_ARRAY_LED_ARRAY_H_ */
