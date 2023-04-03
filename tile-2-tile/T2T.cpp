/*
  T2T.h - Library for communicating between two tiles
  Created by Finn Knauber, January 20, 2022
*/


#include "Arduino.h"
#include <DFRobot_IICSerial.h>
#include "EEPROM.h"
#include "T2T.h"

DFRobot_IICSerial up(Wire, SUBUART_CHANNEL_1, 0, 0);
DFRobot_IICSerial right(Wire, SUBUART_CHANNEL_2, 0, 0);
DFRobot_IICSerial down(Wire, SUBUART_CHANNEL_1, 1, 1);
DFRobot_IICSerial left(Wire, SUBUART_CHANNEL_2, 1, 1);


T2T::T2T() {
}

int T2T::begin() {
    up.begin(115200);
    right.begin(115200);
    down.begin(115200);
    left.begin(115200);

    NETWORK_ID = 0;

    getUID();
    delay(100);
    return 0;
}


int T2T::setParent(int direction) {
    if (validDirection(direction)) {
        MASTER_DIRECTION = direction;
    }
    return ERROR_UNKNOWN_DIRECTION;
}

int T2T::getParent() {
    return MASTER_DIRECTION;
}


int T2T::sendData(uint8_t type, uint8_t target_network_id, const uint8_t *data, uint8_t size, int direction, int sender) {
    if (direction==-1) {direction=MASTER_DIRECTION;}

    int res = 0;

    res = writeByte(type, direction);
    if (res!=0) {return res;}

    res = writeByte(target_network_id, direction);
    if (res!=0) {return res;}

    if (sender != -1) {
        res = writeByte(sender, direction);
    }
    else {
        res = writeByte(NETWORK_ID, direction);
    }
    if (res!=0) {return res;}

    res = writeByte(size, direction);
    if (res!=0) {return res;}

    for (int curByte=0; curByte<size; curByte++) {
        res = writeByte(data[curByte], direction);
        if (res!=0) {return res;}
    }
    // TODO CHECKSUM
    return 0;
}

int T2T::readData(uint8_t* buffer, uint8_t size, int direction) {
    if (direction==-1) {direction=MASTER_DIRECTION;}

    if (validDirection(direction)) {
        for (int i=0; i<size; i++) {
            buffer[i] = getUART(direction).read();
        }
    }
    return ERROR_UNKNOWN_DIRECTION;
}

int T2T::peek(int direction) {
    if (direction==-1) {direction=MASTER_DIRECTION;}

    if (validDirection(direction)) {
        return getUART(direction).peek();
    }
    return ERROR_UNKNOWN_DIRECTION;
}

int T2T::available(int direction){
    if (direction==-1) {direction=MASTER_DIRECTION;}

    if (validDirection(direction)) {
        return getUART(direction).available();
    }
    return ERROR_UNKNOWN_DIRECTION;
}

int T2T::println(const String &value) {
    if (validDirection(MASTER_DIRECTION)) {
        return getUART(MASTER_DIRECTION).println(value);
    }
    return ERROR_UNKNOWN_DIRECTION;
}

int T2T::println(bool value) {
    if (validDirection(MASTER_DIRECTION)) {
        return getUART(MASTER_DIRECTION).println(value);
    }
    return ERROR_UNKNOWN_DIRECTION;
}

int T2T::writeByte(uint8_t value, int direction) {
    if (validDirection(direction)) {
        return getUART(direction).write(value);
    }
    return ERROR_UNKNOWN_DIRECTION;
}

int T2T::readByte(int direction) {
    if (direction==-1) {direction=MASTER_DIRECTION;}

    if (validDirection(direction)) {
        return getUART(direction).read();
    }
    return ERROR_UNKNOWN_DIRECTION;
}

bool T2T::IDisZero() {
    for (int i=0; i<4; i++) {
        if(UID[i] != 0) {
            return false;
        }
    }
    return true;
}

void T2T::getUID() {
    UID[0] = EEPROM.read(1);
    UID[1] = EEPROM.read(2);
    UID[2] = EEPROM.read(3);
    UID[3] = EEPROM.read(4);
}

void T2T::setUID(int a, int b, int c, int d) {
    EEPROM.write(1, a);
    EEPROM.write(2, b);
    EEPROM.write(3, c);
    EEPROM.write(4, d);
    getUID();
}


DFRobot_IICSerial T2T::getUART(int direction) {
    if (direction == SUBUART_CHANNEL_1) {
        return up;
    }
    else if (direction == SUBUART_CHANNEL_2) {
        return right;
    }
    else if (direction == SUBUART_CHANNEL_3) {
        return down;
    }
    return left;
}


bool T2T::validDirection(int direction) {
    if (direction == SUBUART_CHANNEL_1) {
        return true;
    }
    else if (direction == SUBUART_CHANNEL_2) {
        return true;
    }
    else if (direction == SUBUART_CHANNEL_3) {
        return true;
    }
    else if (direction == SUBUART_CHANNEL_4) {
        return true;
    }
    return false;
}