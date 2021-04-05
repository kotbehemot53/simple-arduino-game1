#include <Arduino.h>

const int FRAME_DURATION_US = 1000;

const int LEDS_CENTER[] = {13, 8};

unsigned long frame = 0;
unsigned long frameStartUs = 0;

void initFrame() {
    frameStartUs = micros();
}

void frameUp() {
    frame++;

    if (frame >= 2) {
        frame = 0;
    }

    unsigned long frameEndUs = micros();
    int delayLength = FRAME_DURATION_US - (frameEndUs - frameStartUs);
    delayMicroseconds(delayLength > 0 ? delayLength : 1);
}

void setup() {
    Serial.begin(115200);

    for (int i = 0; i < 2; i++) {
        pinMode(LEDS_CENTER[i], OUTPUT);
    }

    Serial.println("oi fegit");
}

void loop() {
    initFrame();

    digitalWrite(LEDS_CENTER[frame % 2], HIGH);
    digitalWrite(LEDS_CENTER[(frame+1) % 2], LOW);

    frameUp();
}