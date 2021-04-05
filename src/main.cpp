#include <Arduino.h>

const int LEDS_CENTER[] = {13, 8};

int frame = 0;

void setup() {
    Serial.begin(115200);

    for (int i = 0; i < 2; i++) {
        pinMode(LEDS_CENTER[i], OUTPUT);
    }

    Serial.println("oi fegit");
}

void frameUp() {
    frame++;

    if (frame >= 2) {
        frame = 0;
    }

    delay(1000);
}

void loop() {
    digitalWrite(LEDS_CENTER[frame % 2], HIGH);
    digitalWrite(LEDS_CENTER[(frame+1) % 2], LOW);

    frameUp();
}