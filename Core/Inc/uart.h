/*
 * uart.h
 *
 *  Created on: Nov 19, 2024
 *      Author: dbfir
 */

#ifndef INC_UART_H_
#define INC_UART_H_

#include "main.h"
// 함수 선언
void initUart(UART_HandleTypeDef *inHuart);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
uint8_t getUart();
#endif /* INC_UART_H_ */
