#include "functions.h"

void displayTime(int h, int m) {
	ledclear();
	if (!((h == 0 || h == 12) && m == 0)) {
		updateHour(h);
		updateMin(m);
	}
	else {
		if (h == 0) {
			printled(17);
			printled(6);
		}
		else {
			printled(6);
			printled(5);
		}
	}
	strip.show();
}

int updateHour(int h) {
	printled(23);
	if (h == 0 || h == 12) { printled(20); printled(22); }
	else if (h == 1 || h == 13) { printled(30); }
	else if (h == 2 || h == 14) { printled(31); }
	else if (h == 3 || h == 15) { printled(32); }
	else if (h == 4 || h == 16) { printled(33); }
	else if (h == 5 || h == 17) { printled(34); printled(35); }
	else if (h == 6 || h == 18) { printled(29); printled(28); }
	else if (h == 7 || h == 19) { printled(27); printled(26); }
	else if (h == 8 || h == 20) { printled(25); printled(24); }
	else if (h == 9 || h == 21) { printled(18); printled(19); }
	else if (h == 10 || h == 22) printled(20);
	else if (h == 11 || h == 23) { printled(20); printled(21); }
}

int updateMin(int m) {
	if(m) printled(0);
	int ten = m / 10;
	if      (ten == 1) { printled(12); printled(0); }
	else if (ten == 2) { printled(12); printled(16); }
	else if (ten == 3) { printled(12); printled(15); }
	else if (ten == 4) { printled(12); printled(14); }
	else if (ten == 5) { printled(12); printled(13); }

	m -= ten * 10;
	if      (m == 1) printled(7);
	else if (m == 2) printled(8);
	else if (m == 3) printled(9);
	else if (m == 4) printled(10);
	else if (m == 5) printled(4);
	else if (m == 6) printled(11);
	else if (m == 7) printled(3);
	else if (m == 8) printled(2);
	else if (m == 9) printled(1);
}

void printled(int n) {
	if (ledmode == 0) {
		randomBase++;
		randomSeed(randomBase);
		byte rand = random(0, 12);
		r = color[rand][0];
		g = color[rand][1];
		b = color[rand][2];
	}
	strip.setPixelColor(n, r, g, b);
}

void ledclear() {
	for (int i = 0; i < strip.numPixels(); i++) {
		strip.setPixelColor(i, 0, 0, 0);
	}
	strip.show();
}

void changeTimeButton() {
	int reading = digitalRead(2);
	if (reading != last_bu_state[0]) {
		LastDebounceTime[0] = time;
	}
	if ((time - LastDebounceTime[0]) > debounceDelay) {
		if (reading != bu_state[0]) {
			bu_state[0] = reading;
			if (bu_state[0] == LOW) {
				//Serial.println("Time_pressed");
				bu_t_w = time;
				timeCheck = true;
			}
			else {
				if (time < bu_t_w + bu_interval) {
					if (tchange == 1) {
						//Serial.println("hour plus");
						hourPlus++;
						if (hourPlus > 23) hourPlus = 0;
						hour = (hourRtc + hourPlus) % 24;
						displayTime(hour, min);
					}
					else if(tchange == 2) {
						//Serial.println("min plus");
						minPlus++;
						if (minPlus > 59) minPlus = 0;
						min = (minRtc + minPlus) % 60;
						displayTime(hour, min);
					}
					timeCheck = false;
				}
			}
		}
	}
	last_bu_state[0] = reading;
}

void changeLedButton() {
	int reading = digitalRead(3);
	if (reading != last_bu_state[1]) {
		LastDebounceTime[1] = time;
	}
	if ((time - LastDebounceTime[1]) > debounceDelay) {
		if (reading != bu_state[1]) {
			bu_state[1] = reading;
			if (bu_state[1] == LOW) {
				//Serial.println("LED_pressed");
				bu_led_w = time;
				ledCheck = true;
			}
			else {	//�����ٸ�
				if (time < bu_led_w + bu_interval) {
					bright += 30;
					if (bright > 210) bright = 30;
					strip.setBrightness(bright);
					displayTime(hour, min);
					//Serial.print("change brightness: ");
					//Serial.println(bright);
					ledCheck = false;
				}
			}
		}
	}
	last_bu_state[1] = reading;
}

void longTimeButton() {
	if (timeCheck == true) {
		if ((time - bu_t_w) >= bu_interval) {
			tchange += 1;
			if (tchange > 2) tchange = 0;
			if (tchange == 1) {
				//Serial.println("hour off");
				wait_t = time;
				if ((hour == 0 || hour == 12) && min == 0) {
					printled(23);
					strip.show();
				}
				else {
					strip.setPixelColor(23, 0, 0, 0);
					strip.show();
				}
				isblinkH = true;
			}
			else if (tchange == 2) {
				//Serial.println("min off");
				wait_m = time;
				if (min == 0) {
					printled(0);
					strip.show();
				}
				else {
					strip.setPixelColor(0, 0, 0, 0);
					strip.show();
				}
				isblinkM = true;
			}
			timeCheck = false;
			//Serial.print("mode change: ");
			//Serial.println(tchange);
		}
	}
}

void longLedButton() {
	if (ledCheck == true) {
		if ((time - bu_led_w) >= bu_interval) {
			//Serial.println("change color");
			ledmode++;
			if (ledmode > 12) ledmode = 0;
			if (ledmode > 0) {
				r = color[ledmode][0];
				g = color[ledmode][1];
				b = color[ledmode][2];
			}
			displayTime(hour, min);
			ledCheck = false;
		}
	}
}

byte decToBcd(byte val) {
	return ((val / 10 * 16) + (val % 10));
}

void set3231Date() {
	//year = (byte)((Serial.read() - 48) * 10 + (Serial.read() - 48));
	//month = (byte)((Serial.read() - 48) * 10 + (Serial.read() - 48));
	//date = (byte)((Serial.read() - 48) * 10 + (Serial.read() - 48));
	//hour = (byte)((Serial.read() - 48) * 10 + (Serial.read() - 48));
	//min = (byte)((Serial.read() - 48) * 10 + (Serial.read() - 48));
	//sec = (byte)((Serial.read() - 48) * 10 + (Serial.read() - 48));
	//day = (byte)(Serial.read() - 48);
	if (!min) hour++;
	Wire.beginTransmission(DS3231_I2C_ADDRESS);
	Wire.write(0x00);
	Wire.write(sec);
	Wire.write(decToBcd(min));
	Wire.write(decToBcd(hour));
	//Wire.write(decToBcd(day));
	//Wire.write(decToBcd(date));
	//Wire.write(decToBcd(month));
	//Wire.write(decToBcd(year));
	Wire.endTransmission();

	minPlus = hourPlus = 0;
}

void get3231Date() {
	// send request to receive data starting at register 0
	Wire.beginTransmission(DS3231_I2C_ADDRESS); // 104 is DS3231 device address
	Wire.write(0x00); // start at register 0
	Wire.endTransmission();
	Wire.requestFrom(DS3231_I2C_ADDRESS, 7); // request seven bytes

	if (Wire.available()) {
		sec = Wire.read(); // get seconds
		minRtc = Wire.read(); // get minutes
		hourRtc = Wire.read();   // get hours
	//	day = Wire.read();
	//	date = Wire.read();
	//	month = Wire.read(); //temp month
	//	year = Wire.read();

		sec = (((sec & B11110000) >> 4) * 10 + (sec & B00001111)); // convert BCD to decimal
		minRtc = (((minRtc & B11110000) >> 4) * 10 + (minRtc & B00001111)); // convert BCD to decimal
		hourRtc = (((hourRtc & B00110000) >> 4) * 10 + (hourRtc & B00001111)); // convert BCD to decimal (assume 24 hour mode)
	//	day = (day & B00000111); // 1-7
	//	date = (((date & B00110000) >> 4) * 10 + (date & B00001111)); // 1-31
	//	month = (((month & B00010000) >> 4) * 10 + (month & B00001111)); //msb7 is century overflow
	//	year = (((year & B11110000) >> 4) * 10 + (year & B00001111));
	}
	else {
		//oh noes, no data!
	}

	/*switch (day) {
	case 1:
		strcpy(weekDay, "Sun");
		break;
	case 2:
		strcpy(weekDay, "Mon");
		break;
	case 3:
		strcpy(weekDay, "Tue");
		break;
	case 4:
		strcpy(weekDay, "Wed");
		break;
	case 5:
		strcpy(weekDay, "Thu");
		break;
	case 6:
		strcpy(weekDay, "Fri");
		break;
	case 7:
		strcpy(weekDay, "Sat");
		break;
	}*/
}

float get3231Temp() {
	//temp registers (11h-12h) get updated automatically every 64s
	Wire.beginTransmission(DS3231_I2C_ADDRESS);
	Wire.write(0x11);
	Wire.endTransmission();
	Wire.requestFrom(DS3231_I2C_ADDRESS, 2);

	if (Wire.available()) {
		tMSB = Wire.read(); //2's complement int portion
		tLSB = Wire.read(); //fraction portion

		temp3231 = (tMSB & B01111111); //do 2's math on Tmsb
		temp3231 += ((tLSB >> 6) * 0.25); //only care about bits 7 & 8
	}
	else {
		//error! no data!
	}
	return temp3231;
}

void blinkHM() {
	if (isblinkH == true) {
		if (time >= wait_t + 300) {
			//Serial.println("hour on");
			if ((hour == 0 || hour == 12) && min == 0) {
				strip.setPixelColor(23, 0, 0, 0);
				strip.show();
			}
			else {
				printled(23);
				strip.show();
			}
			isblinkH = false;
		}
	}
	if (isblinkM == true) {
		if (time >= wait_m + 300) {
			//Serial.println("min on");
			if (min == 0) {
				strip.setPixelColor(0, 0, 0, 0);
				strip.show();
			}
			else {
				printled(0);
				strip.show();
			}
			isblinkM = false;
		}
	}
}

void showSerialTime() {

	Serial.print(hour, DEC);
	Serial.print(":");
	Serial.print(min, DEC);
	Serial.print(":");
	Serial.print(sec, DEC);
	Serial.print(" - Temp: ");
	Serial.println(get3231Temp());

	Serial.print(hourRtc);
	Serial.print(", ");
	Serial.print(hourPlus);
	Serial.print(" ; ");
	Serial.print(minRtc);
	Serial.print(",");
	Serial.println(minPlus);
	Serial.println();
}
