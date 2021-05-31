#line 1 "d:\\.PROJECT\\HangeulClock_DIY_v1\\Programming\\han_clock\\han_clock.ino"

/*
 Name:		han_clock.ino
 Created:	2020-08-25 오후 5:32:32
 Author:	daseu
 
 Ver.2 - 모든 기능 구현...
*/

//헤더파일 선언
#include <Arduino.h>

#include "src/functions.h"

//실제 변수 선언
byte color[12][3]
{
  {255, 255, 255},
  {255, 0, 0},
  {255, 94, 0},
  {255, 187, 0},
  {255, 228, 0},
  {171, 242, 0},
  {29, 219, 22},
  {0, 84, 255},
  {1, 0, 255},
  {95, 0, 255},
  {255, 0, 221},
  {255, 0, 100}
};

Adafruit_NeoPixel strip(36, 6, NEO_GRB + NEO_KHZ800);

extern volatile unsigned long timer0_millis; //millis 오버플로우 대비 초기화
unsigned long time = 0;
byte sec, lastSec = 0;
byte hourPlus, minPlus = 0; //버튼이 눌릴 때 이곳에 값이 임시저장 됨
byte minRtc, hourRtc = 0; //RTC의 시간값이 저장됨
byte min, hour = 0;   //최종적으로 출력되는 시간값이 저장됨
bool isResetMillis = false;
byte tMSB, tLSB;  //RTC온도관련
float temp3231;
byte r = 255; //네오픽셀 Red
byte g = 255; //네오픽셀 Green
byte b = 255; //네오픽셀 Blue
byte bright = 180;  //255가 최대, 0이 최소, 7단계 제어
byte ledmode = 0; //12개의 색
long randomBase = 0;
unsigned long bu_led_w = 0;
bool ledCheck = false;
bool bu_reading[2] = { HIGH, HIGH };  //버튼의 실제 상태를 표현하는 변수
bool bu_state[2] = { HIGH, HIGH };  //체터링을 거른 버튼의 상태를 표현하는 변수
bool last_bu_state[2] = { HIGH, HIGH }; //마지막 버튼 상태
unsigned long LastDebounceTime[2] = { 0, 0};
unsigned long bu_t_w, last_bu_t_w = 0;  //시간 버튼이 언제 눌렸는가
unsigned long wait_t, wait_m = 0; //LED깜박일 때 사용
bool timeCheck = false;
bool isblinkH, isblinkM = false;
byte tchange = 0;


#line 62 "d:\\.PROJECT\\HangeulClock_DIY_v1\\Programming\\han_clock\\han_clock.ino"
void setup();
#line 86 "d:\\.PROJECT\\HangeulClock_DIY_v1\\Programming\\han_clock\\han_clock.ino"
void loop();
#line 62 "d:\\.PROJECT\\HangeulClock_DIY_v1\\Programming\\han_clock\\han_clock.ino"
void setup() {
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
	clock_prescale_set(clock_div_1);
#endif
	Wire.begin();
	//시리얼 모니터
	Serial.begin(115200);
	Serial.println("Hangeul Clock has turn ON");
	Serial.println("Clock start");
	//네오픽셀 초기설정
	strip.begin();
	strip.show();
	strip.setBrightness(bright);
	//버튼입력설정
	pinMode(BUTTON_T, INPUT_PULLUP);
	pinMode(BUTTON_LED, INPUT_PULLUP);
	//시간출력
	time = millis();
	get3231Date();
	hour = (hourRtc + hourPlus) % 24;
	min = (minRtc + minPlus) % 60;
	displayTime(hour, min);
}

void loop() {
	time = millis();
	get3231Date();
	if (!sec && !minRtc && !hourRtc) {	//millis 초기화
		if(!timer0_millis) isResetMillis = true;
		if (isResetMillis == true) {
			//Serial.println("Timer reset");
			bu_led_w = 0;
			LastDebounceTime[0] = 0;
			LastDebounceTime[1] = 0;
			bu_t_w = 0;
			last_bu_t_w = 0;
			wait_t = 0;
			wait_m = 0;
			time = 0;
			timer0_millis = 0;
			isResetMillis = false;
		}
	}
	//매 초마다
	if (sec != lastSec) {
		hour = (hourRtc + hourPlus) % 24;
		min = (minRtc + minPlus) % 60;
		if (!sec) {	//매 0초마다(1분 간격으로)
			if (hourPlus || minPlus) {
				//Serial.println("RTC set");
				set3231Date();
			}
			displayTime(hour, min);
			//Serial.println("updated");
		}
		lastSec = sec;
	}

	//버튼 눌림 감지 함수
	changeTimeButton();
	changeLedButton();
	//버튼 길게 눌림 감지 함수
	longTimeButton();
	longLedButton();
	//시간 변경 시 LED깜박임
	blinkHM();
  
}

