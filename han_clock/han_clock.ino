/*
	Name:		han_clock.ino
	Created:	2023-12-08 오후 5:40:17
	Author:	gogomaker
*/

#include "src/functions.h"

Adafruit_NeoPixel strip(36, LED_PIN, NEO_GRB + NEO_KHZ800);

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
byte bright = 180;  //240이 최대, 30이 최소, 8단계 제어
byte ledmode = 0; //12개의 색
unsigned long bu_led_w = 0;
bool ledCheck = false;
bool bu_reading[2] = { HIGH, HIGH };  //버튼의 실제 상태를 표현하는 변수
bool bu_state[2] = { HIGH, HIGH };  //체터링을 거른 버튼의 상태를 표현하는 변수
bool last_bu_state[2] = { HIGH, HIGH }; //마지막 버튼 상태
unsigned long LastDebounceTime[2] = {0, 0};
unsigned long bu_t_w, last_bu_t_w = 0;  //시간 버튼이 언제 눌렸는가
unsigned long wait_t, wait_m = 0; //LED깜박일 때 사용
bool timeCheck = false;
bool isblinkH, isblinkM = false;
byte tchange = 0;

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
	if (sec != lastSec) {	//내부시간 동기화하고, 시간 표시하고
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

	changeTimeButton();
	changeLedButton();
	longTimeButton();
	blinkHM();
}