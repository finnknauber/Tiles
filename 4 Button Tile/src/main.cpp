#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include <T2T.h>

T2T tile;

#define BUTTON1 PB12
#define BUTTON2 PB13
#define BUTTON3 PB14
#define BUTTON4 PB15
#define READ_UP PA1
#define READ_RIGHT PA3
#define READ_DOWN PA4
#define READ_LEFT PA5
#define TILETYPE 0x03

Adafruit_NeoPixel pixels(4, PA8, NEO_GRB + NEO_KHZ800);


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


uint8_t currentLEDValues[4][3];
uint8_t newLEDValues[4][3];
bool newLEDValuesAvailable = true;


void updateLEDValues(int led) {
	for (int i = 0; i < 3; i++) {
		if (currentLEDValues[led][i] > newLEDValues[led][i]) {
			currentLEDValues[led][i]--;
		}
		if (currentLEDValues[led][i] < newLEDValues[led][i]) {
			currentLEDValues[led][i]++;
		}
	}
}

void setNewLEDValuesAvailable() {
	newLEDValuesAvailable = false;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 3; j++) {
			if (currentLEDValues[i][j] != newLEDValues[i][j]) {
				newLEDValuesAvailable = true;
			}
		}
	}
}

void setNewLEDValues(int led, int r, int g, int b) {
	newLEDValues[led][0] = r;
	newLEDValues[led][1] = g;
	newLEDValues[led][2] = b;
	setNewLEDValuesAvailable();
}


void setLed(uint8_t led)
{
	pixels.setPixelColor(led, pixels.Color(currentLEDValues[led][0], currentLEDValues[led][1], currentLEDValues[led][2]));
	pixels.show();
}

void button1_pressed()
{
	if (tile.NETWORK_ID != 0)
	{
		bool value = digitalRead(BUTTON1);
		if (value) {
			data = data & 0b0;
		}
		else {
			data = data | 0b1;
		}
		setNewLEDValues(0, 0, value ? 0 : 255, 0);
	}
}

void button2_pressed()
{
	if (tile.NETWORK_ID != 0)
	{
		bool value = digitalRead(BUTTON2);
		setNewLEDValues(1, 0, value ? 0 : 255, 0);
		bitWrite(data, 1, !value);
	}
}

void button3_pressed()
{
	if (tile.NETWORK_ID != 0)
	{
		bool value = digitalRead(BUTTON3);
		setNewLEDValues(3, 0, value ? 0 : 255, 0);
		bitWrite(data, 2, !value);
	}
}

void button4_pressed()
{
	if (tile.NETWORK_ID != 0)
	{
		bool value = digitalRead(BUTTON4);
		setNewLEDValues(2, 0, value ? 0 : 255, 0);
		bitWrite(data, 3, !value);
	}
}

void setup()
{
	pinMode(BUTTON1, INPUT);
	pinMode(BUTTON2, INPUT);
	pinMode(BUTTON3, INPUT);
	pinMode(BUTTON4, INPUT);
	pinMode(READ_UP, INPUT);
	pinMode(READ_RIGHT, INPUT);
	pinMode(READ_DOWN, INPUT);
	pinMode(READ_LEFT, INPUT);
	pixels.begin();
	pixels.setBrightness(50);
	pixels.clear();
	pixels.show();
	for (int i = 0; i < 4; i++) {
		currentLEDValues[i][0] = 0;
		currentLEDValues[i][1] = 0;
		currentLEDValues[i][2] = 0;
	}
	tile.begin();


	attachInterrupt(digitalPinToInterrupt(BUTTON1), button1_pressed, CHANGE);
	attachInterrupt(digitalPinToInterrupt(BUTTON2), button2_pressed, CHANGE);
	attachInterrupt(digitalPinToInterrupt(BUTTON3), button3_pressed, CHANGE);
	attachInterrupt(digitalPinToInterrupt(BUTTON4), button4_pressed, CHANGE);
}


void loop()
{
	delay(10);
	if (newLEDValuesAvailable) {
		for (int led = 0; led < 4; led++) {
			updateLEDValues(led);
			setLed(led);
		}
		setNewLEDValuesAvailable();
	}

	if (tile.MASTER_DIRECTION != 100)
	{
		/*****SEND DATA*****/
		if (tile.NETWORK_ID != 0 and data != lastData)
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
			if (bitRead(data, 2) != bitRead(lastData, 2))
			{
				uint8_t value = bitRead(data, 2) ? 4 : 5;
				tile.sendData(TypeGeneral, 1, &value, 1);
			}
			if (bitRead(data, 3) != bitRead(lastData, 3))
			{
				uint8_t value = bitRead(data, 3) ? 6 : 7;
				tile.sendData(TypeGeneral, 1, &value, 1);
			}
			lastData = data;
		}
		/*****SEND DATA*****/

		/*****REQUEST NETWORK ID*****/
		if (tile.NETWORK_ID == 0 and lastAskedForNetworkID != -1 and (millis() > lastAskedForNetworkID + 750 or lastAskedForNetworkID == 0)) {
			for (int i = 0; i < 4; i++) {
				setNewLEDValues(i, 255, 0, 0);
			}
			tile.sendData(TypeNeedNID, 1, {}, 0);
			lastAskedForNetworkID = millis();
		}
		/*****REQUEST NETWORK ID*****/
	}

	else {
		if (millis() % 1200 < 300) {
			setNewLEDValues(0, 50, 0, 0);
			setNewLEDValues(1, 20, 0, 0);
			setNewLEDValues(2, 0, 0, 0);
			setNewLEDValues(3, 20, 0, 0);
		}
		else if (millis() % 1200 < 600) {
			setNewLEDValues(0, 20, 0, 0);
			setNewLEDValues(1, 50, 0, 0);
			setNewLEDValues(2, 20, 0, 0);
			setNewLEDValues(3, 0, 0, 0);
		}
		else if (millis() % 1200 < 900) {
			setNewLEDValues(0, 0, 0, 0);
			setNewLEDValues(1, 20, 0, 0);
			setNewLEDValues(2, 50, 0, 0);
			setNewLEDValues(3, 20, 0, 0);
		}
		else {
			setNewLEDValues(0, 20, 0, 0);
			setNewLEDValues(1, 0, 0, 0);
			setNewLEDValues(2, 20, 0, 0);
			setNewLEDValues(3, 50, 0, 0);
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
			// *****READ MESSAGE DATA***** //
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
			// *****READ MESSAGE DATA***** //


			else
			{
				// Target is this tile
				if (target[direction] == tile.NETWORK_ID and tile.NETWORK_ID != 0)
				{
					// Change Hardware ID
					if (type[direction] == TypeChangeHardwareID)
					{
						uint8_t buffer[length[direction]];
						tile.readData(buffer, length[direction], direction);
						tile.setUID(buffer[0], buffer[1], buffer[2], buffer[3]);
						uint8_t uid_type[5] = {tile.UID[0], tile.UID[1], tile.UID[2], tile.UID[3], TILETYPE};
						tile.sendData(TypeReportHID, 1, uid_type, 5);
					}
					// Tile Command
					else if (type[direction] == TypeTileCommand)
					{
						uint8_t command = tile.readByte(direction);
						if (command == CommandOnLights) {
							uint8_t buffer[12];
							tile.readData(buffer, length[direction], direction);

							for (int i = 0; i < 4; i++) {
								setNewLEDValues(i, buffer[i*3], buffer[i*3+1], buffer[i*3+2]);
							}
						}

						else if (command == CommandOffLights) {
							for (int i = 0; i < 4; i++) {
								setNewLEDValues(i, 0, 0, 0);
							}
						}
						else if (command == CommandResetNID) {
							for (int i = 0; i < 4; i++) {
								setNewLEDValues(i, 255, 0, 0);
							}

							tile.NETWORK_ID = 0;
							lastAskedForNetworkID = 0;
							for (int j = 0; j < 4; j++)
							{
								tile.sendData(TypeHereIsMyNID, 0, &tile.NETWORK_ID, 1, j);
							}
						}
						else if (command == CommandFactoryReset) {
							for (int i = 0; i < 4; i++) {
								setNewLEDValues(i, 255, 35, 0);
							}

							tile.setUID(0, 0, 0, 0);
							tile.NETWORK_ID = 0;
							lastAskedForNetworkID = -1;
							for (int j = 0; j < 4; j++)
							{
								tile.sendData(TypeHereIsMyNID, 0, &tile.NETWORK_ID, 1, j);
							}
						}
					}
				}

				// Target is new tile (this tile or other)
				else if (target[direction] == 0)
				{
					// *****PARENT MANAGEMENT***** //
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
						// *****PARENT MANAGEMENT***** //
					}

					// Get Network ID assigned
					else if (type[direction] == TypeHereIsYourNID and tile.NETWORK_ID == 0)
					{
						tile.NETWORK_ID = tile.readByte(direction);
						uint8_t uid_type[5] = {tile.UID[0], tile.UID[1], tile.UID[2], tile.UID[3], TILETYPE};
						tile.sendData(TypeReportHID, 1, uid_type, 5);
						for (int i = 0; i < 4; i++) {
							for (int i = 0; i < 4; i++) {
								setNewLEDValues(i, 0, 0, 0);
							}
						}
						for (int j = 0; j < 4; j++)
						{
							tile.sendData(TypeHereIsMyNID, 0, &tile.NETWORK_ID, 1, j);
						}
						tile.sendData(TypeReportNeighbours, 1, active_directions, 4);
					}

					// Share Network ID with neighbours
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

					else {
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

