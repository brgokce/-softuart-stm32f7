/*
 * softuart.h
 *
 *  Created on: 28 de jul de 2018
 *      Author: Helder Sales
 */

#ifndef INC_SOFTUART_H_
#define INC_SOFTUART_H_

#include "stdbool.h"
#include "string.h"
#include "dwt_stm32f7_delay.h"
#include "main.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/*
 * D0 (PG09) -> RX
 * D1 (PG14) -> TX
 */

#define TX	true
#define	RX	false

void UARTSend(char *text, uint32_t baud);
char UARTReceiveChar(uint32_t baud);
void UARTReceiveString(char *charBuf, uint32_t stringLen, uint32_t baud);

#endif /* INC_SOFTUART_H_ */
