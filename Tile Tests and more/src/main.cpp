#include <Arduino.h>
#include <T2T.h>

T2T tile;

#define LED PC13
#define BUTTON1 PA8
#define BUTTON2 PA9


void ledOn() {
	digitalWrite(LED, LOW);
}

void ledOff() {
	digitalWrite(LED, HIGH);
}

void setup() {
	pinMode(LED, OUTPUT);
	pinMode(BUTTON1, INPUT);
	pinMode(BUTTON2, INPUT);
	ledOff();
	delay(500);
	if (tile.begin() != 0) {
		ledOn();
		while (true) {}
	}
	delay(1000);
	ledOn();
	delay(100);
	ledOff();
	delay(250);
	// uint8_t register_value = 0x04;
	// uint8_t value = 0x07;
	// if ((tile.sendPing(register_value, true) & value) == 0b00000011) {
	uint8_t register_value = 0x05;
	uint8_t value = 0xFF;
	if ((tile.sendPing(register_value, true) & value) == 0x00) {
		ledOn();
	}
	delay(2500);
	ledOff();
}

void loop() {
	delay(250);
}

