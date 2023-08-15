/*
  T2T.h - Library for communicating between two tiles
  Created by Finn Knauber, January 20, 2022
*/

#ifndef T2T_h
#define T2T_h

#include "Arduino.h"
#include "EEPROM.h"
#include <DFRobot_IICSerial.h>

class T2T
{
public:
/* Shortcuts for single digit integers */
#define SUBUART_CHANNEL_1 0x00
#define SUBUART_CHANNEL_2 0x01
#define SUBUART_CHANNEL_3 0x02
#define SUBUART_CHANNEL_4 0x03

/* ERROR CODES */
#define ERROR_WRITE -1
#define ERROR_TX_FIFO_FULL -3
#define ERROR_NO_BYTES_AVAIL -4
#define ERROR_NULL_POINTER -5
#define ERROR_NO_ACK -6
#define ERROR_BUFFER_LIMIT -7
#define ERROR_UNKNOWN_DIRECTION -8

/* Message Types */
#define TypeGeneral 0b00010001
#define TypeParentManagement 0b00100010
#define TypeHereIsMyNID 0b00110011
#define TypeNeedNID 0b01000100
#define TypeReportHID 0b01010101
#define TypeReportNeighbours 0b01100110
#define TypeHereIsYourNID 0b01110111
#define TypeChangeHardwareID 0b10001000
#define TypeTileCommand 0b10011001

#define ParentAreYou 1
#define ParentIam 2
#define ParentIamNot 3
#define ParentYouAre 4
#define ParentGone 5

#define CommandOnLights 1
#define CommandOffLights 2
#define CommandResetNID 3
#define CommandFactoryReset 4


  T2T();
  uint8_t UID[4] = {0, 0, 0, 0};
  uint8_t MASTER_DIRECTION = 100;
  uint8_t NETWORK_ID = 0;

  int begin();
  int setParent(int direction);
  int getParent();
  int available(int direction = -1);
  int sendData(uint8_t type, uint8_t target_network_id, const uint8_t *data, uint8_t size, int direction = -1, int sender = -1);
  int readData(uint8_t *buf, uint8_t size, int direction = -1);
  int peek(int direction = -1);
  int readByte(int direction = -1);
  int println(const String &value);
  int println(bool value);
  bool IDisZero();
  void setUID(int a, int b, int c, int d);
  int writeByte(uint8_t value, int direction);
  bool validDataType(uint8_t type);

private:
  void getUID();
  DFRobot_IICSerial getUART(int direction);
  bool validDirection(int direction);
};

#endif
