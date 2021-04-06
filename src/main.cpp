#include <Arduino.h>

/*
 * CONSTANTS & VARIABLES
 */

//general constants
const int SCORE_LED_CNT = 8;
const int FRAME_DURATION_US = 1000;
const int FRAMES_IN_CYCLE = SCORE_LED_CNT + 1;  //TODO: do we need this & frame counter?
const int BUTTON_COOLDOWN_MS = 1000;

//pins
const int LEDS_CENTER[2] = {13, 8};
//const int LEFT_BUTTONS[2] = {9, 12};
//const int RIGHT_BUTTONS[2] = {10, 11};
const int BUTTONS[2][2] = {{9, 10}, {12, 11}}; //players on top, l/r leds inside
const int SCORE_CLS[2] = {4, 7};
const int SCORE_MRS[2] = {3, 6};
const int SCORE_DAS[2] = {2, 5};

//game state
bool ledsCenterStates[2] = {false, false};
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

    //this is debug
//    if (delayLength < 100) {
//        Serial.print("Delay length ");
//        Serial.println(delayLength);
//    }

    delayMicroseconds(delayLength > 0 ? delayLength : 1);
}

void multiplexLeds(const int *ledArray, int ledCount, bool *states) {
    int currentLed = frame % ledCount;
    digitalWrite(ledArray[currentLed], states[currentLed]);
    for (int i = 1; i < ledCount; i++) {
        digitalWrite(ledArray[(frame + i) % ledCount], LOW);
    }
}

void resetScoreTrackWalkingPulse(int playerIdx) {
    digitalWrite(SCORE_MRS[playerIdx], LOW);
    delayMicroseconds(100); //TODO: what number is right? should it pause things?
    digitalWrite(SCORE_MRS[playerIdx], HIGH);
}

void stepScoreTrackWalkingPulse(int playerIdx) {
    digitalWrite(SCORE_CLS[playerIdx], HIGH);
    delayMicroseconds(100); //TODO: what number is right? should it pause things?
    digitalWrite(SCORE_CLS[playerIdx], LOW);
}

void delegalizeButton(int playerIdx) {
    buttonLegality[playerIdx] = false;
    //if there's immunity it will still be illegal in next frame if it's not released
    // (so you cant exploit the game by constantly holding it) but there'll be no added cooldown
    if (!buttonImmunity[playerIdx]) {
        buttonIllegalityTimes[playerIdx] = frameStartMs;
    }
}

bool isButtonLegalizable(int playerIdx) {
    if (frameStartMs - buttonIllegalityTimes[playerIdx] > BUTTON_COOLDOWN_MS) {
        return true;
    }

    return false;
}

void tryLegalizeButton(int playerIdx) {
    if (isButtonLegalizable(playerIdx)) {
        buttonLegality[playerIdx] = true;
        buttonImmunity[playerIdx] = false;
    }
}

void determineLedsCenterStates() {
    if (!ledsCenterStates[0] && !ledsCenterStates[1]) {
        if (random(100000) > 99960) {
            byte ledToSet = random(2);

            //TODO: this is debug
            Serial.print("settin led ");
            Serial.println(ledToSet);

            ledsCenterStates[ledToSet] = true;
        }
    }

//    this is just an example!
//    ledsCenterStates[(frameStartMs / 1000) % 2] = true;
//    ledsCenterStates[((frameStartMs / 1000) + 1) % 2] = false;
}


void resetGame() {
    for (int playerIdx = 0; playerIdx < 2; playerIdx++) {
        for (int ledIdx = 0; ledIdx < 2; ledIdx++) {
            buttonStates[playerIdx][ledIdx] = false;
        }
        buttonLegality[playerIdx] = false;
        buttonImmunity[playerIdx] = false;
        points[playerIdx] = 0;
    }
    for (int ledIdx = 0; ledIdx < 2; ledIdx++) {
        ledsCenterStates[ledIdx] = false;
    }
}

void readButtonStates() {
    for (int playerIdx = 0; playerIdx < 2; playerIdx++) {
        for (int ledIdx = 0; ledIdx < 2; ledIdx++) {
            buttonStates[playerIdx][ledIdx] = digitalRead(BUTTONS[playerIdx][ledIdx]);
        }
    }
}

//TODO!
void handleButtonStates() {
    for (int playerIdx = 0; playerIdx < 2; playerIdx++) { //for each led
        for (int ledIdx = 0; ledIdx < 2; ledIdx++) { //for each playerIdx
            if (!ledsCenterStates[ledIdx]) {
                if (buttonStates[playerIdx][ledIdx]) {
                    delegalizeButton(playerIdx);
                } else {
                    tryLegalizeButton(playerIdx);
                }

            } else {
                //TODO: do for BOTH buttons together!! now legality of one button can override illegality of another!
                if (!buttonStates[playerIdx][ledIdx]) {
                    tryLegalizeButton(playerIdx);
                }

                if (buttonStates[playerIdx][ledIdx] && buttonLegality[playerIdx]) {

                    delegalizeButton(playerIdx);
                    points[playerIdx]++;
                    buttonImmunity[playerIdx] = true;
                    ledsCenterStates[ledIdx] = false;

                    //TODO: this is debug
                    Serial.println(points[playerIdx]);

                    return; //don't allow the other player to get a point if he pressed the btn simultaneously (is it possible??)
                }
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
    for (int playerIdx = 0; playerIdx < 2; playerIdx++) {
        if (points[playerIdx] > SCORE_LED_CNT) {
//            winAnimation(playerIdx);
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

    for (int playerIdx = 0; playerIdx < 2; playerIdx++) {
        for (int ledIdx = 0; ledIdx < 2; ledIdx++) {
            pinMode(BUTTONS[playerIdx][ledIdx], INPUT);
        }
        pinMode(SCORE_CLS[playerIdx], OUTPUT);
        pinMode(SCORE_MRS[playerIdx], OUTPUT);
        pinMode(SCORE_DAS[playerIdx], OUTPUT);
    }
    for (int ledIdx = 0; ledIdx < 2; ledIdx++) {
        pinMode(LEDS_CENTER[ledIdx], OUTPUT);
    }
    randomSeed(analogRead(0));

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
    //TODO: 1 too many led lights up, and the topmost is dimmed
    for (int playerIdx = 0; playerIdx < 2; playerIdx ++) {
        if (frame % (SCORE_LED_CNT + 1) >= points[playerIdx]) {
            resetScoreTrackWalkingPulse(playerIdx);
        }

        if ((frame % (SCORE_LED_CNT + 1) == 0) && points[playerIdx] && isButtonLegalizable(playerIdx)) {
            digitalWrite(SCORE_DAS[playerIdx], HIGH);
        } else {
            digitalWrite(SCORE_DAS[playerIdx], LOW);
        }

        stepScoreTrackWalkingPulse(playerIdx);
    }

//    for (int i = 0; )


    frameUp();
}