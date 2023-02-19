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
    #define SUBUART_CHANNEL_1    0x00
    #define SUBUART_CHANNEL_2    0x01
    #define SUBUART_CHANNEL_3    0x02
    #define SUBUART_CHANNEL_4    0x03

    /* ERROR CODES */
    #define ERROR_WRITE               -1
    #define ERROR_TX_FIFO_FULL        -3
    #define ERROR_NO_BYTES_AVAIL      -4
    #define ERROR_NULL_POINTER        -5
    #define ERROR_NO_ACK              -6
    #define ERROR_BUFFER_LIMIT        -7
    #define ERROR_UNKNOWN_DIRECTION   -8

    /* Bytes */
    #define STARTBYTE   0b10101010


    T2T();
    uint8_t UID[4] = {0,0,0,0};
    int begin();
    int sendPing(int direction, bool zeroIndexed=true);
    int sendStartByte(int numberOfBytes, int direction, bool zeroIndexed=true);
    int sendData(const uint8_t *data, uint8_t size, int direction, bool zeroIndexed=true);
    int readData(uint8_t* buf, uint8_t size, int direction, bool zeroIndexed=true);
    int available(int direction, bool zeroIndexed=true);
    int writeUID(int direction, bool zeroIndexed=true);
    int writeByte(uint8_t value, int direction, bool zeroIndexed=true);
    int print(int value, int direction, bool zeroIndexed=true);
    int readByte(int direction, bool zeroIndexed=true);
    bool IDisZero();
    void setUID(int a, int b, int c, int d);

  private:
    void getUID();
    DFRobot_IICSerial getUART(int direction, bool zeroIndexed=true);
    bool validDirection(int direction, bool zeroIndexed=true);

};

#endif

