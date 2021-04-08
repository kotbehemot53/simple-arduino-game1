#include <Arduino.h>

//TODO: PWM LED DIMMING
//TODO: autosleep on inactivity to conserve energy

/*
 * CONSTANTS & GLOBAL VARIABLES
 */

//general constants
const int SCORE_LED_CNT = 8;                   //score track length
const int FRAME_DURATION_US = 1000;            //single frame (loop function execution) duration
const int FRAMES_IN_CYCLE = SCORE_LED_CNT + 1; //point track shift registers cycle: 1-8 pts + 0
const int BUTTON_COOLDOWN_MS = 1000;           //cooldown period on wrong button press
const int SERIAL_REG_CLK_PULSE_LEN_US = 100;   //clock/reset pulse length for the shift registers
                                               // TODO: make sure if the period can't be shorter
const long CENTER_LED_PROBABILITY = 99900;     //probability * 100000 (so 99900 is 0,999) of one of the top LEDs lighting
                                               // up in the current frame; current number chosen experimentally, so that
                                               // intervals aren't so long that the game is boring but there still is some
                                               // degree of suspense

//pins
const int LEDS_CENTER[2] = {13, 8};            //[ledIdx: 0 - left, 1 - right]
const int BUTTONS[2][2] = {{9, 10}, {12, 11}}; //[playerIdx: 0 - left, 1 - right][ledIdx: 0 - left, 1 - right]
const int SCORE_CLS[2] = {4, 7};               //[playerIdx: 0 - left, 1 - right] - clocks for shift registers
const int SCORE_MRS[2] = {3, 6};               //[playerIdx: 0 - left, 1 - right] - /RESETs for shift registers
const int SCORE_DAS[2] = {2, 5};               //[playerIdx: 0 - left, 1 - right] - data input for shift registers

//game state
bool ledsCenterStates[2] = {false, false};                  //[ledIdx: 0 - left, 1 - right] - whether the top leds are lit
bool buttonStates[2][2] = {{false, false}, {false, false}}; //[playerIdx: 0 - left, 1 - right][ledIdx: 0 - left, 1 - right]
                                                            // whether particular buttons are pressed
bool buttonLegality[2] = {true, true};                      //[playerIdx: 0 - left, 1 - right] - if false, given user
                                                            // can't use their buttons (happens during cooldown)
bool buttonImmunity[2] = {false, false};                    //[playerIdx: 0 - left, 1 - right] - if true, cooldown period
                                                            // doesn't apply to given player - instead it only lasts 1 frame;
                                                            // it happens after receiving
                                                            // a point, when button is still held, so that there's no penalty,
                                                            // only a momentary release needed to be able to earn next point
unsigned long buttonIllegalityTimes[2] = {0, 0};            //[playerIdx: 0 - left, 1 - right] - when given player's
                                                            // buttons have become blocked (cooldown start time)
int points[2] = {0, 0};                                     //[playerIdx: 0 - left, 1 - right] - current score for each player

//rendering state
int frame = 0;
unsigned long frameStartUs = 0;
unsigned long frameStartMs = 0;



/*
 * UTIL FUNCTIONS - VISUALS
 */

/**
 * Initialize frame (start timers to ensure constant frame length)
 */
void initFrame() {
    frameStartUs = micros();
    frameStartMs = millis();
}

/**
 * Finish frame - increase frame counter (for keeping track of shift register cycle), wait to reach desired frame length
 */
void frameUp() {
    frame++;

    if (frame >= FRAMES_IN_CYCLE) {
        frame = 0;
    }

    unsigned long frameEndUs = micros();
    int delayLength = FRAME_DURATION_US - (frameEndUs - frameStartUs);

    //debug
//    if (delayLength < 100) {
//        Serial.print("Delay length ");
//        Serial.println(delayLength);
//    }

    delayMicroseconds(delayLength > 0 ? delayLength : 1);
}

/**
 * Stops the "walk" up the score track, should be called when all points for player have been displayed in given cycle
 *
 * @param playerIdx
 */
void resetScoreTrackWalkingPulse(int playerIdx) {
    digitalWrite(SCORE_MRS[playerIdx], LOW);
    delayMicroseconds(SERIAL_REG_CLK_PULSE_LEN_US);
    digitalWrite(SCORE_MRS[playerIdx], HIGH);
}

/**
 * "Walk" the score track up one step (called in every frame of the score track cycle)
 *
 * @param playerIdx
 */
void stepScoreTrackWalkingPulse(int playerIdx) {
    digitalWrite(SCORE_CLS[playerIdx], HIGH);
    delayMicroseconds(SERIAL_REG_CLK_PULSE_LEN_US);
    digitalWrite(SCORE_CLS[playerIdx], LOW);
}

/**
 * Multiplex the top LEDs
 *
 * @param ledArray
 * @param ledCount
 * @param states
 * @param frameNumber
 */
void multiplexLeds(const int *ledArray, int ledCount, bool *states, int frameNumber) {
    int currentLed = frameNumber % ledCount;
    digitalWrite(ledArray[currentLed], states[currentLed]);
    for (int i = 1; i < ledCount; i++) {
        digitalWrite(ledArray[(frameNumber + i) % ledCount], LOW);
    }
}

/**
 * Handle a single frame of a score track register
 *
 * @param playerIdx
 * @param playerScore
 * @param displayCondition
 * @param frameNumber
 */
void multiplexCustomScoreViaShiftRegister(int playerIdx, byte playerScore, bool displayCondition, int frameNumber) {
    //stop displaying points for player when all are displayed (clear the shift register)
    if (frameNumber % (SCORE_LED_CNT + 1) >= playerScore) {
        resetScoreTrackWalkingPulse(playerIdx);
    }

    //send a single "1" to the shift register in the beginning of each cycle
    if ((frameNumber % (SCORE_LED_CNT + 1) == 0) && playerScore && displayCondition) {
        digitalWrite(SCORE_DAS[playerIdx], HIGH);
    } else {
        digitalWrite(SCORE_DAS[playerIdx], LOW);
    }

    //1 clock pulse to the shift register
    stepScoreTrackWalkingPulse(playerIdx);
}

/**
 * Multiplex both LED score tracks
 */
void multiplexScoresViaShiftRegister() {
    for (int playerIdx = 0; playerIdx < 2; playerIdx ++) {
        multiplexCustomScoreViaShiftRegister(playerIdx, points[playerIdx], buttonLegality[playerIdx], frame);
    }
}

/**
 * Show a cool win "animation" using the LEDs to induce a reward reaction in player's brain and manipulate them
 * into playing even more [evil laughter].
 *
 * @param winnerIdx
 */
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

/**
 * Render regular frame stuff on all LEDs
 */
void renderVisuals() {
    multiplexLeds(LEDS_CENTER, 2, ledsCenterStates, frame);
    multiplexScoresViaShiftRegister();
}



/*
 * UTIL FUNCTIONS - GAME STATE
 */

/**
 * Try to initiate cooldown for a given player.
 *
 * @param playerIdx
 */
void delegalizeButton(int playerIdx) {
    buttonLegality[playerIdx] = false;
    //if there's immunity it will still be illegal in next frame if it's not released
    // (so you cant exploit the game by constantly holding it) but there'll be no added cooldown
    if (!buttonImmunity[playerIdx]) {
        buttonIllegalityTimes[playerIdx] = frameStartMs;
    } else {
        buttonIllegalityTimes[playerIdx] = 0; //we pretend that illegality period started a long time ago so it will end immediately
    }
}

/**
 * Check if cooldown can be ended for given player (illegality period has passed).
 *
 * @param playerIdx
 * @return bool
 */
bool isButtonLegalizable(int playerIdx) {
    if (frameStartMs - buttonIllegalityTimes[playerIdx] > BUTTON_COOLDOWN_MS) {
        return true;
    }

    return false;
}

/**
 * Try to end cooldown for a given player
 * 
 * @param playerIdx 
 */
void tryLegalizeButton(int playerIdx) {
    if (!buttonLegality[playerIdx] && isButtonLegalizable(playerIdx)) {
        buttonLegality[playerIdx] = true;
        buttonImmunity[playerIdx] = false;
    }
}

/**
 * Determine which of the top LEDs shall light up in this frame (if any).
 */
void determineLedsCenterStates() {
    //if none of the top LEDs is lit, determine if one of them should light up in the current frame;
    // there's a chance of it happening every frame; this could probably be improved to make gameplay more interesting.
    if (!ledsCenterStates[0] && !ledsCenterStates[1]) {
        if (random(100000) > CENTER_LED_PROBABILITY) {

            //choose which led to light up (50/50)
            byte ledToSet = random(2);

            //debug
//            Serial.print("settin led ");
//            Serial.println(ledToSet);

            //set lit state for the chosen LED
            ledsCenterStates[ledToSet] = true;
        }
    }
}

/**
 * Reset the game state after a player wins
 */
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

/**
 * Read button presses
 */
void readButtonStates() {
    for (int playerIdx = 0; playerIdx < 2; playerIdx++) {
        for (int ledIdx = 0; ledIdx < 2; ledIdx++) {
            buttonStates[playerIdx][ledIdx] = digitalRead(BUTTONS[playerIdx][ledIdx]);
        }
    }
}

/**
 * Handle button presses (awarod points or start cooldowns)
 */
void handleButtonStates() {
    for (int playerIdx = 0; playerIdx < 2; playerIdx++) { //for each player...
        if (!buttonStates[playerIdx][0] && !buttonStates[playerIdx][1]) { //finish cooldown if BOTH buttons are not pressed
            tryLegalizeButton(playerIdx);
        }
        for (int ledIdx = 0; ledIdx < 2; ledIdx++) { //for each of the top LEDs...
            if (!ledsCenterStates[ledIdx]) { //if given LED is not lit and button is pressed, punis the user
                if (buttonStates[playerIdx][ledIdx]) {
                    delegalizeButton(playerIdx);
                }
            } else { //if given LED is lit and the user isn't on cooldown, award them a point
                if (buttonStates[playerIdx][ledIdx] && buttonLegality[playerIdx]) {

                    points[playerIdx]++;
                    buttonImmunity[playerIdx] = true; //turn on immunity so that the user isn't punished for holding
                                                      // the button after the top LED goes off
                    ledsCenterStates[ledIdx] = false; //turn the top LED off
//                    delegalizeButton(playerIdx); //just for 1 frame - not necessary dude won't be able to release it < 1ms

//                    //debug
//                    Serial.print("Points: ");
//                    Serial.println(points[playerIdx]);

                    return; //don't allow the other player to get a point if he pressed the btn simultaneously (is it actually possible??)
                }
            }
        }
    }
}

/**
 * Check if win condition (max points for a player exceeded) has been reached.
 * If yes, run a win animation to amuse the player and reset the state of the game.
 *
 * @return bool
 */
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
    //debug
//    Serial.begin(115200);

    //set all used inputs/outputs
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
    //initialize random seed
    randomSeed(analogRead(0));

    //debug
//    Serial.println("yo yo yo");
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