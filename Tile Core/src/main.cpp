#include <Arduino.h>
#include "HID-Project.h"
uint8_t rawhidData[255];
byte megabuff[64];

#define TypeGeneral           0b00010001
#define TypeACK               0b00100010
#define TypeNACK              0b00110011
#define TypeAreYouMaster      0b01000100
#define TypeNeedNID           0b01010101
#define TypeReportHID         0b01100110
#define TypeGiveMeYourNID     0b01110111
#define TypeHereIsMyNID       0b10001000
#define TypeHereIsYourNID     0b10101010
#define TypeReportNeighbours  0b10011001
#define TypeChangeHardwareID  0b10111011


bool active_message = false;
int message_type = 0;

void sendData(uint8_t type, uint8_t target_network_id, const int *data, uint8_t size) {
    while (!Serial1.availableForWrite()) {}
    Serial1.write(type);
    while (!Serial1.availableForWrite()) {}
    Serial1.write(target_network_id);
    while (!Serial1.availableForWrite()) {}
    Serial1.write(1);
    while (!Serial1.availableForWrite()) {}
    Serial1.write(size);

    for (int curByte=0; curByte<size; curByte++) {
        while (!Serial1.availableForWrite()) {}
        Serial1.write(data[curByte]);
    }
    // TODO Checksum
}

void resetBuf() {
    for (int i = 0; i < sizeof(megabuff); i++) {
        megabuff[i] = 0;
    }
}


void setup() {
    randomSeed(analogRead(A1));
    pinMode(0, INPUT_PULLUP);
    Serial.begin(115200);
    Serial1.begin(115200);
    RawHID.begin(rawhidData, sizeof(rawhidData));
    for (int i = 0; i < sizeof(megabuff); i++) {
        megabuff[i] = 0;
    }
    // megabuff[0] = data;
    // RawHID.write(megabuff, sizeof(megabuff));
    delay(10);
    Serial.println("Starting");
}


void loop() {
    if (Serial1.available()) {
        int data = Serial1.read();
        if (data == TypeAreYouMaster) {
            Serial.println("Are you Master?");
            sendData(TypeACK, 0, {}, 0);
            while (Serial1.available()) {
                Serial1.read();
            }
            delay(10);
        }

        else if (data == TypeNeedNID) {
            Serial.print("Requested Network ID, Sending ");
            // Target is Core
            if (Serial1.read() == 1) {
                // Sender has Network id 0
                if (Serial1.read() == 0) {
                    // Length 0
                    Serial1.read();
                    megabuff[0] = TypeNeedNID;
                    megabuff[1] = 1;
                    megabuff[2] = 0;
                    megabuff[3] = 0;
                    RawHID.write(megabuff, sizeof(megabuff));
                    resetBuf();
                    while (RawHID.available() < 64) {delay(5);}
                    int newAdress = random(2, 255);
                    int data = RawHID.read();
                    if (data == TypeHereIsYourNID) {
                        // target, sender, length
                        RawHID.read();
                        RawHID.read();
                        RawHID.read();
                        newAdress = RawHID.read();
                        while (RawHID.available()) {
                            RawHID.read();
                        }
                    }
                    Serial.println(newAdress);
                    sendData(TypeHereIsYourNID, 0, &newAdress, 1);
                }
            } 
        }

        else if (data == TypeGeneral) {
            while (Serial1.available() < 4) {}
            if (Serial1.read() == 1) {
                int sender = Serial1.read();
                int length = Serial1.read();
                megabuff[0] = TypeGeneral;
                megabuff[1] = 1;
                megabuff[2] = sender;
                megabuff[3] = length;
                for (int i=0; i<length; i++) {
                    while (!Serial1.available()) {}
                    Serial.print(sender);
                    Serial.print(": ");
                    data = Serial1.read();
                    switch (data) {
                        case 0:
                            Serial.println("Button Pressed!");
                            break;

                        case 1:
                            Serial.println("Button Released!");
                            break;
                        
                        case 2:
                            Serial.println("Encoder Turned Right!");
                            break;
                        
                        case 3:
                            Serial.println("Encoder Turned Left!");
                            break;

                        default:
                            Serial.print("Not recognized: ");
                            Serial.println(data);
                            break;
                    }
                    megabuff[4+i] = data;
                }
                RawHID.write(megabuff, sizeof(megabuff));
                resetBuf();
            }
        }

        else if (data == TypeReportHID) {
            while (Serial1.available() < 3) {}
            if (Serial1.read() == 1) {
                int sender = Serial1.read();
                int length = Serial1.read();
                while (Serial1.available() < length) {}
                Serial.print("HID Reported, ");
                Serial.print(sender);
                Serial.print(": ");
                uint8_t id[] = {Serial1.read(), Serial1.read(), Serial1.read(), Serial1.read()};
                Serial.print(id[0]);
                Serial.print(".");
                Serial.print(id[1]);
                Serial.print(".");
                Serial.print(id[2]);
                Serial.print(".");
                Serial.println(id[3]);
                megabuff[0] = TypeReportHID;
                megabuff[1] = 1;
                megabuff[2] = sender;
                megabuff[3] = 4;
                megabuff[4] = id[0];
                megabuff[5] = id[1];
                megabuff[6] = id[2];
                megabuff[7] = id[3];
                if (id[0] == 0 and id[1] == 0 and id[2] == 0 and id[3] == 0) {
                    Serial.print("Invalid HID, sending new: ");
                    int new_id[] = {random(256), random(256), random(256), random(256)};
                    Serial.print(new_id[0]);
                    Serial.print(".");
                    Serial.print(new_id[1]);
                    Serial.print(".");
                    Serial.print(new_id[2]);
                    Serial.print(".");
                    Serial.println(new_id[3]);
                    sendData(TypeChangeHardwareID, sender, new_id, 4);
                    megabuff[4] = new_id[0];
                    megabuff[5] = new_id[1];
                    megabuff[6] = new_id[2];
                    megabuff[7] = new_id[3];
                }
                else {
                    sendData(TypeACK, sender, {}, 0);
                }
                RawHID.write(megabuff, sizeof(megabuff));
                resetBuf();
            }
        }

        else if (data == TypeGiveMeYourNID) {
            while (Serial1.available() < 3) {}
            // target, sender, length
            Serial1.read();
            int sender = Serial1.read();
            Serial1.read();
            Serial.print(sender);
            Serial.println(" asked what my network ID is, responding 1");
            int id = 1;
            sendData(TypeHereIsMyNID, sender, &id, 1);
        }

        else if (data == TypeReportNeighbours) {
            while (Serial1.available() < 3) {}
            // target, sender, length
            Serial1.read();
            int sender = Serial1.read();
            Serial1.read();
            Serial.print("Neighbours of ");
            Serial.print(sender);
            Serial.print(": ");
            while (Serial1.available() < 4) {}
            uint8_t Neighbours[] = {Serial1.read(), Serial1.read(), Serial1.read(), Serial1.read()};
            Serial.print(Neighbours[0]);
            Serial.print(", ");
            Serial.print(Neighbours[1]);
            Serial.print(", ");
            Serial.print(Neighbours[2]);
            Serial.print(", ");
            Serial.println(Neighbours[3]);
            megabuff[0] = TypeReportNeighbours;
            megabuff[1] = 1;
            megabuff[2] = sender;
            megabuff[3] = 4;
            megabuff[4] = Neighbours[0];
            megabuff[5] = Neighbours[1];
            megabuff[6] = Neighbours[2];
            megabuff[7] = Neighbours[3];

            RawHID.write(megabuff, sizeof(megabuff));
            resetBuf();
        }
    }/*
    delay(5000);
    megabuff[0] = TypeNeedNID;
    megabuff[1] = 1;
    megabuff[2] = 0;
    megabuff[3] = 0;
    RawHID.write(megabuff, sizeof(megabuff));
    resetBuf();
    while (RawHID.available() < 64) {delay(5);}
    int newAdress = random(2, 255);
    int data = RawHID.read();
    Serial.print("Received: ");
    Serial.print(data);
    Serial.print(", ");
    Serial.println(data == TypeHereIsYourNID);
    if (data == TypeHereIsYourNID) {
        RawHID.read();
        RawHID.read();
        RawHID.read();
        newAdress = RawHID.read();
        while (RawHID.available()) {
            RawHID.read();
        }
    }
    Serial.println(newAdress);*/
}

