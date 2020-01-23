/*
 * dwt_stm32f7_delay.h
 *
 *  Created on: 22 de jul de 2018
 *      Author: helde
 */

#ifndef INC_DWT_STM32F7_DELAY_H_
#define INC_DWT_STM32F7_DELAY_H_

#include "stm32f7xx_hal.h"

/**
 * @brief  Initializes DWT_Cycle_Count for DWT_Delay_us function
 * @return Error DWT counter
 *         1: DWT counter Error
 *         0: DWT counter works
 */
uint8_t DWT_Delay_Init(void);

/**
 * @brief  This function provides a delay (in microseconds)
 * @param  microseconds: delay in microseconds
 */
__STATIC_INLINE void DWT_Delay_us(volatile uint32_t microseconds)
{
  uint32_t clk_cycle_start = DWT->CYCCNT;

  /* Go to number of cycles for system */
  microseconds *= (HAL_RCC_GetHCLKFreq() / 1000000);

  /* Delay till end */
  while ((DWT->CYCCNT - clk_cycle_start) < microseconds);
}

#endif /* INC_DWT_STM32F7_DELAY_H_ */
