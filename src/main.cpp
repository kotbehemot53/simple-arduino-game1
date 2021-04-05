#include <Arduino.h>

/*
 * CONSTANTS & VARIABLES
 */

//general constants
const int FRAME_DURATION_US = 1000;
const int FRAMES_IN_CYCLE = 2;

//pins
const int LEDS_CENTER[] = {13, 8};
const int RIGHT_BUTTONS[] = {10, 11};
const int LEFT_BUTTONS[] = {9, 12};
const int SCORE_CLS[] = {4, 7};
const int SCORE_MRS[] = {3, 6};
const int SCORE_DAS[] = {2, 5};

//game state
bool ledsCenterStates[] = {true, true};
bool leftButtonStates[] = {false, false};
bool rightButtonStates[] = {false, false};
bool buttonLegality[] = {true, true};
bool buttonImmunity[] = {false, false};
unsigned long buttonIllegalityTimes[] = {0, 0};
int points[] = {0, 0};

//rendering state
int frame = 0;
unsigned long frameStartUs = 0;
unsigned long elapsedMs = 0;

/*
 * UTIL FUNCTIONS
 */

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
    //TODO: this is just an example!
    ledsCenterStates[(elapsedMs / 1000) % 2] = true;
    ledsCenterStates[((elapsedMs / 1000) + 1) % 2] = false;
}


void resetGame() {
    for (int i = 0; i < 2; i++) {
        leftButtonStates[i] = false;
        rightButtonStates[i] = false;
        buttonLegality[i] = false;
        buttonImmunity[i] = false;
        points[i] = 0;
        ledsCenterStates[i] = false;
    }
}

void readButtonStates() {
    for (int i = 0; i < 2; i++) {
        leftButtonStates[i] = digitalRead(LEFT_BUTTONS[i]);
        rightButtonStates[i] = digitalRead(RIGHT_BUTTONS[i]);
    }
}

/**
 * THE SETUP
 */
void setup() {
    Serial.begin(115200);

    for (int i = 0; i < 2; i++) {
        pinMode(LEDS_CENTER[i], OUTPUT);
    }

    Serial.println("oi fegit");
}

/**
 * THE LOOP
 */
void loop() {
    initFrame();

//    digitalWrite(LEDS_CENTER[frame % 2], HIGH);
//    digitalWrite(LEDS_CENTER[(frame+1) % 2], LOW);

    determineLedsCenterStates();
    multiplexLeds(LEDS_CENTER, 2, ledsCenterStates);

    frameUp();
}