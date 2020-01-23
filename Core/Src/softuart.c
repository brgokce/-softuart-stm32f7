/*
 * softuart.c
 *
 *  Created on: 28 de jul de 2018
 *      Author: Helder Sales
 */

#include "softuart.h"

volatile uint8_t g_textCharTX[32];
volatile uint8_t g_ascii[8];
volatile uint8_t g_asciiBitsTX[32][8];
volatile uint8_t g_count;
volatile uint8_t g_pos;
volatile uint32_t g_startSamplingDelay;

volatile bool g_setMode;
volatile bool g_startBit;
volatile bool g_isCharReceived = true;

uint8_t g_receivedChar;

void UARTSend(char *text, uint32_t baud)
{
	// Select TX mode (for timer purposes)
	g_setMode = TX;

	// Store the current clock value
	uint32_t systemClock = HAL_RCC_GetSysClockFreq();

	// Config timer to run at specified baud rate
	__HAL_TIM_SET_AUTORELOAD(&htim10, systemClock / baud);

	uint8_t numberOfChars = 0;

	// Separate chars inside an array
	for(uint8_t i = 0; *text != '\0'; text++, i++)
	{
		g_textCharTX[i] = *text;

		numberOfChars++;
	}

	// Separate the character bits into a matrix where the lines are
	// the actual characters and the columns are the character bits
	for(uint8_t i = 0; i < numberOfChars; i++)
		for(uint8_t j = 0; j < 8; j++)
			g_asciiBitsTX[i][7 - j] = g_textCharTX[i] >> j & 0x01;

	// Send the string
	for(g_pos = 0; g_pos < numberOfChars; g_pos++)
	{
		g_startBit = true;
		g_count = 0;

		HAL_TIM_Base_Start_IT(&htim10);

		// Wait until 8 bits of data are sent
		while(g_count <= 8);
	}
}

char UARTReceiveChar(uint32_t baud)
{
	// Select RX mode (for timer purposes)
	g_setMode = RX;

	// Delay for getting the bit's half position
	g_startSamplingDelay = (1 / (2.0 * baud)) * 1000000;

	// Store the current clock value
	uint32_t systemClock = HAL_RCC_GetSysClockFreq();

	// Config timer to run at specified baud rate
	__HAL_TIM_SET_AUTORELOAD(&htim10, systemClock / baud);

	// Clear and enable interrupt to receive start bit
	__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_9);
	HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

	g_isCharReceived = false;
	g_count = 0;

	while(!g_isCharReceived);

	g_receivedChar = 0;

	// Make a char with received bits
	for (uint8_t i = 0; i < 8; i++)
		g_receivedChar |= g_ascii[i] << (7 - i);

	return g_receivedChar;
}

void UARTReceiveString(char *charBuf, uint32_t stringLen, uint32_t baud)
{
	// Select RX mode (for timer purposes)
	g_setMode = RX;

	// Delay for getting the bit's half
	g_startSamplingDelay = (1 / (2.0 * baud)) * 1000000;

	// Store the current clock value
	uint32_t systemClock = HAL_RCC_GetSysClockFreq();

	// Config timer to run at specified baud rate
	__HAL_TIM_SET_AUTORELOAD(&htim10, systemClock / baud);

	// Clear buffer to receive data
	memset(charBuf, 0, stringLen);

	// Make a string, placing the characters inside the pointer to char array
	for(uint8_t i = 0; i <= stringLen; i++)
	{
		// Clear and enable interrupt to receive start bit
		__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_9);
		HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

		g_isCharReceived = false;
		g_count = 0;

		while(!g_isCharReceived);

		// Make a char with received bits
		for(uint8_t j = 0; j < 8; j++)
			*charBuf |= g_ascii[j] << (7 - j);

		if(*charBuf == '\n')
			break;

		// Increments pointer position to receive next char
		charBuf++;
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	// Small delay before activating timer to get the equivalent time of
	// half of the position of receiveing bit
	DWT_Delay_us(g_startSamplingDelay);

	// Activate timer to get 8 bit data
	HAL_TIM_Base_Start_IT(&htim10);

	//desativa o interrupt pois os bits a serem recebidos ser�o
	//interpretados no timer, n�o aqui
	HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	// Timer signaled to receive data
	if(g_setMode == RX)
	{
		//Read the receiveing bits (bit0, bit1, ..., bit7)
		g_ascii[7 - g_count] = HAL_GPIO_ReadPin(GPIOG, GPIO_PIN_9);

		// If 8th iteraction, then 8 bits was received
		// Deactivate timer and signals char received
		if(g_count == 7)
		{
			HAL_TIM_Base_Stop_IT(&htim10);
			g_isCharReceived = true;
			return;
		}

		// Increments the number of received bits
		g_count++;
	}

	// Timer signaled to transmit data
	else if(g_setMode == TX)
	{
		// Send start bit
		if(g_startBit)
		{
			HAL_GPIO_WritePin(GPIOG, GPIO_PIN_14, GPIO_PIN_RESET);

			g_startBit = false;

			return;
		}

		// If transmitted 8 bits of data, send stop bit
		// Stop timer and clear counter
		if(g_count == 8)
		{
			HAL_GPIO_WritePin(GPIOG, GPIO_PIN_14, GPIO_PIN_SET);

			HAL_TIM_Base_Stop_IT(&htim10);
			TIM10->CNT = 0;

			g_count++;

			return;
		}

		// Send the bit
		HAL_GPIO_WritePin(GPIOG, GPIO_PIN_14,
				g_asciiBitsTX[g_pos][7 - g_count]);

		// Increments the numer that represents the sent bits
		g_count++;
	}
}
