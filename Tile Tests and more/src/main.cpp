#include <Arduino.h>
#include <T2T.h>

T2T tile;

#define LED PC13
#define BUTTON1 PA8


void ledOn() {
	digitalWrite(LED, LOW);
}

void ledOff() {
	digitalWrite(LED, HIGH);
}


void setup() {
	delay(3000);
	pinMode(LED, OUTPUT);
	pinMode(BUTTON1, INPUT);
	tile.begin();
	ledOn();
	delay(250);
	ledOff();
	delay(500);
}

void loop() {
	if (digitalRead(BUTTON1) == LOW) {
		tile.writeByte(65, SUBUART_CHANNEL_2);
		ledOn();
		delay(250);
		ledOff();
		delay(50);
	}
}

