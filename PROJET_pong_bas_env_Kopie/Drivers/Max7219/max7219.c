/*
 * max7219.c
 *
 *  Created on: Oct 11, 2022
 *      Author: mathi
 */

#include "max7219.h"

#define RESET_MAX7219_PARAMS() \
	do                         \
	{                          \
		max7219_handle = NULL; \
	} while (0)

#define CHECK_MAX7219_PARAMS()      \
	do                              \
	{                               \
		if (max7219_handle == NULL) \
		{                           \
			RESET_MAX7219_PARAMS(); \
			return HAL_ERROR;       \
		}                           \
	} while (0)

/* Static variables used to store MAX7219 related objects */
static MAX7219_Handle_TypeDef *max7219_handle = NULL;

static uint8_t digits_registers[] = {
	DIGIT_0_REG_BASE,
	DIGIT_1_REG_BASE,
	DIGIT_2_REG_BASE,
	DIGIT_3_REG_BASE,
	DIGIT_4_REG_BASE,
	DIGIT_5_REG_BASE,
	DIGIT_6_REG_BASE,
	DIGIT_7_REG_BASE,
};

/*
 * @brief Send data to address
 * @param _address Address on 8 bits
 * @param _data Data on 8 bits
 */
static HAL_StatusTypeDef max7219_transmit(uint8_t _address, uint8_t _data)
{
	uint8_t data[] = {_address, _data};				 // SPI transmit buffer
	size_t data_sz = sizeof(data) / sizeof(uint8_t); // Size of SPI transmit buffer
	HAL_StatusTypeDef max7219_status = HAL_OK;		 // Return value

	// Select MAX7219, send data, de-select MAX7219
	HAL_GPIO_WritePin(max7219_handle->spi_ncs_port, max7219_handle->spi_ncs_pin, GPIO_PIN_RESET);
	max7219_status = HAL_SPI_Transmit(max7219_handle->hspi, data, data_sz, 100);
	HAL_GPIO_WritePin(max7219_handle->spi_ncs_port, max7219_handle->spi_ncs_pin, GPIO_PIN_SET);

	// Return transmit status
	return max7219_status;
}

/**
 * @brief Init function. Pass hardware handles and constants.
 * also initializes basic functions of MAX7219
 * @param _max_7219_handle Pointer to MAX7219 handle
 *
 */
HAL_StatusTypeDef max7219_init(MAX7219_Handle_TypeDef *_max_7219_handle)
{
	/* Attribute parameters to static variables */
	max7219_handle = _max_7219_handle;

	CHECK_MAX7219_PARAMS();

	/* Initialize MAX7219 following datasheet */
	HAL_StatusTypeDef max7219_status = HAL_OK;

	// Shutdown MAX7219 to reset configuration
	max7219_status = max7219_transmit(SHUTDOWN_REG_BASE, SHUTDOWN_REG_SHUTDOWN_MODE);
	if (max7219_status != HAL_OK)
		return max7219_status;

	// Enable MAX7219
	max7219_status = max7219_transmit(SHUTDOWN_REG_BASE, SHUTDOWN_REG_NORMAL_MODE);
	if (max7219_status != HAL_OK)
		return max7219_status;

	// Set scan limit to number of digits
	max7219_status = max7219_transmit(SCAN_LIMIT_REGG_BASE, max7219_handle->digits_count - 1);
	if (max7219_status != HAL_OK)
		return max7219_status;

	// Set decode mode to 'no decode'
	max7219_status = max7219_transmit(DECODE_MODE_REG_BASE, 0x00);
	if (max7219_status != HAL_OK)
		return max7219_status;

	// Set brightness to middle value
	max7219_status = max7219_transmit(INTENSITY_REG_BASE, 0x08);
	if (max7219_status != HAL_OK)
		return max7219_status;

	// Erase all digits
	max7219_status = max7219_erase_no_decode();
	if (max7219_status != HAL_OK)
		return max7219_status;

	// Return on success
	return HAL_OK;
}

/**
 * @brief Display value without code B decoding.
 * @param _digit_index 7 segment digit index (starts at 0)
 * @param _digit_value Desired digit value to be written
 * @retval HAL_OK on success
 */
HAL_StatusTypeDef max7219_display_no_decode(uint8_t _digit_index, uint8_t _digit_value){
	HAL_StatusTypeDef max7219_status = HAL_OK;

	// Check if init has been called
	CHECK_MAX7219_PARAMS();

	// Set decode mode to 'no decode'
	max7219_status = max7219_transmit(DECODE_MODE_REG_BASE, 0x00);
	if (max7219_status != HAL_OK)
		return max7219_status;

	/* Check if digit index does not overflow actual hardware setup */
	if (_digit_index > max7219_handle->digits_count)
		return HAL_ERROR;

	// Display value
	max7219_status = max7219_transmit(digits_registers[_digit_index], _digit_value);
	if (max7219_status != HAL_OK)
		return max7219_status;

	return max7219_status;
}

/**
 * @brief Display value with code B decoding.
 * @param _digit_index 7 segment digit index (starts at 0)
 * @param _digit_value Desired digit value to be written
 * @retval HAL_OK on success
 */
HAL_StatusTypeDef max7219_display_decode(uint8_t _digit_index, uint8_t _digit_value){
	HAL_StatusTypeDef max7219_status = HAL_OK;

	// Check if init has been called
	CHECK_MAX7219_PARAMS();

	/* Check if digit index does not overflow actual hardware setup */
	if (_digit_index > max7219_handle->digits_count)
		return HAL_ERROR;

	// Set decode mode to 'decode'
	max7219_status = max7219_transmit(DECODE_MODE_REG_BASE, 0xFF);
	if (max7219_status != HAL_OK)
		return max7219_status;

	// Display value
	max7219_status = max7219_transmit(digits_registers[_digit_index], _digit_value);
	if (max7219_status != HAL_OK)
		return max7219_status;

	return max7219_status;
}

/**
 * @brief Erase display
 * @retval HAL_OK on success
 */
HAL_StatusTypeDef max7219_erase_no_decode(void)
{
	HAL_StatusTypeDef max7219_status = HAL_OK;

	// Check if init has been called
	CHECK_MAX7219_PARAMS();

	// Set decode mode to 'no decode'
	max7219_status = max7219_transmit(DECODE_MODE_REG_BASE, 0x00);
	if (max7219_status != HAL_OK)
		return max7219_status;

	for (int i = 0; i < max7219_handle->digits_count; i++)
	{
		max7219_status = max7219_transmit(digits_registers[i], DIGIT_OFF);
		if (max7219_status != HAL_OK)
			return max7219_status;
	}

	return max7219_status;
}

/**
 * @brief Erase display
 * @retval HAL_OK on success
 */
HAL_StatusTypeDef max7219_erase_decode(void)
{
	HAL_StatusTypeDef max7219_status = HAL_OK;

	// Check if init has been called
	CHECK_MAX7219_PARAMS();

	// Set decode mode to 'decode'
	max7219_status = max7219_transmit(DECODE_MODE_REG_BASE, 0xFF);
	if (max7219_status != HAL_OK)
		return max7219_status;

	for (int i = 0; i < max7219_handle->digits_count; i++)
	{
		max7219_status = max7219_transmit(digits_registers[i], DIGIT_OFF_DECODE);
		if (max7219_status != HAL_OK)
			return max7219_status;
	}

	return max7219_status;
}
