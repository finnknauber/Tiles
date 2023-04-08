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
#define TILETYPE 0x01

int direction_pins[4] = {READ_UP, READ_RIGHT, READ_DOWN, READ_LEFT};
bool active_new_tiles[4] = {false, false, false, false};
uint8_t active_directions[4] = {0, 0, 0, 0};

bool askedForNetworkID = false;
bool sendDataFlag = false;
uint8_t data = 0;

void ledOn()
{
	digitalWrite(LED, LOW);
}

void ledOff()
{
	digitalWrite(LED, HIGH);
}

void ledReverse()
{
	digitalWrite(LED, !digitalRead(LED));
}

bool is_valid_data_type(uint8_t type)
{
	int data_types[] = {TypeGeneral, TypeIAmMaster, TypeAreYouMaster, TypeNeedNID, TypeReportHID, TypeHereIsMyNID, TypeHereIsYourNID, TypeReportNeighbours, TypeChangeHardwareID, TypeResetNetworkID};
	for (int i = 0; i < 10; i++)
	{
		if (type == data_types[i])
		{
			return true;
		}
	}
	return false;
}

void rotate_right()
{
	if (digitalRead(ENCODERA) == LOW and digitalRead(ENCODERB) == LOW)
	{
		data = 2;
		sendDataFlag = true;
		ledOn();
	}
	delay(10);
	ledOff();
}

void rotate_left()
{
	if (digitalRead(ENCODERA) == LOW and digitalRead(ENCODERB) == LOW)
	{
		data = 3;
		sendDataFlag = true;
		ledOn();
	}
	delay(10);
	ledOff();
}

void switch_pressed()
{
	ledOn();
	data = 0;
	if (digitalRead(ENCODERSW))
	{
		data = 1;
	}
	sendDataFlag = true;
	delay(10);
	ledOff();
}

void setup()
{
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
	for (int i = 0; i < 4; i++)
	{
		if (digitalRead(direction_pins[i]))
		{
			tile.sendData(TypeAreYouMaster, 0, {}, 0, i);
		}
	}

	delay(5);
	attachInterrupt(digitalPinToInterrupt(ENCODERA), rotate_left, FALLING);
	attachInterrupt(digitalPinToInterrupt(ENCODERB), rotate_right, FALLING);
	attachInterrupt(digitalPinToInterrupt(ENCODERSW), switch_pressed, CHANGE);
}

void loop()
{
	delay(25);
	if (tile.MASTER_DIRECTION != 100)
	{
		if (sendDataFlag)
		{
			int res = tile.sendData(TypeGeneral, 1, &data, 1);
			sendDataFlag = false;
		}
		if (tile.NETWORK_ID == 0 and not askedForNetworkID)
		{
			tile.sendData(TypeNeedNID, 1, {}, 0);
			askedForNetworkID = true;
		}
	}

	for (int direction = 0; direction < 4; direction++)
	{
		// *****REPORT NEIGHBOURS***** //
		if (!digitalRead(direction_pins[direction]) and active_directions[direction] != 0)
		{
			active_directions[direction] = 0;
			if (tile.NETWORK_ID != 0)
			{
				tile.sendData(TypeReportNeighbours, 1, active_directions, 4);
			}
		}
		// *****REPORT NEIGHBOURS***** //

		// *****HANDLE INCOMING MESSAGES***** //
		if (tile.available(direction) >= 4)
		{
			uint8_t type = tile.readByte(direction);
			while (!is_valid_data_type(type))
			{
				while (!tile.available(direction))
				{
				}
				type = tile.readByte(direction);
			}
			uint8_t target = tile.readByte(direction);
			uint8_t sender = tile.readByte(direction);
			uint8_t length = tile.readByte(direction);
			while (tile.available(direction) < length)
			{
			}
			// Target is this tile
			if (target == tile.NETWORK_ID and tile.NETWORK_ID != 0)
			{
				if (type == TypeChangeHardwareID)
				{
					uint8_t buffer[length];
					tile.readData(buffer, length, direction);
					tile.setUID(buffer[0], buffer[1], buffer[2], buffer[3]);
				}
				else if (type == TypeResetNetworkID)
				{
					ledOn();
					tile.NETWORK_ID = 0;
					askedForNetworkID = false;
				}
			}
			// Target is a new tile (ID=0) or me with no network id
			else if (target == 0)
			{
				if (type == TypeAreYouMaster)
				{
					if (tile.MASTER_DIRECTION != 100)
					{
						tile.sendData(TypeIAmMaster, 0, {}, 0, direction);
					}
					else
					{
						tile.sendData(TypeIAmNotMaster, 0, {}, 0, direction);
					}
				}
				else if (type == TypeIAmMaster)
				{
					if (tile.MASTER_DIRECTION == 100)
					{
						tile.setParent(direction);
					}
				}
				else if (type == TypeIAmNotMaster)
				{
					if (tile.MASTER_DIRECTION == 100)
					{
						tile.sendData(TypeAreYouMaster, 0, {}, 0, direction);
					}
				}
				else if (type == TypeHereIsYourNID and tile.NETWORK_ID == 0)
				{
					tile.NETWORK_ID = tile.readByte();
					uint8_t uid_type[5] = {tile.UID[0], tile.UID[1], tile.UID[2], tile.UID[3], TILETYPE};
					tile.sendData(TypeReportHID, 1, uid_type, 5);
					ledOff();
					for (int j = 0; j < 4; j++)
					{
						tile.sendData(TypeHereIsMyNID, 0, &tile.NETWORK_ID, 1, j);
					}
				}
				else if (type == TypeHereIsMyNID and tile.NETWORK_ID != 0)
				{
					int id = tile.readByte(direction);
					if (id != 0 and active_directions[direction] != id)
					{
						tile.sendData(TypeHereIsMyNID, 0, &tile.NETWORK_ID, 1, direction);
						active_directions[direction] = id;
						tile.sendData(TypeReportNeighbours, 1, active_directions, 4);
					}
				}

				else
				{
					uint8_t buffer[length];
					tile.readData(buffer, length, direction);
					for (int i = 0; i < 4; i++)
					{
						if (active_new_tiles[i])
						{
							if (type == TypeHereIsYourNID)
							{
								active_new_tiles[i] = false;
							}
							tile.sendData(type, 0, buffer, length, i, sender);
							break;
						}
					}
				}
			}
			// Target is master
			else if (target == 1)
			{
				uint8_t buffer[length];
				tile.readData(buffer, length, direction);
				if (sender == 0)
				{
					active_new_tiles[direction] = true;
				}
				tile.sendData(type, target, buffer, length, -1, sender);
			}
			// Broadcast message
			else if (target == 255)
			{
			}
			// Target is some other tile in network
			else
			{
				uint8_t buffer[length];
				tile.readData(buffer, length, direction);
				for (int j = 0; j < 4; j++)
				{
					if (j != tile.MASTER_DIRECTION)
					{
						tile.sendData(type, target, buffer, length, j, sender);
					}
				}
			}
		}
		// *****HANDLE INCOMING MESSAGES***** //
	}
}
