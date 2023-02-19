#include <Arduino.h>
#include "HID-Project.h"
uint8_t rawhidData[255];
byte megabuff[64];


void setup() {
    Serial.begin(115200);
    Serial1.begin(115200);
    RawHID.begin(rawhidData, sizeof(rawhidData));
    for (int i = 0; i < sizeof(megabuff); i++) {
        megabuff[i] = i;
    }
    Serial.println("Starting");
    delay(1000);
}


void loop() {
    if (Serial1.available()) {
        byte data = Serial1.read();
        Serial.println(data);
        megabuff[0] = data;
        RawHID.write(megabuff, sizeof(megabuff));
    }
}
