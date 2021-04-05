#include <Arduino.h>

const int FRAME_DURATION_US = 1000;
const int FRAMES_IN_CYCLE = 2;

const int LEDS_CENTER[] = {13, 8};

bool ledsCenterStates[] = {true, true};

int frame = 0;
unsigned long frameStartUs = 0;
unsigned long elapsedMs = 0;

void initFrame() {
    frameStartUs = micros();
    elapsedMs = millis();
}

void frameUp() {
    frame++;

    if (frame >= FRAMES_IN_CYCLE) {
        frame = 0;
    }

    unsigned long frameEndUs = micros();
    int delayLength = FRAME_DURATION_US - (frameEndUs - frameStartUs);
    delayMicroseconds(delayLength > 0 ? delayLength : 1);
}

void multiplexLeds(const int *ledArray, int ledCount, bool *states) {
    int currentLed = frame % ledCount;
    digitalWrite(ledArray[currentLed], states[currentLed]);
    for (int i = 1; i < ledCount; i++) {
        digitalWrite(ledArray[(frame + i) % ledCount], LOW);
    }
}

void determineLedsCenterStates() {
    ledsCenterStates[(elapsedMs / 1000) % 2] = true;
    ledsCenterStates[((elapsedMs / 1000) + 1) % 2] = false;
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

//    digitalWrite(LEDS_CENTER[frame % 2], HIGH);
//    digitalWrite(LEDS_CENTER[(frame+1) % 2], LOW);

    determineLedsCenterStates();
    multiplexLeds(LEDS_CENTER, 2, ledsCenterStates);

    frameUp();
}