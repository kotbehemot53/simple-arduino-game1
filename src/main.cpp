#include <Arduino.h>

/*
 * CONSTANTS & VARIABLES
 */

//general constants
const int FRAME_DURATION_US = 1000;
const int FRAMES_IN_CYCLE = 2;

//pins
const int LEDS_CENTER[2] = {13, 8};
//const int LEFT_BUTTONS[2] = {9, 12};
//const int RIGHT_BUTTONS[2] = {10, 11};
const int BUTTONS[2][2] = {{9, 12}, {10, 11}}; //L/R on top, inside
const int SCORE_CLS[2] = {4, 7};
const int SCORE_MRS[2] = {3, 6};
const int SCORE_DAS[2] = {2, 5};

//game state
bool ledsCenterStates[2] = {true, true};
//bool leftButtonStates[] = {false, false};
//bool rightButtonStates[] = {false, false};
bool buttonStates[2][2] = {{false, false}, {false, false}}; //L/R on top, inside
bool buttonLegality[2] = {true, true};
bool buttonImmunity[2] = {false, false};
unsigned long buttonIllegalityTimes[2] = {0, 0};
int points[2] = {0, 0};

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
        for (int j = 0; j < 2; j++) {
            buttonStates[i][j] = false;
        }
        buttonLegality[i] = false;
        buttonImmunity[i] = false;
        points[i] = 0;
        ledsCenterStates[i] = false;
    }
}

void readButtonStates() {
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            buttonStates[i][j] = digitalRead(BUTTONS[i][j]);
        }
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