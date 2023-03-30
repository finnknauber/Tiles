#include <Arduino.h>
#include "HID-Project.h"
uint8_t rawhidData[255];
byte megabuff[64];


#define TypeGeneral           0b00010001
#define TypeACK               0b00100010
#define TypeNACK              0b00110011
#define TypeAreYouMaster      0b01000100
#define TypeGiveMeANID        0b01010101
#define TypeReportHID         0b01100110
#define TypeGiveMeYourNID     0b01110111
#define TypeHereIsMyNID       0b10001000
#define TypeReportNeighbours  0b10011001


bool active_message = false;
int message_type = 0;


void setup() {
    pinMode(0, INPUT_PULLUP);
    Serial.begin(115200);
    Serial1.begin(115200);
    RawHID.begin(rawhidData, sizeof(rawhidData));
    for (int i = 0; i < sizeof(megabuff); i++) {
        megabuff[i] = i;
    }
    // megabuff[0] = data;
    // RawHID.write(megabuff, sizeof(megabuff));
    Serial.println("Starting");
    delay(1000);
}


void loop() {
    if (Serial1.available()) {
        int data = Serial1.read();
        if (data == TypeAreYouMaster) {
            Serial.println("Are you Master?");
            Serial1.write(TypeACK);
            while (Serial1.available()) {
                Serial1.read();
            }
            delay(10);
        }
        else if (data == TypeGeneral) {
            while (Serial1.available() < 4) {}
            if (Serial1.read() == 0) {
                Serial.print(Serial1.read());
                Serial.print(": ");
                Serial1.read();
                data = Serial1.read();
                Serial.println(data);
                switch (data) {
                    case 0:
                        Serial.println("Button Pressed!");
                        break;

                    case 1:
                        Serial.println("Button Released!");
                        break;
                    
                    case 2:
                        Serial.println("Enoder Turned Right!");
                        break;
                    
                    case 3:
                        Serial.println("Encoder Turned Left!");
                        break;

                    default:
                        Serial.println("Not recognized");
                        break;
                }
            }
        }
    }
}

