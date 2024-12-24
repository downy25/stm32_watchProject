/*
 * app.c
 *
 *  Created on: Nov 25, 2024
 *      Author: wjdek
 */
#include "app.h"

// 외부 장치 핸들 정의
extern I2C_HandleTypeDef hi2c1; //OLCD handle
extern TIM_HandleTypeDef htim3; //rotary handle
extern UART_HandleTypeDef huart2;
extern ADC_HandleTypeDef hadc1;
const float temperatures[] = {
    -20,  -19,  -18,  -17,  -16,  -15,  -14,  -13,  -12,  -11,  -10,  -9,  -8,  -7,
    -6,  -5,  -4,  -3,  -2,  -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10,  11,
    12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,
    28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,
    44,  45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,
    60,  61,  62,  63,  64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,
    76,  77,  78,  79,  80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,
    92,  93,  94,  95,  96,  97,  98,  99,  100
}; // 온도 샘플
const float resistances[] = {
    97839.6,  92302,  87112.4,  82247.1,  77683.7,  73401.8,  69382.3,  65607.7,  62061.6,
    58728.8,  55595.3,  52648,  49874.7,  47264.3,  44806.2,  42490.6,  40308.6,  38251.6,
    36311.7,  34481.7,  32754.7,  31124.3,  29584.7,  28130.1,  26755.6,  25456.2,  24227.4,
    23065,  21965,  20923.9,  19938,  190041,  18119.3,  17280.7,  16485.7,  15731.7,
    15016.4,  14337.6,  13693.3,  13081.6,  12500.5,  11948.5,  11423.9,  10925.2,  10451,
    10000,  9570.9,  9162.6,  8773.8,  8403.7,  8051.2,  7715.4,  7395.3,  7090.3,  6799.5,  6522.1,
    6257.6,  6005.1,  5764.2,  5534.2,  5314.6,  5104.9,  4904.5,  4713,  4530,  4355.1,  4187.8,
    4027.8,  3874.8,  3728.3,  3588.2,  3454,  3325.5,  3202.5,  3084.6,  2971.7,  2863.5,  2759.7,
    2660.3,  2564.9,  2473.4,  2385.6,  2301.4,  2220.6,  2143.1,  2068.6,  1997,  1928.3,  1862.3,
    1798.9,  1738,  1679.4,  1623.1,  1568.9,  1516.8,  1466.7,  1418.5,  1372.2,  1327.5,  1284.5,
    1243.1,  1203.3,  1164.9,  1127.9,  1092.3,  1058,  1024.9,  993,  962.3,  932.6,  904,
    876.4,  849.8,  824.1,  799.4,  775.4,  752.3,  730,  708.5,  687.7,  667.6
}; // 해당 온도에서의 저항값 샘플
const int tableSize = sizeof(temperatures) / sizeof(temperatures[0]);

float lookupTemperature(uint16_t inValue){
	float Vcc = 3.3; //Vcc 전원 전압
	float R1 = 10000; //Pull up 저항값
	float Vo; //thermistor 양단의 전압값
	float R2; //thermistor 저항값
	float temperature = 0.0;

	//ADC 값으로 부터 전압을 계산 (12Bit의 ADC인 경우)
	Vo = ((float)inValue*Vcc)/4095.0;

	// themistor의 저항값 계산
	R2 = R1 / ((Vcc / Vo)-1.0);

	// 저항 테이블에서 R2에 해당하는 위치를 찾음
	for(int i=0; i<tableSize-1; i++){
		if(R2<=resistances[i]&& R2 > resistances[i+1]){
			// 기울기 계산
			float slope = (temperatures[i+1]-temperatures[i]) / (resistances[i+1]-resistances[i]);
			// 계산된 기울기 만큼 보간
			temperature = temperatures[i] + slope * (R2-resistances[i]);
			break;
		}
	}
	return temperature;
}

uint8_t hour = 12, minute = 0, second = 0;
uint8_t update = 0;
uint32_t time = 0;
uint16_t delay = 0;

//1ms주기로 호출됨(시간측정 콜백함수)
void Systick(){
	time++;
}

// 1ms의 주기로 호출됨
void SystickCallbackDelay(){
	if(delay > 0) delay--;
}

//1ms주기로 호출됨
void SystickCallbackClock(){
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
				if(hour == 24){
					hour = 0;
				}
			}
		}
	}
}

void AnalogClock(){
	//for analogClock
	const uint8_t centerX = 63;
	const uint8_t centerY = 31;
	const uint8_t lengthS = 29;
	const uint8_t lengthM = 25;
	const uint8_t lengthH = 18;
	ssd1306_DrawCircle(63, 31, 31, 1);
	ssd1306_UpdateScreen();
	static uint8_t oldXs, oldYs, oldXm, oldYm, oldXh, oldYh;

	// 각도계산 (도 단위)
	uint16_t angleS = -90 + (360 / 60) * second;
	uint16_t angleM = -90 + (360 / 60) * minute;
	uint16_t angleH = -90 + (360 / 12) * hour;

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
	while (update == 0);
}
void DigitClock(){
	//digitClock
	char str[20];
	sprintf(str, "%02d:%02d:%02d", hour, minute, second); //3개의 값을 뿌려줌
	ssd1306_SetCursor(0, 15); //좌표값 설정
	ssd1306_WriteString(str, Font_16x26, 1); //폰트, 컬러
	ssd1306_UpdateScreen(); //스크린에 업데이트
}
void DisplayStr(){
	//display string
	char str[20];
	strcpy(str,"User Mode!!");
	ssd1306_SetCursor(5, 20); //좌표값 설정
	ssd1306_WriteString(str, Font_11x18, 1); //폰트, 컬러
	ssd1306_UpdateScreen(); //스크린에 업데이트
}
void Displaytemp(){
	char str[20];
	strcpy(str,"20C'");
	ssd1306_SetCursor(20, 20); //좌표값 설정
	ssd1306_WriteString(str, Font_11x18, 1); //폰트, 컬러
	ssd1306_UpdateScreen(); //스크린에 업데이트
}
void DisplayFillBlack(){
	ssd1306_Fill(0x00); //이전화면을 지우고 새로쓰기
	ssd1306_UpdateScreen(); //스크린에 업데이트
}


void app(){
	// uart 장치 초기화
	initUart(&huart2);
	ssd1306_Init();
	// timer 장치 초기화
	HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);
	//변수 초기화
	static uint8_t OldSwState = 0;
	static uint8_t OldEnState = 0;
	static uint16_t StartTime = 0;
	static uint16_t EndTime = 0;
	static uint8_t counterState = 0;
	static uint8_t initMode=0;
	static uint32_t countSw=0;
	static uint8_t countEn=0;
	static uint8_t timeState=0;
	uint8_t displayOn = 0;

	while(1){
		uint8_t EnState=(int)htim3.Instance->CNT;
		uint8_t SwState=HAL_GPIO_ReadPin(SW_GPIO_Port, SW_Pin);

		////시간 측정구간
		if((OldSwState!=SwState)&&(SwState==0)){
			//하강엣지 일때 엣지의 시작시간 저장 후 카운팅 시작시그널이 바뀌고 count 값이 증가
			countSw++;
			StartTime = time;
			counterState = 1;
		}//상승엣지일때 시작,끝시간,카운팅 시작시그널을 0으로 리셋
		else if((OldSwState!=SwState)&&(SwState==1)){
			StartTime=0;
			EndTime=0;
			counterState=0;
			//버튼을 떼는 그순간에 timeState가 4이면 다시 초만 변경하도록 바꿈
			if(timeState==4)timeState=0;
		}

		//counting이 시작되었다고 판단되면
		if (counterState == 1) {
			//예전상태와 이전상태가 같고 현제상태가 == 0 이면 시간을 계속업데이트하면서 끝나는 시간을 기록
			if ((OldSwState == SwState) && (SwState == 0))EndTime = time;
		}

		//init모드 활성,비활성화 구간
		//엔코더 상태가 0이고 측정시간이 3초가 넘어가고
		if ((EndTime - StartTime) >= 3000) {
			if (countSw % 2 == 0) initMode = 0; //짝수이면 initMode 비활성화
			else initMode = 1; //홀수이면 initMode 활성화
		}

		//init모드에 관한 설정
		if (initMode) {
			if ((EndTime - StartTime) > 3000 && (EndTime - StartTime) <= 5000) timeState = 1; //초만 변경가능
			else if ((EndTime - StartTime) > 5000 && (EndTime - StartTime) <= 7000)timeState = 2; //분만 변경가능
			else if ((EndTime - StartTime) > 7000 )timeState = 3; //시만 변경가능

			//500ms주기로 초기화하려는 부분이 켜졌다 꺼졌다 하는부분
			if (delay == 0) {
				//500ms마다 한번씩 들어옴
				delay = 500;
				if (displayOn == 0){
					switch (timeState) {
						case 1:
							ssd1306_FillSec(0x00);
							break;
						case 2:
							ssd1306_FillMin(0x00);
							break;
						case 3:
							ssd1306_FillHour(0x00);
							break;
						default: {
							ssd1306_Fill(0x00);
							ssd1306_UpdateScreen();
							break;
						}
					}
				}
				else if (displayOn == 1) DigitClock(); //digital clock 띄우기
				displayOn ^= 1; //번갈아가면서 실행 1초 주기로
			}

			if (OldEnState != EnState){//엔코더의 이전값과 현재값이 다르면
				switch (timeState){
					case 1:
						second++;
						if(second==60)second=0;
						break;
					case 2:
						minute++;
						if(minute==60)minute=0;
						break;
					case 3:
						hour++;
						if(hour==24)hour=0;
						break;
					default:
						break;
				}
			}
			OldEnState = EnState; //이전값을 저장
		}
		else{ //init모드가 아닐때는 슬라이드 기능
			ssd1306_Fill(0x00);
			switch (EnState) {
				case 0:
					DigitClock();
					break;
				case 1:
					AnalogClock();
					break;
				case 2:
					DisplayStr();
					userMode();
					break;
			}
		}
		OldSwState = SwState;
	}
}
void userMode(){
	static char buffer[256];
	static uint16_t count = 0;

	char ch = getUart();
	if ((ch != '\0') && (ch != '\n')) {
		buffer[count] = ch;
		count++;
	}

	if (ch == '\n') {
		buffer[count] = '\0';
		printf("%s\n", buffer);
		ssd1306_Fill(0x00);
		if(strncmp(buffer,"temp",4)==0){
			Displaytemp();
		}
		buffer[0] = '\0';
		count = 0;
	}
}
