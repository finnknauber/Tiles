#include <Arduino.h>
#include <T2T.h>

T2T tile;

#define LED PC13
#define ENCODERA PA10
#define ENCODERB PA11
#define ENCODERSW PB15
#define READ_UP PA1
#define READ_RIGHT PA9
#define READ_DOWN PA3
#define READ_LEFT PA4

int direction_pins[4] = {READ_UP, READ_RIGHT, READ_DOWN, READ_LEFT};
bool active_directions[4] = {false, false, false, false};
int network_id = 0;


void ledOn() {
	digitalWrite(LED, LOW);
}

void ledOff() {
	digitalWrite(LED, HIGH);
}

void ledReverse() {
	digitalWrite(LED, !digitalRead(LED));
}


void up_event() {
	if (digitalRead(READ_UP)) {
		active_directions[0] = true;
	}
	else {
		active_directions[0] = false;
	}
}

void right_event() {
	if (digitalRead(READ_RIGHT)) {
		active_directions[1] = true;
	}
	else {
		active_directions[1] = false;
	}
}

void down_event() {
	if (digitalRead(READ_DOWN)) {
		active_directions[2] = true;
	}
	else {
		active_directions[2] = false;
	}
}

void left_event() {
	if (digitalRead(READ_LEFT)) {
		active_directions[3] = true;
	}
	else {
		active_directions[3] = false;
	}
}


void rotate_right() {
	if (digitalRead(ENCODERA) == LOW and digitalRead(ENCODERB) == LOW) {
		uint8_t data = 2;
		tile.sendData(TypeGeneral, 0, &data, 1);
		ledOn();
	}
	delay(10);
	ledOff();
}


void rotate_left() {
	if (digitalRead(ENCODERA) == LOW and digitalRead(ENCODERB) == LOW) {
		uint8_t data = 3;
		tile.sendData(TypeGeneral, 0, &data, 1);
		ledOn();
	}
	delay(10);
	ledOff();
}


void switch_pressed() {
	ledOn();
	uint8_t data = 0;
	if (digitalRead(ENCODERSW)) {
		data = 1;
	}
	int res = tile.sendData(TypeGeneral, 0, &data, 1);
	delay(10);
	if (res == 0) {
		ledOff();
	}
}


void setup() {
	pinMode(LED, OUTPUT);
	pinMode(ENCODERA, INPUT);
	pinMode(ENCODERB, INPUT);
	pinMode(ENCODERSW, INPUT);
	pinMode(READ_UP, INPUT);
	pinMode(READ_RIGHT, INPUT);
	pinMode(READ_DOWN, INPUT);
	pinMode(READ_LEFT, INPUT);
	ledOff();
	delay(100);
	tile.begin();
	for (int i=0; i<4; i++) {
		if (digitalRead(direction_pins[i])) {
			tile.sendData(TypeAreYouMaster, 0, {}, 0, i);
			active_directions[i] = true;
		}
	}

	while (tile.MASTER_DIRECTION == 100) {
		for (int i=0; i<4; i++) {
			if (digitalRead(direction_pins[i])) {
				if (tile.available(i)) {
					if (tile.readByte(i) == TypeACK) {
						tile.setParent(i);
					}
				}
			}
		}
	}


	ledOn();
	delay(50);
	ledOff();
	delay(5);
	attachInterrupt(digitalPinToInterrupt(ENCODERA), rotate_left, FALLING);
	attachInterrupt(digitalPinToInterrupt(ENCODERB), rotate_right, FALLING);
	attachInterrupt(digitalPinToInterrupt(ENCODERSW), switch_pressed, CHANGE);

	attachInterrupt(digitalPinToInterrupt(READ_UP), up_event, CHANGE);
	attachInterrupt(digitalPinToInterrupt(READ_RIGHT), right_event, FALLING);
	attachInterrupt(digitalPinToInterrupt(READ_DOWN), down_event, FALLING);
	attachInterrupt(digitalPinToInterrupt(READ_LEFT), left_event, FALLING);
}


void loop() {
	delay(100);
}

