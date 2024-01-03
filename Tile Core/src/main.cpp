#include <Adafruit_NeoPixel.h>
#include "HID-Project.h"
uint8_t rawhidData[255];
byte megabuff[64];

#define TypeGeneral 0b00010001
#define TypeParentManagement 0b00100010
#define TypeHereIsMyNID 0b00110011
#define TypeNeedNID 0b01000100
#define TypeReportHID 0b01010101
#define TypeReportNeighbours 0b01100110
#define TypeHereIsYourNID 0b01110111
#define TypeChangeHardwareID 0b10001000
#define TypeTileCommand 0b10011001
#define TypeFrontendPing 0b10101010


int connected_to = 0;
int currentLength = 0;
int currentType = 0;
int currentTarget = 0;
int currentSender = 0;
bool activeMessage = false;


Adafruit_NeoPixel pixels(1, 10, NEO_GRB + NEO_KHZ800);

uint8_t currentLEDValues[3];
uint8_t newLEDValues[3];
bool newLEDValuesAvailable = true;

int highestNetworkID = 1;


void sendData(uint8_t type, uint8_t target_network_id, const int *data, uint8_t size)
{
	Serial1.write(type);
	Serial1.write(target_network_id);
	Serial1.write(1);
	Serial1.write(size);

	for (int curByte = 0; curByte < size; curByte++)
	{
		Serial1.write(data[curByte]);
	}
}

void resetBuf()
{
	for (int i = 0; i < sizeof(megabuff); i++)
	{
		megabuff[i] = 0;
	}
}

void sendToFrontend(uint8_t type, uint8_t sender, const int *data, uint8_t size)
{
	resetBuf();
	megabuff[0] = type;
	megabuff[1] = 1;
	megabuff[2] = sender;
	megabuff[3] = size;
	for (int i = 0; i < size; i++)
	{
		megabuff[4 + i] = data[i];
	}
	RawHID.write(megabuff, sizeof(megabuff));
}

bool is_valid_data_type(uint8_t type) {
	int data_types[] = {TypeGeneral, TypeParentManagement, TypeHereIsMyNID, TypeNeedNID, TypeReportHID, TypeReportNeighbours, TypeHereIsYourNID, TypeChangeHardwareID, TypeTileCommand, TypeFrontendPing};
	for (int i = 0; i < 12; i++)
	{
		if (type == data_types[i])
		{
			return true;
		}
	}
	return false;
}

void setNewLEDValuesAvailable() {
	newLEDValuesAvailable = false;
	for (int j = 0; j < 3; j++) {
		if (currentLEDValues[j] != newLEDValues[j]) {
			newLEDValuesAvailable = true;
		}
	}
}

void setNewLEDValues(int r, int g, int b) {
	newLEDValues[0] = r;
	newLEDValues[1] = g;
	newLEDValues[2] = b;
	setNewLEDValuesAvailable();
}



void setup() {
	pixels.begin();
	pixels.clear();
	pixels.show();
	pixels.setBrightness(25);
	randomSeed(analogRead(A1));
	pinMode(0, INPUT_PULLUP);
	pinMode(5, INPUT);
	setNewLEDValues(255, 0, 0);
	delay(50);

	RawHID.begin(rawhidData, sizeof(rawhidData));
	Serial.begin(115200);
	Serial1.begin(115200);


	Serial.println("Starting");
}


void loop() {
	if (newLEDValuesAvailable) {
		for (int i = 0; i < 3; i++) {
			if (currentLEDValues[i] > newLEDValues[i]) {
				currentLEDValues[i]--;
			}
			if (currentLEDValues[i] < newLEDValues[i]) {
				currentLEDValues[i]++;
			}
			pixels.setPixelColor(0, pixels.Color(currentLEDValues[0], currentLEDValues[1], currentLEDValues[2]));
			pixels.show();
		}
		delay(5);

		setNewLEDValuesAvailable();
	}


	delay(1);
	/* NEIGHBOUR MANAGEMENT */
	if (!digitalRead(5) and connected_to != 0)
	{
		Serial.println("Update connected to: 0");
		connected_to = 0;
		int data[] = {0, 0, connected_to, 0};
		sendToFrontend(TypeReportNeighbours, 1, data, 4);
	}
	/* NEIGHBOUR MANAGEMENT */

	/* MESSAGES FROM FRONTEND */
	if (RawHID.available() >= 4)
	{
		uint8_t type = RawHID.read();
		uint8_t target = RawHID.read();
		uint8_t sender = RawHID.read();
		uint8_t length = RawHID.read();
		Serial.print("- Type: ");
		Serial.print(type);
		Serial.print(", Target: ");
		Serial.print(target);
		Serial.print(", Sender: ");
		Serial.print(sender);
		Serial.print(", Length: ");
		Serial.print(length);
		Serial.print(", Data: ");
		while (RawHID.available() < length) {}
		int buffer[length];
		for (int i = 0; i < length; i++) {
			buffer[i] = RawHID.read();
		}
		for (int i = 0; i < (60 - length); i++) {
			RawHID.read();
		}

		if (type == TypeFrontendPing) {
			if (buffer[0] == 2) {
				Serial.println("First Connect!");
				setNewLEDValues(0, 0, 0);
			}
			else if (buffer[0] == 3) {
				Serial.println("Disconnected!");
				setNewLEDValues(255, 0, 0);
				int value = 3;
				for (int i = highestNetworkID; i >= 2; i--) {
					sendData(TypeTileCommand, i, &value, 1);
				}
				highestNetworkID = 1;
			}
		}
		else if (type == TypeHereIsYourNID)
		{
			if (buffer[0] > highestNetworkID) {
				highestNetworkID = buffer[0];
			}
			Serial.println(buffer[0]);
			sendData(TypeHereIsYourNID, 0, buffer, 1);
		}
		else if (type == TypeChangeHardwareID)
		{
			for (int i = 0; i < 4; i++) {
				Serial.print(buffer[i]);
				Serial.print(",");
			}
			Serial.println();
			sendData(TypeChangeHardwareID, target, buffer, 4);
		}
		else if (type == TypeTileCommand)
		{
			if (target == 1) {
				if (buffer[0] == 1) {
					setNewLEDValues(buffer[1], buffer[2], buffer[3]);
				}
				else if (buffer[0] == 2) {
					setNewLEDValues(0, 0, 0);
				}
				else if (buffer[0] == 3 or buffer[0] == 4) {
					for (int i = highestNetworkID; i >= 2; i--) {
						sendData(TypeTileCommand, i, &buffer[0], 1);
					}
					highestNetworkID = 1;
				}
			}
			else {
				sendData(TypeTileCommand, target, buffer, length);
			}
		}
	}
	/* MESSAGES FROM FRONTEND */


	if (Serial1.available() >= (activeMessage ? currentLength : 4))
	{
		if (!activeMessage)
		{
			currentType = Serial1.read();
			if (!is_valid_data_type(currentType))
			{
				Serial.print("Invalid Type: ");
				Serial.println(currentType);
				return;
			}
			currentTarget = Serial1.read();
			currentSender = Serial1.read();
			currentLength = Serial1.read();
			Serial.print("Type: ");
			Serial.print(currentType);
			Serial.print(", Target: ");
			Serial.print(currentTarget);
			Serial.print(", Sender: ");
			Serial.print(currentSender);
			Serial.print(", Length: ");
			Serial.print(currentLength);
			if (currentLength == 255)
			{
				currentLength = 0;
				Serial.println(" ???dafuq???");
				return;
			}
			activeMessage = true;
		}
		else
		{
			Serial.print(", Data: ");
			int buffer[currentLength] = {};
			for (int i = 0; i < currentLength; i++)
			{
				buffer[i] = Serial1.read();
				Serial.print(buffer[i]);
				Serial.print(", ");
			}
			Serial.println();

			/* NORMAL TILE MANAGEMENT */
			if (currentType == TypeParentManagement)
			{
				if (buffer[0] == 1) {
					int value = 2;
					sendData(TypeParentManagement, 0, &value, 1);
				}
			}

			else if (currentType == TypeHereIsMyNID)
			{
				int id = buffer[0];
				if (id != connected_to)
				{
					connected_to = id;
					int id = 1;
					int data[] = {0, 0, connected_to, 0};
					sendToFrontend(TypeReportNeighbours, 1, data, 4);
					sendData(TypeHereIsMyNID, 0, &id, 1);
					Serial.print("Update connected to: ");
					Serial.println(connected_to);
				}
			}
			/* NORMAL TILE MANAGEMENT */

			/* THINGS THAT HAVE TO GO TO FRONTEND */
			else if (currentType == TypeNeedNID)
			{
				Serial.println("Requested Network ID");
				sendToFrontend(TypeNeedNID, 0, {}, 0);
			}

			else if (currentType == TypeGeneral)
			{
				sendToFrontend(TypeGeneral, currentSender, buffer, currentLength);
			}

			else if (currentType == TypeReportHID)
			{
				Serial.println("Reported Hardware ID");
				sendToFrontend(TypeReportHID, currentSender, buffer, currentLength);
			}

			else if (currentType == TypeReportNeighbours)
			{
				Serial.println("Reported Neighbours");
				sendToFrontend(TypeReportNeighbours, currentSender, buffer, 4);
			}
			/* THINGS THAT HAVE TO GO TO FRONTEND */
			activeMessage = false;
		}
	}
}

