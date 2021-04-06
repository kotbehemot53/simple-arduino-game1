#include <Arduino.h>

//TODO: PWM DIMMING

/*
 * CONSTANTS & VARIABLES
 */

//general constants
const int SCORE_LED_CNT = 8;
const int FRAME_DURATION_US = 1000;
const int FRAMES_IN_CYCLE = SCORE_LED_CNT + 1;
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
 * UTIL FUNCTIONS - VISUALS
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

void multiplexLeds(const int *ledArray, int ledCount, bool *states, int frameNumber) {
    int currentLed = frameNumber % ledCount;
    digitalWrite(ledArray[currentLed], states[currentLed]);
    for (int i = 1; i < ledCount; i++) {
        digitalWrite(ledArray[(frameNumber + i) % ledCount], LOW);
    }
}

void multiplexCustomScoreViaShiftRegister(int playerIdx, byte playerScore, bool displayCondition, int frameNumber) {
    if (frameNumber % (SCORE_LED_CNT + 1) >= playerScore) {
        resetScoreTrackWalkingPulse(playerIdx);
    }

    if ((frameNumber % (SCORE_LED_CNT + 1) == 0) && playerScore && displayCondition) {
        digitalWrite(SCORE_DAS[playerIdx], HIGH);
    } else {
        digitalWrite(SCORE_DAS[playerIdx], LOW);
    }

    stepScoreTrackWalkingPulse(playerIdx);
}

void multiplexScoresViaShiftRegister() {
    for (int playerIdx = 0; playerIdx < 2; playerIdx ++) {
        multiplexCustomScoreViaShiftRegister(playerIdx, points[playerIdx], buttonLegality[playerIdx], frame);
    }
}

void winAnimation(int winnerIdx) {
    //dim the loser to show who won
    for (int playerIdx = 0; playerIdx < 2; playerIdx++) {
        resetScoreTrackWalkingPulse(playerIdx);
    }

    //go up the ladder
    for (int i = 0; i < 9; i++) {
        multiplexCustomScoreViaShiftRegister(winnerIdx, i+1, true, i);
        delay(500-i*30);
    }

    //blink the ladder and the center leds
    bool centStates[2];
    for (int i = 0; i < 1800; i++) {
        //ladder blink
        multiplexCustomScoreViaShiftRegister(winnerIdx, 8, (i/300) % 2 == 0, i);
        //center leds blink (interchangeable)
        centStates[0] = (i/300) % 2 == 0;
        centStates[1] = !centStates[0];
        multiplexLeds(LEDS_CENTER, 2, centStates, i);

        delay(1);
    }
}

void renderVisuals() {
    multiplexLeds(LEDS_CENTER, 2, ledsCenterStates, frame);
    multiplexScoresViaShiftRegister();
}

/*
 * UTIL FUNCTIONS - GAME STATE
 */

void delegalizeButton(int playerIdx) {
    buttonLegality[playerIdx] = false;
    //if there's immunity it will still be illegal in next frame if it's not released
    // (so you cant exploit the game by constantly holding it) but there'll be no added cooldown
    if (!buttonImmunity[playerIdx]) {
//        Serial.println("no immunity!");
        buttonIllegalityTimes[playerIdx] = frameStartMs;
    } else {
//        Serial.println("immunity!");
        buttonIllegalityTimes[playerIdx] = 0;
    }
}

bool isButtonLegalizable(int playerIdx) {
    if (frameStartMs - buttonIllegalityTimes[playerIdx] > BUTTON_COOLDOWN_MS) {
        return true;
    }

    return false;
}

void tryLegalizeButton(int playerIdx) {
    if (!buttonLegality[playerIdx] && isButtonLegalizable(playerIdx)) {
        buttonLegality[playerIdx] = true;
        buttonImmunity[playerIdx] = false;
//        Serial.println("Legalized, immunity reset");
    }
}

void determineLedsCenterStates() {
    if (!ledsCenterStates[0] && !ledsCenterStates[1]) {
        if (random(100000) > 99960) {
            byte ledToSet = random(2);

            //this is debug
//            Serial.print("settin led ");
//            Serial.println(ledToSet);

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
        buttonLegality[playerIdx] = true;
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

void handleButtonStates() {
    for (int playerIdx = 0; playerIdx < 2; playerIdx++) { //for each led
        if (!buttonStates[playerIdx][0] && !buttonStates[playerIdx][1]) { //legalize player if BOTH buttons are not pressed
            tryLegalizeButton(playerIdx);
        }
        for (int ledIdx = 0; ledIdx < 2; ledIdx++) { //for each playerIdx
            if (!ledsCenterStates[ledIdx]) {
                if (buttonStates[playerIdx][ledIdx]) {
                    delegalizeButton(playerIdx);
                }
            } else {
                if (buttonStates[playerIdx][ledIdx] && buttonLegality[playerIdx]) {

                    points[playerIdx]++;
                    buttonImmunity[playerIdx] = true;
                    ledsCenterStates[ledIdx] = false;
//                    delegalizeButton(playerIdx); //just for 1 frame - not necessary dude won't be able to release it < 1ms

//                    //this is debug
//                    Serial.println(points[playerIdx]);

                    return; //don't allow the other player to get a point if he pressed the btn simultaneously (is it possible??)
                }
            }
        }
    }
}

bool assertWinState() {
    for (int playerIdx = 0; playerIdx < 2; playerIdx++) {
        if (points[playerIdx] > SCORE_LED_CNT) {
            winAnimation(playerIdx);
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

    determineLedsCenterStates();
    readButtonStates();
    handleButtonStates();

    if (assertWinState()) {
        return;
    }

    renderVisuals();

    frameUp();
}