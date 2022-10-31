/*
  T2T.h - Library for communicating between two tiles
  Created by Finn Knauber, January 20, 2022
*/


#include "Arduino.h"
#include <DFRobot_IICSerial.h>
#include "T2T.h"

DFRobot_IICSerial left(Wire, SUBUART_CHANNEL_1, 0, 0);
DFRobot_IICSerial up(Wire, SUBUART_CHANNEL_2, 0, 0);
DFRobot_IICSerial right(Wire, SUBUART_CHANNEL_1, 1, 1);
DFRobot_IICSerial down(Wire, SUBUART_CHANNEL_2, 1, 1);


T2T::T2T() {
}

int T2T::begin() {
    left.begin(115200);
    up.begin(115200);
    right.begin(115200);
    down.begin(115200);
    delay(100);
    return 0;
}


int T2T::sendStartByte(int numberOfBytes, int direction, bool zeroIndexed) {
    return 0;
}

int T2T::sendPing(int direction, bool zeroIndexed) {
    return 0;
}

int T2T::sendData(const uint8_t *data, uint8_t size, int direction, bool zeroIndexed) {
    return 0;
}

int T2T::readData(uint8_t* buffer, uint8_t size, int direction, bool zeroIndexed) {
    return 0;
}

int T2T::available(int direction, bool zeroIndexed){
    if (validDirection(direction, zeroIndexed)) {
        return getUART(direction, zeroIndexed).available();
    }
    return ERROR_UNKNOWN_DIRECTION;
}

int T2T::writeByte(uint8_t value, int direction, bool zeroIndexed) {
    if (validDirection(direction, zeroIndexed)) {
        return getUART(direction, zeroIndexed).write(value);
    }
    return ERROR_UNKNOWN_DIRECTION;
}

int T2T::readByte(int direction, bool zeroIndexed) {
    if (validDirection(direction, zeroIndexed)) {
        return getUART(direction, zeroIndexed).read();
    }
    return ERROR_UNKNOWN_DIRECTION;
}

int T2T::writeUID(int direction, bool zeroIndexed) {
    if (validDirection(direction, zeroIndexed)) {
        for (int i=0; i<4; i++) {
            if (getUART(direction, zeroIndexed).write(UID[i]) == -1) {
                return -1;
            }
        }
        return 0;
    }
    return ERROR_UNKNOWN_DIRECTION;
}


DFRobot_IICSerial T2T::getUART(int direction, bool zeroIndexed) {
    if (!zeroIndexed) {
        direction--;
    }
    if (direction == SUBUART_CHANNEL_1) {
        return left;
    }
    else if (direction == SUBUART_CHANNEL_2) {
        return up;
    }
    else if (direction == SUBUART_CHANNEL_3) {
        return right;
    }
    else if (direction == SUBUART_CHANNEL_4) {
        return down;
    }
}


bool T2T::validDirection(int direction, bool zeroIndexed) {
    if (!zeroIndexed) {
        direction--;
    }
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