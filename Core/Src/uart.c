/*
 * uart.c
 *
 *  Created on: Nov 19, 2024
 *      Author: dbfir
 */

#include "uart.h"
#include <stdio.h>

UART_HandleTypeDef *myHuart;   //uart의 핸들

//Ring buffer cicular buffer

uint8_t rxChar; 							 //수신문자
#define rxBufferMax 255				 //수신버퍼의 최대크기
int rxBufferWrite; 					   //수신버퍼 쓰기 포인터
int rxBufferRead;						   //수신버퍼 읽기 포인터
uint8_t rxBuffer[rxBufferMax]; //수신버퍼

int _write(int file, char* p, int len){
	HAL_UART_Transmit(myHuart, p, len, 10);
	return len;
}

// uart장치의 초기화 함수 구현
void initUart(UART_HandleTypeDef *inHuart) {
	myHuart = inHuart;
	//수신인터럽트 설정
	HAL_UART_Receive_IT(myHuart, &rxChar, 1);
	rxBufferRead = rxBufferWrite = 0;
}

//문자수신 처리 함수 구현
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	// 수신딘 문자를 버퍼에 저장하고 쓰기포인터의 값을 1증가
	rxBuffer[rxBufferWrite++] = rxChar;
	// 쓰기포인터의 값이 최대치에 도달하면 다시 0으로 초기화
	rxBufferWrite %= rxBufferMax;
	// 다음 문자 수신 인터럽트를 위하여 재 설정
			HAL_UART_Receive_IT(myHuart, &rxChar, 1);
}

// 버퍼에서 문자 꺼내오기
uint8_t getUart(){
	uint8_t result;
	// 수신된 문자 없음
	if(rxBufferWrite == rxBufferRead) return 0;
	//읽기 포인터가 가르키는 위치의 버퍼문자를 꺼내고, 읽기포인터를 1 증가
	result = rxBuffer[rxBufferRead++];
	//읽기포인터가 최대치에 도달하면 다시 0으로 초기화
	rxBufferRead %= rxBufferMax;
	return result;
}
