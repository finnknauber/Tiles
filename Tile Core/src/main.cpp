#include <Arduino.h>
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

int connected_to = 0;
int currentLength = 0;
int currentType = 0;
int currentTarget = 0;
int currentSender = 0;
bool activeMessage = false;

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
	// TODO Checksum
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
	int data_types[] = {TypeGeneral, TypeParentManagement, TypeHereIsMyNID, TypeNeedNID, TypeReportHID, TypeReportNeighbours, TypeHereIsYourNID, TypeChangeHardwareID, TypeTileCommand};
	for (int i = 0; i < 11; i++)
	{
		if (type == data_types[i])
		{
			return true;
		}
	}
	return false;
}

void setup()
{
	randomSeed(analogRead(A1));
	pinMode(0, INPUT_PULLUP);
	pinMode(3, INPUT);

	RawHID.begin(rawhidData, sizeof(rawhidData));
	Serial.begin(115200);
	Serial1.begin(115200);
	delay(10);
	Serial.println("Starting");
}

void loop()
{
	delay(10);
	/* NEIGHBOUR MANAGEMENT */
	if (!digitalRead(3) and connected_to != 0)
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
		while (RawHID.available() < length)
		{
		}
		if (type == TypeHereIsYourNID)
		{
			int newAdress = RawHID.read();
			Serial.println(newAdress);
			sendData(TypeHereIsYourNID, 0, &newAdress, 1);
		}
		else if (type == TypeChangeHardwareID)
		{
			int new_id[4] = {};
			for (int i = 0; i < 4; i++)
			{
				new_id[i] = RawHID.read();
				Serial.print(new_id[i]);
				Serial.print(",");
			}
			Serial.println();
			sendData(TypeChangeHardwareID, target, new_id, 4);
		}
		else if (type == TypeTileCommand)
		{
			int value = RawHID.read();
			sendData(TypeTileCommand, target, &value, 1);
		}
		for (int i = 0; i < (60 - length); i++)
		{
			RawHID.read();
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
