/*
  M2M.h - Library for communicating between two modules
  Created by Finn Knauber, January 20, 2022
*/

#include "Arduino.h"
#include "T2T.h"


T2T::T2T(TwoWire &wire) {
    _iicWire = &wire;
}

int T2T::begin() {
    _iicWire->begin();
    delay(5);
    _setBaudRate();
    delay(15);
    return 0;

    _setRegisterPage(SUBUART_CHANNEL_1, SPAGE0);
    _setRegisterPage(SUBUART_CHANNEL_3, SPAGE0);
    // printDebug();

    int8_t globalRegs = _getGlobalRegisters();
    if (globalRegs != 0) {
       return globalRegs;
    }
    // write GENA register (set clock enable to true)
    // _setRegister(SUBUART_CHANNEL_1, REGISTER_GENA, 0x03);
    // _setRegister(SUBUART_CHANNEL_3, REGISTER_GENA, 0x03);

    // set SIER
    uint8_t value;
    _readRegister(SUBUART_CHANNEL_1, REGISTER_SIER, &value);
    value |= 0x8F;
    _writeRegister(SUBUART_CHANNEL_1, REGISTER_SIER, value);
    // write FCR (FIFO behaviour)
    _writeRegister(SUBUART_CHANNEL_1, REGISTER_FCR, 0x0E);
    // write SCR (Sleep and transmit enable)
    _readRegister(SUBUART_CHANNEL_1, REGISTER_SCR, &value);
    bitClear(value, 2);
    value |= 0x03;
    _writeRegister(SUBUART_CHANNEL_1, REGISTER_SCR, value);

    // write LCR
    uint8_t lcr;
    _readRegister(SUBUART_CHANNEL_1, REGISTER_LCR, &lcr);
    value = lcr >> 6;
    _writeRegister(SUBUART_CHANNEL_1, REGISTER_LCR, value << 6);

    // set SIER
    _readRegister(SUBUART_CHANNEL_3, REGISTER_SIER, &value);
    value |= 0x8F;
    _writeRegister(SUBUART_CHANNEL_3, REGISTER_SIER, value);
    // write FCR (FIFO behaviour)
    _writeRegister(SUBUART_CHANNEL_3, REGISTER_FCR, 0x0E);
    // write SCR (Sleep and transmit enable)
    _readRegister(SUBUART_CHANNEL_3, REGISTER_SCR, &value);
    bitClear(value, 2);
    value |= 0x03;
    _writeRegister(SUBUART_CHANNEL_3, REGISTER_SCR, value);
    // write LCR
    _readRegister(SUBUART_CHANNEL_3, REGISTER_LCR, &lcr);
    value = lcr >> 6;
    _writeRegister(SUBUART_CHANNEL_3, REGISTER_LCR, value << 6);

    delay(10);
    return 0;
}

int T2T::sendStartByte(int numberOfBytes, int direction, bool zeroIndexed) {
    return 0;
}

int T2T::sendPing(int direction, bool zeroIndexed) {
    _setRegisterPage(SUBUART_CHANNEL_1, SPAGE1);
    uint8_t value = 0;
    if (zeroIndexed) {
        _readRegister(SUBUART_CHANNEL_3, direction, &value);
    }
    else {
        _readRegister(SUBUART_CHANNEL_1, direction, &value);
    }
    _setRegisterPage(SUBUART_CHANNEL_1, SPAGE0);
    return value;
}

int T2T::sendData(uint8_t uid[4], const uint8_t *data, uint8_t size, int direction, bool zeroIndexed) {
    if (!zeroIndexed) {
        direction--;
    }
    if (data == NULL) {
        return ERROR_NULL_POINTER;
    }
    if (size > 250) {
        return ERROR_BUFFER_LIMIT;
    }
    uint8_t fifoStateReg;
    _readRegister(direction, REGISTER_FSR, &fifoStateReg);
    fifoStateReg &= 0x02;
    if (fifoStateReg >> 1 == 1) {
        return ERROR_TX_FIFO_FULL;
    }

    _address = _getAddress(direction, FIFO_ACCESS);
    _iicWire->beginTransmission(_address);
    _iicWire->write(uid, 4);
    _iicWire->write(size);
    for (int byte=0; byte<size; byte++) {
        _iicWire->write(data[byte]);
    }
    if(_iicWire->endTransmission() != 0){
        return ERROR_NO_ACK;
    }
    return 0;
}

int T2T::readData(uint8_t* buffer, uint8_t size, int direction, bool zeroIndexed) {
    if (!zeroIndexed) {
        direction--;
    }
    if(buffer == NULL){
        return 0;
    }
    if (available(direction) < size) {
        return 0;
    }

    uint8_t *_pBuf = (uint8_t *)buffer;
    _address = _getAddress(direction, FIFO_ACCESS);
    _iicWire->beginTransmission(_address);
    if(_iicWire->endTransmission() != 0){
        return 0;
    }

    _iicWire->requestFrom(_address, size);
    for(size_t i = 0; i < size; i++){
        buffer[i] = (uint8_t) _iicWire->read();
    }
    return size;
}

int T2T::available(int direction, bool zeroIndexed){
    if (!zeroIndexed) {
        direction--;
    }
    uint8_t val;
    if(_readRegister(direction, REGISTER_RFCNT, &val) != 0){
        return 0;
    }
    int index = (int)val;
    if(index == 0) {
        uint8_t fifoStateReg;
        _readRegister(direction, REGISTER_FSR, &fifoStateReg);
        fifoStateReg &= 0x08;
        if(fifoStateReg >> 3 == 1){
            index = 256;
        }
    }
    return index;
}

int T2T::writeByte(uint8_t value, int direction, bool zeroIndexed) {
    if (!zeroIndexed) {
        direction--;
    }
    uint8_t fifoStateReg;
    _readRegister(direction, REGISTER_FSR, &fifoStateReg);
    fifoStateReg &= 0x02;
    if (fifoStateReg >> 1 == 1) {
        return ERROR_TX_FIFO_FULL;
    }
    _writeRegister(direction, REGISTER_FDAT, value);
    return 0;
}

int T2T::readByte(int direction, bool zeroIndexed) {
    uint8_t value;
    if (!zeroIndexed) {
        direction--;
    }

    if (!available(direction)) {
        return ERROR_NO_BYTES_AVAIL;
    }
    if (_readRegister(direction, REGISTER_FDAT, &value) != 0) {
        return ERROR_READ_REG;
    }
    return value;
}

void T2T::writeUID(int direction, bool zeroIndexed) {
    if (!zeroIndexed) {
        direction--;
    }
    uint8_t currentUID[4] = {255, 255, 255, 255};
    uint8_t newUID[4] = {2,2,2,2};
    // uint8_t newUID[4] = {random(0, 255), random(0, 255), random(0, 255), random(0, 255)};

    uint8_t fifoStateReg;
    _readRegister(direction, REGISTER_FSR, &fifoStateReg);
    fifoStateReg &= 0x02;
    if (fifoStateReg >> 1 == 1) {
        return;
    }

    _address = _getAddress(direction, FIFO_ACCESS);
    _iicWire->beginTransmission(_address);
    _iicWire->write(currentUID, 4);
    for (int byte=0; byte<4; byte++) {
        _iicWire->write(newUID[byte]);
    }
    _iicWire->endTransmission();
}



int8_t T2T::_getGlobalRegisters() {
    uint8_t GENA_value;
    _readRegister(SUBUART_CHANNEL_1, REGISTER_GENA, &GENA_value);
    // if no acknowledgment from chip register can not be read and begin process is stopped
    if (GENA_value == -2) {
        return ERROR_READ_REG;
    }
    // if first bit in gena register is not set the chip won't work properly
    if ((GENA_value & 0x80) == 0) {
        return ERROR_REG_DATA;
    }

    _readRegister(SUBUART_CHANNEL_3, REGISTER_GENA, &GENA_value);
    // if no acknowledgment from chip register can not be read and begin process is stopped
    if (GENA_value == ERROR_READ_REG) {
        return ERROR_READ_REG;
    }
    // if first bit in gena register is not set the chip won't work properly
    if ((GENA_value & 0x80) == 0) {
        return ERROR_REG_DATA;
    }
    return 0;
}

void T2T::_setBaudRate() {
    _setRegisterPage(SUBUART_CHANNEL_1, SPAGE1);
    // set BAUD1
    _writeRegister(SUBUART_CHANNEL_1, REGISTER_BAUD1, 0x00);
    // set BAUD0
    _writeRegister(SUBUART_CHANNEL_1, REGISTER_BAUD0, 0x07);
    _setRegisterPage(SUBUART_CHANNEL_1, SPAGE0);

    // // same thing for second IC
    // _setRegisterPage(SUBUART_CHANNEL_3, SPAGE1);
    // // set BAUD1
    // _writeRegister(SUBUART_CHANNEL_3, REGISTER_BAUD1, 0x00);
    // // set BAUD0
    // _writeRegister(SUBUART_CHANNEL_3, REGISTER_BAUD0, 0x07);
    // _setRegisterPage(SUBUART_CHANNEL_3, SPAGE0);
}


void T2T::_setRegister(uint8_t channel, uint8_t register_address, uint8_t value) {
    uint8_t prev_value;
    _readRegister(channel, register_address, &prev_value);
    value |= prev_value;
    _writeRegister(channel, register_address, value);
}

uint8_t T2T::_setRegisterPage(uint8_t channel, uint8_t pageNumber) {
    uint8_t val;
    uint8_t page;
    _readRegister(channel, REGISTER_SPAGE, &val);
    if (pageNumber == 1) {
        val |= 0x01;
        page = 1;
    }
    else {
        val &= 0xFE;
        page = 0;
    }
    _writeRegister(channel, REGISTER_SPAGE, val);
    return page;
}

void T2T::_writeRegister(uint8_t channel, uint8_t register_address, uint8_t value) {
    _address = _getAddress(channel, REGISTER_ACCESS);
    _iicWire->beginTransmission(_address);
    _iicWire->write(&register_address, 1);
    _iicWire->write(value);
    _iicWire->endTransmission();
}

int8_t T2T::_readRegister(uint8_t channel, uint8_t register_address, uint8_t *buffer) {
    _address = _getAddress(channel, REGISTER_ACCESS);
    _address &= 0xFE;
    _iicWire->beginTransmission(_address);
    _iicWire->write(&register_address, 1);
    if (_iicWire->endTransmission() != 0) {
        return ERROR_READ_REG;
    }
    _iicWire->requestFrom(_address, (uint8_t) 1);
    *buffer = (char)_iicWire->read();
    return 0;
}


uint8_t T2T::_getAddress(uint8_t channel, uint8_t operation) {
    uint8_t _newAddr = 0b00100000;
    if (channel > SUBUART_CHANNEL_2) {
        _newAddr = 0b11100000;
    }
    if (channel % 2 != 0) {
        bitSet(_newAddr, 2);
    }
    bitWrite(_newAddr, 1, operation);
    return _newAddr>>1;
}


void T2T::printDebug() {
    uint8_t debug_value;
    Serial.println("Channel 1+2");
    Serial.print("GENA: ");
    _readRegister(SUBUART_CHANNEL_1, REGISTER_GENA, &debug_value);
    Serial.println(debug_value, BIN);
    Serial.print("GRST: ");
    _readRegister(SUBUART_CHANNEL_1, REGISTER_GRST, &debug_value);
    Serial.println(debug_value, BIN);
    Serial.print("GIER: ");
    _readRegister(SUBUART_CHANNEL_1, REGISTER_GIER, &debug_value);
    Serial.println(debug_value, BIN);
    _setRegisterPage(SUBUART_CHANNEL_1, 1);
    Serial.print("BAUD1: ");
    _readRegister(SUBUART_CHANNEL_1, REGISTER_BAUD1, &debug_value);
    Serial.println(debug_value, BIN);
    Serial.print("BAUD0: ");
    _readRegister(SUBUART_CHANNEL_1, REGISTER_BAUD0, &debug_value);
    Serial.println(debug_value, BIN);
    _setRegisterPage(SUBUART_CHANNEL_1, 0);
    Serial.print("SIER: ");
    _readRegister(SUBUART_CHANNEL_1, REGISTER_SIER, &debug_value);
    Serial.println(debug_value, BIN);
    Serial.print("FCR: ");
    _readRegister(SUBUART_CHANNEL_1, REGISTER_FCR, &debug_value);
    Serial.println(debug_value, BIN);
    Serial.print("SCR: ");
    _readRegister(SUBUART_CHANNEL_1, REGISTER_SCR, &debug_value);
    Serial.println(debug_value, BIN);
    Serial.print("LCR: ");
    _readRegister(SUBUART_CHANNEL_1, REGISTER_LCR, &debug_value);
    Serial.println(debug_value, BIN);


    Serial.println("Channel 3+4");
    Serial.print("GENA: ");
    _readRegister(SUBUART_CHANNEL_3, REGISTER_GENA, &debug_value);
    Serial.println(debug_value, BIN);
    Serial.print("GRST: ");
    _readRegister(SUBUART_CHANNEL_3, REGISTER_GRST, &debug_value);
    Serial.println(debug_value, BIN);
    Serial.print("GIER: ");
    _readRegister(SUBUART_CHANNEL_3, REGISTER_GIER, &debug_value);
    Serial.println(debug_value, BIN);
    _setRegisterPage(SUBUART_CHANNEL_3, 1);
    Serial.print("BAUD1: ");
    _readRegister(SUBUART_CHANNEL_3, REGISTER_BAUD1, &debug_value);
    Serial.println(debug_value, BIN);
    Serial.print("BAUD0: ");
    _readRegister(SUBUART_CHANNEL_3, REGISTER_BAUD0, &debug_value);
    Serial.println(debug_value, BIN);
    _setRegisterPage(SUBUART_CHANNEL_3, 0);
    Serial.print("SIER: ");
    _readRegister(SUBUART_CHANNEL_3, REGISTER_SIER, &debug_value);
    Serial.println(debug_value, BIN);
    Serial.print("FCR: ");
    _readRegister(SUBUART_CHANNEL_3, REGISTER_FCR, &debug_value);
    Serial.println(debug_value, BIN);
    Serial.print("SCR: ");
    _readRegister(SUBUART_CHANNEL_3, REGISTER_SCR, &debug_value);
    Serial.println(debug_value, BIN);
    Serial.print("LCR: ");
    _readRegister(SUBUART_CHANNEL_3, REGISTER_LCR, &debug_value);
    Serial.println(debug_value, BIN);
}