#include <Arduino.h>

/*
 * CONSTANTS & VARIABLES
 */

//general constants
const int SCORE_LED_CNT = 8;
const int FRAME_DURATION_US = 1000;
const int FRAMES_IN_CYCLE = SCORE_LED_CNT;  //TODO: do we need this & frame counter?
const int BUTTON_COOLDOWN_MS = 1000;

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
int frame = 0;  //TODO: do we need this & FRAMES_IN_CYCLE?
unsigned long frameStartUs = 0;
unsigned long frameStartMs = 0;

/*
 * UTIL FUNCTIONS
 */

void initFrame() {
    frameStartUs = micros();
    frameStartMs = millis();
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

void resetScoreTrack(int i) {
    digitalWrite(SCORE_MRS[i], LOW);
    delayMicroseconds(100); //TODO: what number is right? should it pause things?
    digitalWrite(SCORE_MRS[i], HIGH);
}

void clockPulseScoreTrack(int i) {
    digitalWrite(SCORE_CLS[i], HIGH);
    delayMicroseconds(100); //TODO: what number is right? should it pause things?
    digitalWrite(SCORE_CLS[i], LOW);
}

void delegalizeButton(int i) {
    buttonLegality[i] = false;
    //if there's immunity it will still be illegal in next frame if it's not released
    // (so you cant exploit the game by constantly holding it) but there'll be no added cooldown
    if (!buttonImmunity[i]) {
        buttonIllegalityTimes[i] = frameStartMs;
    }
}

bool isButtonLegalizable(int i) {
    if (frameStartMs - buttonIllegalityTimes[i] > BUTTON_COOLDOWN_MS) {
        return true;
    }

    return false;
}

void tryLegalizeButton(int i) {
    if (isButtonLegalizable(i)) {
        buttonLegality[i] = true;
        buttonImmunity[i] = false;
    }
}

void determineLedsCenterStates() {
    //TODO: this is just an example!
    ledsCenterStates[(frameStartMs / 1000) % 2] = true;
    ledsCenterStates[((frameStartMs / 1000) + 1) % 2] = false;
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

//TODO!
void handleButtonStates() {
    //TODO: this is just a test
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            if (buttonStates[i][j]) {
                points[j]++;
                ledsCenterStates[i] = false;
            }
        }
    }

//    for (int i = 0; i < 2; i++) { //0 = LEFT, 1 = RIGHT
//        if (!ledsCenterStates[i]) {
//            for (int j = 0; j < 2; j++) {
//                if (buttonStates[i][j]) {
//                    delegalizeButton(j);
//                } else {
//                    tryLegalizeButton(j);
//                }
//            }
//        } else {
//            for (int j = 0; j < 2; j++) {
//                if (!buttonStates[i][j]) {
//                    tryLegalizeButton(j);
//                }
//                if (buttonStates[j] && buttonLegality[j]) {
//                    points[j]++;
//                    buttonImmunity[j] = true;
//                    centerState = false;
//                    break;
//                }
//            }
//        }
//    }
}


bool assertWinState() {
    for (int i = 0; i < 2; i++) {
        if (points[i] > SCORE_LED_CNT) {
//            winAnimation(i);
            resetGame();
            return true;
        }
    }

    return false;
}

/**
 * THE SETUP
 */
void setup() {
    Serial.begin(115200);

    for (int i = 0; i < 2; i++) {
        pinMode(LEDS_CENTER[i], OUTPUT);
        for (int j = 0; j < 2; j++) {
            pinMode(BUTTONS[i][j], INPUT);
        }
        pinMode(SCORE_CLS[i], OUTPUT);
        pinMode(SCORE_MRS[i], OUTPUT);
        pinMode(SCORE_DAS[i], OUTPUT);
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
    readButtonStates();
    handleButtonStates(); //TODO: do proper handling

    if (assertWinState()) {
        return;
    }

    //TODO: rendering as a whole - finish
    multiplexLeds(LEDS_CENTER, 2, ledsCenterStates);

    //TODO: render score PROPERLY in separate function
    for (int i = 0; i < 2; i ++) {
        if ((frame % SCORE_LED_CNT == 0) && (frame % SCORE_LED_CNT < points[i])) {
            digitalWrite(SCORE_DAS[i], HIGH);
        } else {
            digitalWrite(SCORE_DAS[i], LOW);
        }

        clockPulseScoreTrack(i);

        if (frame % SCORE_LED_CNT >= points[i]) {
            resetScoreTrack(i);
        }
    }

//    for (int i = 0; )


    frameUp();
}