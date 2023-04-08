#include <Arduino.h>
#include "HID-Project.h"
uint8_t rawhidData[255];
byte megabuff[64];

#define TypeGeneral           0b00010001
#define TypeIAmMaster         0b00100010
#define TypeIAmNotMaster      0b00110011
#define TypeAreYouMaster      0b01000100
#define TypeNeedNID           0b01010101
#define TypeReportHID         0b01100110
#define TypeHereIsMyNID       0b10001000
#define TypeHereIsYourNID     0b10101010
#define TypeReportNeighbours  0b10011001
#define TypeChangeHardwareID  0b10111011
#define TypeResetNetworkID    0b11001100


bool active_message = false;
int message_type = 0;
int connected_to = 0;
bool connected_to_request_sent = true;


void sendData(uint8_t type, uint8_t target_network_id, const int *data, uint8_t size) {
	while (!Serial1.availableForWrite()) {}
	Serial1.write(type);
	while (!Serial1.availableForWrite()) {}
	Serial1.write(target_network_id);
	while (!Serial1.availableForWrite()) {}
	Serial1.write(1);
	while (!Serial1.availableForWrite()) {}
	Serial1.write(size);

	for (int curByte=0; curByte<size; curByte++) {
		while (!Serial1.availableForWrite()) {}
		Serial1.write(data[curByte]);
	}
	// TODO Checksum
}


void resetBuf() {
	for (int i = 0; i < sizeof(megabuff); i++) {
		megabuff[i] = 0;
	}
}


void sendToFrontend(uint8_t type, uint8_t sender, const int *data, uint8_t size) {
	resetBuf();
	megabuff[0] = type;
	megabuff[1] = 1;
	megabuff[2] = sender;
	megabuff[3] = size;
	for (int i=0; i<size; i++) {
		megabuff[4+i] = data[i];
	}
	RawHID.write(megabuff, sizeof(megabuff));
}

bool is_valid_data_type(uint8_t type) {
	int data_types[] = {TypeGeneral, TypeIAmMaster, TypeAreYouMaster, TypeNeedNID, TypeReportHID, TypeHereIsMyNID, TypeHereIsYourNID, TypeReportNeighbours, TypeChangeHardwareID, TypeResetNetworkID};
	for (int i=0; i<10; i++) {
		if (type == data_types[i]) {
			return true;
		}
	}
	return false;
}


void setup() {
	randomSeed(analogRead(A1));
	pinMode(0, INPUT_PULLUP);
	pinMode(3, INPUT);

	RawHID.begin(rawhidData, sizeof(rawhidData));
	Serial.begin(115200);
	Serial1.begin(115200);
	delay(10000);
}


void loop() {
	delay(10);
	/* NEIGHBOUR MANAGEMENT */
	if (!digitalRead(3) and connected_to != 0) {
		Serial.println("Update connected to: 0");
		connected_to = 0;
		int data[] = {0, 0, connected_to, 0};
		sendToFrontend(TypeReportNeighbours, 1, data, 4);
	}
	/* NEIGHBOUR MANAGEMENT */


	/* MESSAGES FROM FRONTEND */
	if (RawHID.available() >= 4) {
		uint8_t type = RawHID.read();
		uint8_t target = RawHID.read();
		uint8_t sender = RawHID.read();
		uint8_t length = RawHID.read();
		while (RawHID.available() < 60) {}
		if (type == TypeHereIsYourNID) {
			int newAdress = RawHID.read();
			while (RawHID.available()) {RawHID.read();}
			sendData(TypeHereIsYourNID, 0, &newAdress, 1);
		}
		else if (type == TypeChangeHardwareID) {
			int new_id[4] = {};
			for (int i=0; i<4; i++) {
				new_id[i] = RawHID.read();
			}
			while (RawHID.available()) {RawHID.read();}
			sendData(TypeChangeHardwareID, sender, new_id, 4);
		}
	}
	/* MESSAGES FROM FRONTEND */


	if (Serial1.available() >= 4) {
		uint8_t type = Serial1.read();
		while (!is_valid_data_type(type)) {
			while(!Serial1.available()) {}
			type = Serial1.read();
		}
		uint8_t target = Serial1.read();
		uint8_t sender = Serial1.read();
		uint8_t length = Serial1.read();
		Serial.print("Type: ");
		Serial.print(type);
		Serial.print(", Target: ");
		Serial.print(target);
		Serial.print(", Sender: ");
		Serial.print(sender);
		Serial.print(", Length: ");
		Serial.print(length);
		Serial.print(", Data: ");
		while (Serial1.available() < length) {}
		int buffer[length] = {};
		for (int i=0; i<length; i++) {
			buffer[i] = Serial1.read();
			Serial.print(buffer[i]);
			Serial.print(", ");
		}
		Serial.println();

		/* NORMAL TILE MANAGEMENT */
		if (type == TypeAreYouMaster) {
			sendData(TypeIAmMaster, 0, {}, 0);
		}

		else if (type == TypeHereIsMyNID) {
			int id = buffer[0];
			if (id != connected_to) {
				connected_to = id;
				int id = 1; int data[] = {0, 0, connected_to, 0};
				sendToFrontend(TypeReportNeighbours, 1, data, 4);
				sendData(TypeHereIsMyNID, 0, &id, 1);
				Serial.print("Update connected to: ");
				Serial.println(connected_to);
			}
		}
		/* NORMAL TILE MANAGEMENT */


		/* THINGS THAT HAVE TO GO TO FRONTEND */
		else if (type == TypeNeedNID) {
			Serial.println("Requested Network ID");
			sendToFrontend(TypeNeedNID, 0, {}, 0);
		}

		else if (type == TypeGeneral) {
			sendToFrontend(TypeGeneral, sender, buffer, length);
		}

		else if (type == TypeReportHID) {
			Serial.println("Reported Hardware ID");
			sendToFrontend(TypeReportHID, sender, buffer, length);
		}

		else if (type == TypeReportNeighbours) {
			Serial.println("Reported Neighbours");
			sendToFrontend(TypeReportNeighbours, sender, buffer, 4);
		}
		/* THINGS THAT HAVE TO GO TO FRONTEND */
	}
}

