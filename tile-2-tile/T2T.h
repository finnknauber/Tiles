/*
  M2M.h - Library for communicating between two modules
  Created by Finn Knauber, January 20, 2022
*/
#ifndef T2T_h
#define T2T_h

#include "Arduino.h"
#include <Wire.h>
#include "EEPROM.h"

/* Addresses */
#define REGISTER_ACCESS      0x00
#define FIFO_ACCESS          0x01
#define SPAGE0               0x00
#define SPAGE1               0x01

/* Shortcuts for register access */
#define REGISTER_GENA        0x00
#define REGISTER_GRST        0x01
#define REGISTER_GIER        0x10
// Page control register
#define REGISTER_SPAGE       0x03
// SPAGE 0
#define REGISTER_SCR         0x04
#define REGISTER_LCR         0x05
#define REGISTER_FCR         0x06
#define REGISTER_SIER        0x07
#define REGISTER_RFCNT       0x0A
#define REGISTER_FSR         0x0B
#define REGISTER_FDAT        0x0D
// SPAGE 1
#define REGISTER_BAUD1       0x04
#define REGISTER_BAUD0       0x05
#define REGISTER_PRES        0x06


class T2T
{
  public:
    /* Shortcuts for single digit integers */
    #define SUBUART_CHANNEL_1    0x00
    #define SUBUART_CHANNEL_2    0x01
    #define SUBUART_CHANNEL_3    0x02
    #define SUBUART_CHANNEL_4    0x03

    /* ERROR CODES */
    #define ERROR_READ_REG       -1
    #define ERROR_REG_DATA       -2
    #define ERROR_TX_FIFO_FULL   -3
    #define ERROR_NO_BYTES_AVAIL -4
    #define ERROR_NULL_POINTER   -5
    #define ERROR_NO_ACK         -6
    #define ERROR_BUFFER_LIMIT   -7

    T2T(TwoWire &wire = Wire);
    int begin();
    int sendData(uint8_t uid[4], const uint8_t *data, uint8_t size, int direction, bool zeroIndexed=true);
    int writeByte(uint8_t value, int direction, bool zeroIndexed=true);
    int readByte(int direction, bool zeroIndexed=true);
    int available(int direction, bool zeroIndexed=true);
    int readData(uint8_t* buf, uint8_t size, int direction, bool zeroIndexed=true);
    void writeUID(int direction, bool zeroIndexed=true);

    int sendStartByte(int numberOfBytes, int direction, bool zeroIndexed=true);
    int sendPing(int direction, bool zeroIndexed=true);
    void printDebug();


  private:
    TwoWire *_iicWire;
    uint8_t _address;
    int8_t _getGlobalRegisters();
    void _setBaudRate();
    uint8_t _getAddress(uint8_t channel, uint8_t operation);
    void _setRegister(uint8_t channel, uint8_t register_address, uint8_t value);
    void _writeRegister(uint8_t channel, uint8_t register_address, uint8_t value);
    int8_t _readRegister(uint8_t channel, uint8_t register_address, uint8_t *buffer);
    uint8_t _setRegisterPage(uint8_t channel, uint8_t pageNumber);
};

#endif

