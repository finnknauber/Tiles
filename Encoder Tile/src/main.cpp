#include <Arduino.h>
#include <T2T.h>

T2T tile;

#define LED PC13
#define BUTTON1 PA8
#define BUTTON2 PA9
#define ENCODERA PA10
#define ENCODERB PA11
#define ENCODERSW PB15


void ledOn() {
	digitalWrite(LED, LOW);
}

void ledOff() {
	digitalWrite(LED, HIGH);
}

void ledReverse() {
	digitalWrite(LED, !digitalRead(LED));
}


// void call1() {
// 	if (digitalRead(ENCODERA) == LOW and digitalRead(ENCODERB) == LOW) {
// 		ledOn();
// 		tile.writeByte('-', SUBUART_CHANNEL_1);
// 		tile.writeByte('-', SUBUART_CHANNEL_2);
// 		tile.writeByte('-', SUBUART_CHANNEL_3);
// 		tile.writeByte('-', SUBUART_CHANNEL_4);
// 	}
// 	delay(10);
// 	ledOff();
// }

// void call2() {
// 	if (digitalRead(ENCODERA) == LOW and digitalRead(ENCODERB) == LOW) {
// 		ledOn();
// 		tile.writeByte('+', SUBUART_CHANNEL_1);
// 		tile.writeByte('+', SUBUART_CHANNEL_2);
// 		tile.writeByte('+', SUBUART_CHANNEL_3);
// 		tile.writeByte('+', SUBUART_CHANNEL_4);
// 	}
// 	delay(10);
// 	ledOff();
// }


// void setup() {
// 	pinMode(LED, OUTPUT);
// 	pinMode(ENCODERA, INPUT);
// 	pinMode(ENCODERB, INPUT);
// 	tile.begin();
// 	ledOn();
// 	delay(50);
// 	ledOff();
// 	delay(5);
// 	attachInterrupt(digitalPinToInterrupt(ENCODERA), call1, FALLING);
// 	attachInterrupt(digitalPinToInterrupt(ENCODERB), call2, FALLING);
// }

// void loop() {
// 	delay(500);
// }




void call1() {
	ledOn();
	tile.writeByte('-', SUBUART_CHANNEL_1);
	tile.writeByte('-', SUBUART_CHANNEL_2);
	tile.writeByte('-', SUBUART_CHANNEL_3);
	tile.writeByte('-', SUBUART_CHANNEL_4);
	delay(10);
	ledOff();
}

void call2() {
	ledOn();
	tile.writeByte('+', SUBUART_CHANNEL_1);
	tile.writeByte('+', SUBUART_CHANNEL_2);
	tile.writeByte('+', SUBUART_CHANNEL_3);
	tile.writeByte('+', SUBUART_CHANNEL_4);
	delay(10);
	ledOff();
}


void setup() {
	pinMode(LED, OUTPUT);
	pinMode(BUTTON1, INPUT);
	pinMode(BUTTON2, INPUT);
	tile.begin();
	ledOn();
	delay(50);
	ledOff();
	delay(5);

	attachInterrupt(digitalPinToInterrupt(BUTTON1), call1, FALLING);
	attachInterrupt(digitalPinToInterrupt(BUTTON2), call2, FALLING);
}


void loop() {
	delay(500);
}

