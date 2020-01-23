/*
 * dwt_stm32f7_delay.c
 *
 *  Created on: 22 de jul de 2018
 *      Author: helde
 */

#include "dwt_stm32f7_delay.h"

volatile uint32_t *DWT_CONTROL = (uint32_t *) 0xE0001000;
volatile uint32_t *DWT_CYCCNT = (uint32_t *) 0xE0001004;
volatile uint32_t *DEMCR = (uint32_t *) 0xE000EDFC;
volatile uint32_t *LAR  = (uint32_t *) 0xE0001FB0;   // <-- added lock access register


/**
 * @brief  Initializes DWT_Clock_Cycle_Count for DWT_Delay_us function
 * @return Error DWT counter
 *         1: clock cycle counter not started
 *         0: clock cycle counter works
 */
uint8_t DWT_Delay_Init(void)
{
	*DEMCR = *DEMCR | 0x01000000;     // enable trace
	*LAR = 0xC5ACCE55;                // <-- added unlock access to DWT (ITM, etc.)registers
	*DWT_CYCCNT = 0;                  // clear DWT cycle counter
	*DWT_CONTROL = *DWT_CONTROL | 1;  // enable DWT cycle counter

	/* Disable TRC */
	CoreDebug->DEMCR &= ~CoreDebug_DEMCR_TRCENA_Msk; // ~0x01000000;
	/* Enable TRC */
	CoreDebug->DEMCR |=  CoreDebug_DEMCR_TRCENA_Msk; // 0x01000000;

	/* Disable clock cycle counter */
	DWT->CTRL &= ~DWT_CTRL_CYCCNTENA_Msk; //~0x00000001;
	/* Enable  clock cycle counter */
	DWT->CTRL |=  DWT_CTRL_CYCCNTENA_Msk; //0x00000001;

	/* Reset the clock cycle counter value */
	DWT->CYCCNT = 0;

	/* 3 NO OPERATION instructions */
	asm volatile ("NOP");
	asm volatile ("NOP");
	asm volatile ("NOP");

	/* Check if clock cycle counter has started */
	if(DWT->CYCCNT)
		return 0; /*clock cycle counter started*/

	else
		return 1; /*clock cycle counter not started*/
}
