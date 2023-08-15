#include <Arduino.h>
#include <HardwareSerial.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(PB0, PA4);
HardwareSerial Serial3(PB7, PB6);

#define LED PC13
#define TILETYPE 0x02
#define direction_pin PA3

#define TypeGeneral 0b00010001
#define TypeIAmMaster 0b00100010
#define TypeIAmNotMaster 0b00110011
#define TypeAreYouMaster 0b01000100
#define TypeNeedNID 0b01010101
#define TypeReportHID 0b01100110
#define TypeHereIsMyNID 0b10001000
#define TypeHereIsYourNID 0b10101010
#define TypeReportNeighbours 0b10011001
#define TypeChangeHardwareID 0b10111011
#define TypeResetNetworkID 0b11001100

uint32_t lastAskedForMaster = 0;
uint32_t lastAskedForNetworkID = 0;
byte lastData = 0;
byte data = 0;

int active_direction = 0;
bool active_message = false;
uint8_t type = 0;
uint8_t target = 0;
uint8_t sender = 0;
uint8_t length = 0;

uint8_t MASTER_DIRECTION = 100;
uint8_t NETWORK_ID = 0;
int UID[4] = {99, 218, 215, 102};

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

void sendData(uint8_t type, uint8_t target_network_id, const uint8_t *data, uint8_t size, int sender = -1)
{
  Serial3.write(type);
  Serial3.write(target_network_id);
  if (sender != -1)
  {
    Serial3.write(sender);
  }
  else
  {
    Serial3.write(NETWORK_ID);
  }

  Serial3.write(size);

  for (int curByte = 0; curByte < size; curByte++)
  {
    Serial3.write(data[curByte]);
  }
  // TODO CHECKSUM
}

bool is_valid_data_type(uint8_t type)
{
  int data_types[] = {TypeGeneral, TypeIAmMaster, TypeIAmNotMaster, TypeAreYouMaster, TypeNeedNID, TypeReportHID, TypeHereIsMyNID, TypeHereIsYourNID, TypeReportNeighbours, TypeChangeHardwareID, TypeResetNetworkID};
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
  Serial3.begin(115200);
  pinMode(LED, OUTPUT);
  pinMode(direction_pin, INPUT_PULLDOWN);
  radio.begin();
  radio.openReadingPipe(0, 0x07);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();
  ledOn();
  delay(100);
  if (digitalRead(direction_pin))
  {
    sendData(TypeAreYouMaster, 0, {}, 0);
    lastAskedForMaster = millis();
  }

  delay(5);
}

void loop()
{
  delay(15);
  if (radio.available())
  {
    ledOn();
    byte data[1] = {};
    radio.read(&data, 1);
    if (MASTER_DIRECTION != 100 and NETWORK_ID != 0)
    {
      uint8_t toSend = data[0];
      sendData(TypeGeneral, 1, &toSend, 1);
    }
    delay(15);
    ledOff();
  }

  if (MASTER_DIRECTION != 100)
  {
    // TODO only asks if tile is older than 250 milliseconds
    if (NETWORK_ID == 0 and millis() > lastAskedForNetworkID + 750)
    {
      sendData(TypeNeedNID, 1, {}, 0);
      lastAskedForNetworkID = millis();
    }
  }

  if (MASTER_DIRECTION == 100)
  {
    if (millis() > lastAskedForMaster + 100)
    {
      if (digitalRead(direction_pin))
      {
        sendData(TypeAreYouMaster, 0, {}, 0);
        lastAskedForMaster = millis();
      }
    }
  }

  // *****REPORT NEIGHBOURS***** //
  if (!digitalRead(direction_pin) and active_direction != 0)
  {
    active_direction = 0;
    if (NETWORK_ID != 0 and MASTER_DIRECTION != 100)
    {
      uint8_t data_to_send[4] = {active_direction, 0, 0, 0};
      sendData(TypeReportNeighbours, 1, data_to_send, 4);
    }
  }
  // *****REPORT NEIGHBOURS***** //

  // *****HANDLE INCOMING MESSAGES***** //
  if (Serial3.available() >= (active_message ? length : 4))
  {
    if (!active_message)
    {
      type = Serial3.read();
      if (!is_valid_data_type(type))
      {
        return;
      }
      target = Serial3.read();
      sender = Serial3.read();
      length = Serial3.read();
      active_message = true;
    }
    else
    {
      // Target is this tile
      if (target == NETWORK_ID and NETWORK_ID != 0)
      {
        if (type == TypeChangeHardwareID)
        {
          uint8_t buffer[length];
          Serial3.readBytes(buffer, length);
          UID[0] = buffer[0];
          UID[1] = buffer[1];
          UID[2] = buffer[2];
          UID[3] = buffer[3];
          uint8_t uid_type[5] = {UID[0], UID[1], UID[2], UID[3], TILETYPE};
          sendData(TypeReportHID, 1, uid_type, 5);
        }
        else if (type == TypeResetNetworkID)
        {
          ledOn();
          NETWORK_ID = 0;
          sendData(TypeHereIsMyNID, 0, &NETWORK_ID, 1);
        }
      }
      // Target is a new tile (ID=0) or me with no network id
      else if (target == 0)
      {
        if (type == TypeAreYouMaster)
        {
          if (MASTER_DIRECTION != 100)
          {
            sendData(TypeIAmMaster, 0, {}, 0);
          }
          else
          {
            sendData(TypeIAmNotMaster, 0, {}, 0);
          }
        }
        else if (type == TypeIAmMaster)
        {
          if (MASTER_DIRECTION == 100)
          {
            MASTER_DIRECTION = 0;
          }
        }
        else if (type == TypeIAmNotMaster)
        {
        }
        else if (type == TypeHereIsYourNID and NETWORK_ID == 0)
        {
          NETWORK_ID = Serial3.read();
          uint8_t uid_type[5] = {UID[0], UID[1], UID[2], UID[3], TILETYPE};
          sendData(TypeReportHID, 1, uid_type, 5);
          ledOff();
          sendData(TypeHereIsMyNID, 0, &NETWORK_ID, 1);
        }
        else if (type == TypeHereIsMyNID and NETWORK_ID != 0)
        {
          int id = Serial3.read();
          if (id != 0 and id != active_direction and id != NETWORK_ID)
          {
            sendData(TypeHereIsMyNID, 0, &NETWORK_ID, 1);
            active_direction = id;
            uint8_t data_to_send[4] = {active_direction, 0, 0, 0};
            sendData(TypeReportNeighbours, 1, data_to_send, 4);
          }
        }
        else
        {
        }
      }
      // Target is master
      else if (target == 1)
      {
      }
      // Broadcast message
      else if (target == 255)
      {
      }
      // Target is some other tile in network
      else
      {
      }
      active_message = false;
    }
    // *****HANDLE INCOMING MESSAGES***** //
  }
}
