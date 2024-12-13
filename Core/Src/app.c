/*
 * app.c
 *
 *  Created on: Nov 25, 2024
 *      Author: wjdek
 */
#include "app.h"

// 외부 장치 핸들 정의
extern I2C_HandleTypeDef hi2c1;

//전역 변수 선언
uint8_t hour = 12, minute = 0, second = 0;
uint8_t update = 0;

//1ms주기로 호출됨
void SystickCallback(){
	static ksec=0;
	ksec++;
	if(ksec == 1000){
		ksec = 0;
		second++;
		update = 1;
		if (second == 60) {
			second = 0;
			minute++;
			if(minute == 60){
				minute = 0;
				hour++;
				if(hour == 12){
					hour = 0;
				}
			}
		}
	}


}

void app(){
	//장치 초기화
	ssd1306_Init();
	//for analogClock
	const uint8_t centerX = 63;
	const uint8_t centerY = 31;
	const uint8_t lengthS = 29;
	const uint8_t lengthM = 25;
	const uint8_t lengthH = 18;
	ssd1306_DrawCircle(63, 31, 31, 1);
	ssd1306_UpdateScreen();
	while(1){
		//digitClock
//		char str[20];
//		sprintf(str, "%02d:%02d:%02d",hour,minute,second); //3개의 값을 뿌려줌
//		ssd1306_SetCursor(0, 15); //좌표값 설정
//		ssd1306_WriteString(str, Font_16x26, 1); //폰트, 컬러
//		ssd1306_UpdateScreen(); //스크린에 업데이트
		static uint8_t oldXs, oldYs, oldXm, oldYm, oldXh,oldYh;

		// 각도계산 (도 단위)
		uint16_t angleS = -90 + (360/60) * second;
		uint16_t angleM = -90 + (360/60) * minute;
		uint16_t angleH = -90 + (360/12) * hour;

		// 바늘 끝위치 계산
		uint8_t Xs = centerX + cos(angleS * 3.14 / 180) * lengthS;
		uint8_t Ys = centerY + sin(angleS * 3.14 / 180) * lengthS;
		uint8_t Xm = centerX + cos(angleM * 3.14 / 180) * lengthM;
		uint8_t Ym = centerY + sin(angleM * 3.14 / 180) * lengthM;
		uint8_t Xh = centerX + cos(angleH * 3.14 / 180) * lengthH;
		uint8_t Yh = centerY + sin(angleH * 3.14 / 180) * lengthH;
		//이전 그림 지우기
		ssd1306_Line(centerX, centerY, oldXs, oldYs, 0);
		ssd1306_Line(centerX, centerY, oldXm, oldYm, 0);
		ssd1306_Line(centerX, centerY, oldXh, oldYh, 0);
		//새로 그리기
		ssd1306_Line(centerX, centerY, Xs, Ys, 1);
		ssd1306_Line(centerX, centerY, Xm, Ym, 1);
		ssd1306_Line(centerX, centerY, Xh, Yh, 1);
		//좌표 백업
		oldXs = Xs;
		oldYs = Ys;
		oldXm = Xm;
		oldYm = Ym;
		oldXh = Xh;
		oldYh = Yh;
		//스크린에 업데이트
		ssd1306_UpdateScreen();



		update = 0;
		while(update==0);

	}
}
