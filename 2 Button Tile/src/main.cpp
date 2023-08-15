#include <Arduino.h>
#include <T2T.h>

T2T tile;

#define LED PC13
#define BUTTON1 PA8
#define BUTTON2 PA9
#define READ_UP PA1
#define READ_RIGHT PB15
#define READ_DOWN PA3
#define READ_LEFT PA4
#define TILETYPE 0x02

int direction_pins[4] = {READ_UP, READ_RIGHT, READ_DOWN, READ_LEFT};
bool active_new_tiles[4] = {false, false, false, false};
bool active_children[4] = {false, false, false, false};
uint8_t active_directions[4] = {0, 0, 0, 0};

uint32_t lastAskedForMaster[4] = {0, 0, 0, 0};
uint32_t lastAskedForNetworkID = 0;
byte data = 0;
byte lastData = 0;

bool active_message[4] = {false, false, false, false};
uint8_t type[4] = {0, 0, 0, 0};
uint8_t target[4] = {0, 0, 0, 0};
uint8_t sender[4] = {0, 0, 0, 0};
uint8_t length[4] = {0, 0, 0, 0};

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


void button1_pressed()
{
	if (tile.NETWORK_ID != 0)
	{
		ledOn();
		bitWrite(data, 0, !digitalRead(BUTTON1));
		delay(10);
		ledOff();
	}
}

void button2_pressed()
{
	if (tile.NETWORK_ID != 0)
	{
		ledOn();
		bitWrite(data, 1, !digitalRead(BUTTON2));
		delay(10);
		ledOff();
	}
}

void setup()
{
	pinMode(LED, OUTPUT);
	pinMode(BUTTON1, INPUT);
	pinMode(BUTTON2, INPUT);
	pinMode(READ_UP, INPUT);
	pinMode(READ_RIGHT, INPUT);
	pinMode(READ_DOWN, INPUT);
	pinMode(READ_LEFT, INPUT);
	ledOn();
	tile.begin();
	delay(10);

	attachInterrupt(digitalPinToInterrupt(BUTTON1), button1_pressed, CHANGE);
	attachInterrupt(digitalPinToInterrupt(BUTTON2), button2_pressed, CHANGE);
}

void loop()
{
	delay(10);
	if (tile.MASTER_DIRECTION != 100)
	{
		if (data != lastData)
		{
			if (bitRead(data, 0) != bitRead(lastData, 0))
			{
				uint8_t value = bitRead(data, 0) ? 0 : 1;
				tile.sendData(TypeGeneral, 1, &value, 1);
			}
			if (bitRead(data, 1) != bitRead(lastData, 1))
			{
				uint8_t value = bitRead(data, 1) ? 2 : 3;
				tile.sendData(TypeGeneral, 1, &value, 1);
			}
			lastData = data;
		}
    if (tile.NETWORK_ID == 0 and (millis() > lastAskedForNetworkID + 750 or lastAskedForNetworkID == 0))
		{
			tile.sendData(TypeNeedNID, 1, {}, 0);
			lastAskedForNetworkID = millis();
		}
	}

	for (int direction = 0; direction < 4; direction++)
	{
		// *****FIND PARENT***** //
		if (tile.MASTER_DIRECTION == 100 and digitalRead(direction_pins[direction]) and (millis() > lastAskedForMaster[direction] + 100 or lastAskedForMaster[direction] == 0))
		{
			uint8_t value = ParentAreYou;
			tile.sendData(TypeParentManagement, 0, &value, 1, direction);
			lastAskedForMaster[direction] = millis();
		}
		// *****FIND PARENT***** //


		// *****REPORT NEIGHBOURS ON DISCONNECT***** //
		if (active_directions[direction] != 0 and !digitalRead(direction_pins[direction]))
		{
			active_directions[direction] = 0;
			active_children[direction] = false;
			active_new_tiles[direction] = false;
			if (direction == tile.MASTER_DIRECTION) {
				tile.MASTER_DIRECTION = 100;
				for (int i = 0; i < 4; i++) {
					if (active_children[i]) {
						uint8_t value = ParentGone;
						tile.sendData(TypeParentManagement, 0, &value, 1, i);
					}
					lastAskedForMaster[i] = 0;
				}
			}
			if (tile.NETWORK_ID != 0 and tile.MASTER_DIRECTION != 100)
			{
				tile.sendData(TypeReportNeighbours, 1, active_directions, 4);
			}
		}
		// *****REPORT NEIGHBOURS ON DISCONNECT***** //

		// *****HANDLE INCOMING MESSAGES***** //
		if (tile.available(direction) >= (active_message[direction] ? length[direction] : 4))
		{
			if (!active_message[direction])
			{
				type[direction] = tile.readByte(direction);
				if (!tile.validDataType(type[direction]))
				{
					return;
				}
				target[direction] = tile.readByte(direction);
				sender[direction] = tile.readByte(direction);
				length[direction] = tile.readByte(direction);
				active_message[direction] = true;
			}

			else
			{
				// Target is this tile
				if (target[direction] == tile.NETWORK_ID and tile.NETWORK_ID != 0)
				{
					if (type[direction] == TypeChangeHardwareID)
					{
						uint8_t buffer[length[direction]];
						tile.readData(buffer, length[direction], direction);
						tile.setUID(buffer[0], buffer[1], buffer[2], buffer[3]);
						uint8_t uid_type[5] = {tile.UID[0], tile.UID[1], tile.UID[2], tile.UID[3], TILETYPE};
						tile.sendData(TypeReportHID, 1, uid_type, 5);
					}
					else if (type[direction] == TypeTileCommand)
					{
						uint8_t command = tile.readByte(direction);
						if (command == CommandOnLights) {
							ledOn();
						}
						else if (command == CommandOffLights) {
							ledOff();
						}
						else if (command = CommandResetNID) {
							ledOn();
							tile.NETWORK_ID = 0;
							lastAskedForNetworkID = 0;
							for (int j = 0; j < 4; j++)
							{
								if (active_children[j])
								{
									tile.sendData(TypeTileCommand, target[direction], &command, 1, j, sender[direction]);
								}
								tile.sendData(TypeHereIsMyNID, 0, &tile.NETWORK_ID, 1, j);
							}
						}
					}
				}

				// Target is a new tile (ID=0) or me with no network id
				else if (target[direction] == 0)
				{
					if (type[direction] == TypeParentManagement) {
						uint8_t action = tile.readByte(direction);
						if (action == ParentAreYou)
						{
							if (tile.MASTER_DIRECTION != 100)
							{
								uint8_t value = ParentIam;
								tile.sendData(TypeParentManagement, 0, &value, 1, direction);
							}
							else
							{
								uint8_t value = ParentIamNot;
								tile.sendData(TypeParentManagement, 0, &value, 1, direction);
							}
						}
						else if (action == ParentIam)
						{
							if (tile.MASTER_DIRECTION == 100)
							{
								tile.setParent(direction);
								uint8_t value = ParentYouAre;
								tile.sendData(TypeParentManagement, 0, &value, 1, direction);
							}
						}
						else if (action == ParentYouAre) {
							if (tile.MASTER_DIRECTION != 100)
							{
								active_children[direction] = true;
							}
							else {
								uint8_t value = ParentGone;
								tile.sendData(TypeParentManagement, 0, &value, 1, direction);
							}
						}
						else if (action == ParentGone)
						{
							if (tile.MASTER_DIRECTION == direction) {
								tile.MASTER_DIRECTION = 100;
								for (int j = 0; j < 4; j++)
								{
									if (active_children[j]) {
										uint8_t value = ParentGone;
										tile.sendData(TypeParentManagement, 0, &value, 1, j);
									}
								}
							}
						}
					}

					else if (type[direction] == TypeHereIsYourNID and tile.NETWORK_ID == 0)
					{
						tile.NETWORK_ID = tile.readByte(direction);
						uint8_t uid_type[5] = {tile.UID[0], tile.UID[1], tile.UID[2], tile.UID[3], TILETYPE};
						tile.sendData(TypeReportHID, 1, uid_type, 5);
						ledOff();
						for (int j = 0; j < 4; j++)
						{
							tile.sendData(TypeHereIsMyNID, 0, &tile.NETWORK_ID, 1, j);
						}
					}
					else if (type[direction] == TypeHereIsMyNID and tile.NETWORK_ID != 0)
					{
						int id = tile.readByte(direction);
						if (id != 0 and id != active_directions[direction] and id != tile.NETWORK_ID)
						{
							tile.sendData(TypeHereIsMyNID, 0, &tile.NETWORK_ID, 1, direction);
							active_directions[direction] = id;
							tile.sendData(TypeReportNeighbours, 1, active_directions, 4);
						}
					}
					else
					{
						uint8_t buffer[length[direction]];
						tile.readData(buffer, length[direction], direction);

						for (int i = 0; i < 4; i++)
						{
							if (active_new_tiles[i])
							{
								if (type[direction] == TypeHereIsYourNID)
								{
									active_new_tiles[i] = false;
								}
								tile.sendData(type[direction], 0, buffer, length[direction], i, sender[direction]);
								break;
							}
						}
					}
				}

				// Target is master
				else if (target[direction] == 1)
				{
					uint8_t buffer[length[direction]];
					tile.readData(buffer, length[direction], direction);
					if (sender[direction] == 0)
					{
						active_new_tiles[direction] = true;
					}
					tile.sendData(type[direction], target[direction], buffer, length[direction], -1, sender[direction]);
				}

				// Broadcast message
				else if (target[direction] == 255)
				{
				}

				// Target is some other tile in network
				else
				{
					uint8_t buffer[length[direction]];
					tile.readData(buffer, length[direction], direction);
					for (int j = 0; j < 4; j++)
					{
						if (active_children[j]) {
							tile.sendData(type[direction], target[direction], buffer, length[direction], j, sender[direction]);
						}
					}
				}
				active_message[direction] = false;
			}
			// *****HANDLE INCOMING MESSAGES***** //
		}
	}
}

