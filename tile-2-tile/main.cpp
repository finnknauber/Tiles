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
uint8_t active_directions[4] = {0, 0, 0, 0};


void ledOn() {
	digitalWrite(LED, LOW);
}

void ledOff() {
	digitalWrite(LED, HIGH);
}

void ledReverse() {
	digitalWrite(LED, !digitalRead(LED));
}


void rotate_right() {
	if (digitalRead(ENCODERA) == LOW and digitalRead(ENCODERB) == LOW) {
		uint8_t data = 2;
		tile.sendData(TypeGeneral, 1, &data, 1);
		ledOn();
	}
	delay(10);
	ledOff();
}


void rotate_left() {
	if (digitalRead(ENCODERA) == LOW and digitalRead(ENCODERB) == LOW) {
		uint8_t data = 3;
		tile.sendData(TypeGeneral, 1, &data, 1);
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
	int res = tile.sendData(TypeGeneral, 1, &data, 1);
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
	ledOn();
	delay(100);
	tile.begin();
	for (int i=0; i<4; i++) {
		if (digitalRead(direction_pins[i])) {
			tile.sendData(TypeAreYouMaster, 0, {}, 0, i);
		}
	}

	while (tile.MASTER_DIRECTION == 100) {
		for (int i=0; i<4; i++) {
			if (digitalRead(direction_pins[i])) {
				if (tile.available(i)) {
					if (tile.readByte(i) == TypeACK) {
						while (tile.available(i) < 3) {}
						uint8_t target = tile.readByte(i);
						tile.readByte(i);
						tile.readByte(i);
						if (target == 0) {
							tile.setParent(i);
						}
					}
					else {
						while (tile.available(i) < 3) {}
						tile.readByte(i);
						tile.readByte(i);
						int length = tile.readByte(i);
						while (tile.available(i) < length) {}
						for (int j=0; j<length; j++) {
							tile.readByte(i);
						}
						tile.sendData(TypeAreYouMaster, 255, {}, 0, i);
					}
				}
			}
		}
	}

	while (tile.NETWORK_ID <= 0) {
		tile.sendData(TypeNeedNID, 1, {}, 0);
		while (tile.available() < 4) {}
		if (tile.readByte() == TypeHereIsYourNID) {
			// target is 0
			if (tile.readByte() == 0) {
				// ignore sender id
				tile.readByte();
				// if length == 1
				if (tile.readByte() == 1) {
					tile.NETWORK_ID = tile.readByte();
				}
			}
		}
	}

	tile.sendData(TypeReportHID, 1, tile.UID, 4);
	while (!tile.available()) {}
	int response = tile.readByte();
	if (response == TypeChangeHardwareID) {
		uint8_t oldUID[] = {tile.UID[0], tile.UID[1], tile.UID[2], tile.UID[3]};
		while (tile.UID[0] == oldUID[0] and tile.UID[1] == oldUID[1] and tile.UID[2] == oldUID[2] and tile.UID[3] == oldUID[3]) {
			while (tile.available() < 3) {}
			int target = tile.readByte();
			int sender = tile.readByte();
			int length = tile.readByte();
			if (target == tile.NETWORK_ID and sender == 1 and length == 4) {
				while (tile.available() < length) {}
				tile.setUID(tile.readByte(), tile.readByte(), tile.readByte(), tile.readByte());
			}
		}
	}
	else if (response == TypeACK) {
		while (tile.available() < 3) {}
		tile.readByte();
		tile.readByte();
		tile.readByte();
	}


	for (int i=0; i<4; i++) {
		if (digitalRead(direction_pins[i])) {
			tile.sendData(TypeGiveMeYourNID, 0, {}, 0, i);
		}
	}

	for (int i=0; i<4; i++) {
		if (digitalRead(direction_pins[i])) {
			while (tile.available(i) < 5) {}
			if (tile.readByte(i) == TypeHereIsMyNID) {
				int target = tile.readByte(i);
				int sender = tile.readByte(i);
				if (tile.readByte() == 1) {
					int id = tile.readByte(i);
					if (id != 0) {
						active_directions[i] = id;
					}
				}
			}
		}
	}

	tile.sendData(TypeReportNeighbours, 1, active_directions, 4);

	ledOff();
	delay(5);
	attachInterrupt(digitalPinToInterrupt(ENCODERA), rotate_left, FALLING);
	attachInterrupt(digitalPinToInterrupt(ENCODERB), rotate_right, FALLING);
	attachInterrupt(digitalPinToInterrupt(ENCODERSW), switch_pressed, CHANGE);
}


void loop() {
	for (int i=0; i<4; i++) {
		// *****REPORT NEIGHBOURS***** //
		// TODO what if ID changes but device stays connected??? ahhhh
		if (digitalRead(direction_pins[i]) and active_directions[i] == 0) {
			tile.sendData(TypeGiveMeYourNID, 0, {}, 0, i);
			while (tile.available(i) < 5) {}
			if (tile.readByte(i) == TypeHereIsMyNID) {
				tile.readByte(i);
				tile.readByte(i);
				if (tile.readByte(i) == 1) {
					int id = tile.readByte(i);
					if (id != 0) {
						active_directions[i] = id;
						tile.sendData(TypeReportNeighbours, 1, active_directions, 4);
					}
				}
			}
		}
		else if (!digitalRead(direction_pins[i]) and active_directions[i] != 0) {
			active_directions[i] == 0;
			tile.sendData(TypeReportNeighbours, 1, active_directions, 4);
		}
		// *****REPORT NEIGHBOURS***** //

		// *****HANDLE INCOMING MESSAGES***** //
		// if (tile.available(i) >= 4) {
		// 	uint8_t type = tile.readByte(i);
		// 	uint8_t target = tile.readByte(i);
		// 	uint8_t sender = tile.readByte(i);
		// 	uint8_t length = tile.readByte(i);
		// 	while (tile.available(i) < length) {}
		// 	// Target is this tile
		// 	if (target == tile.NETWORK_ID) {
		// 		if (type == TypeAreYouMaster) {
		// 			tile.sendData(TypeACK, sender, {}, 0, i);
		// 		}
		// 	}
		// 	// Target is master
		// 	else if (target == 1) {
		// 		uint8_t buffer[length];
		// 		tile.readData(buffer, length, i);
		// 		tile.sendData(type, target, buffer, length, -1, sender);
		// 	}
		// 	// Target is a new tile (ID=0)
		// 	else if (target == 0) {
		// 		if (type == TypeGiveMeYourNID) {
		// 			tile.sendData(TypeHereIsMyNID, sender, &tile.NETWORK_ID, 1, i);
		// 		}
		// 	}
		// 	// Broadcast message
		// 	else if (target == 255) {

		// 	}
		// 	// Target is some other tile in network
		// 	else {

		// 	}
		// }
		// *****HANDLE INCOMING MESSAGES***** //
	}
	delay(1);
}

