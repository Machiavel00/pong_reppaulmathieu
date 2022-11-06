/*
 * led.c
 *
 *  Created on: Oct 12, 2022
 *      Author: mathi
 */

#include "led_array.h"

static TypeDef_LED_Array *led_array = NULL;

/*
 * @brief Initialize LED array from parameters
 * @param _led_array Sructure containing LED array and array size
 * @retval HAL status
 */
HAL_StatusTypeDef led_array_init(TypeDef_LED_Array *_led_array)
{
	// Attribute led param
	led_array = _led_array;

	CHECK_LED_PARAMS();

	return HAL_OK;
}

/**
 * @brief Write pin state to LED array
 * @param _led_index LED index to write pin state (starts at 0)
 * @param _state Pin state, value can be :
 * 		- GPIO_PIN_SET
 * 		- GPIO_PIN_RESET
 * @retval HAL status
 */
HAL_StatusTypeDef write_array(int _led_index, GPIO_PinState _state)
{
	CHECK_LED_PARAMS();

	// Check led index
	if ((_led_index < 0) || (_led_index >= led_array->array_sz))
		return HAL_ERROR;

	// Write pin state to led index
	HAL_GPIO_WritePin(led_array->array[_led_index].port, led_array->array[_led_index].pin, _state);
	return HAL_OK;
}

HAL_StatusTypeDef clear_array(void)
{
	CHECK_LED_PARAMS();

	// Clear LED array
	for (int i = 0; i < led_array->array_sz; i++)
	{
		write_array(i, GPIO_PIN_RESET);
	}

	return HAL_OK;
}

HAL_StatusTypeDef set_array(void)
{
	CHECK_LED_PARAMS();

	// Set LED array
	for (int i = 0; i < led_array->array_sz; i++)
	{
		write_array(i, GPIO_PIN_SET);
	}

	return HAL_OK;
}
