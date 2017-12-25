/*

GMSN Pure Sequencer v20170915
Rob Spencer 2017
cc-by 4.0

Full Open Source Documentation at https://gmsn.co.uk/pure-adsr including hardware files, Bill of Materials, Mouser Cart and Help Videos

For more indepth build and general support chat, join us at the GMSN! Synth Design Slack: https://join.slack.com/t/gmsnsynthdesign/shared_invite/MjE0NzM1ODc3NDkyLTE1MDA0NTI1MTItODQ3MDM4OTdlYw

*/
#include <EEPROM.h>

//Setup pin variables
const byte clockIn = 2;
const byte clockOut = 10;
const byte dirSW1 = 6;
const byte dirSW2 = 7;
const byte playBut = 4;
const byte playLED = 5;
const byte reset = 3;
const byte rateKnob = A7;

const byte gate1 = A0;
const byte gate2 = A1;
const byte gate3 = A2;
const byte gate4 = A3;
const byte gate5 = A4;
const byte gate6 = A5;
const byte gate7 = 8;
const byte gate8 = 9;

const byte muxA = 11;
const byte muxB = 12;
const byte muxC = 13;

//Setup state variables
byte stepNumber = 1; //Steps 1 - 16
byte dir = 1; //1 = Forward, 2 = 16 Step Pendulum, 3 = Random
byte mode; //O = Paused, 1 = Playing

//Setup internal clock timing variables
unsigned long THEN = 0;
unsigned long RATE;
byte clockState = 0;

//Setup switch debounce variables
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;
int buttonState;
int lastButtonState = LOW;

void setup() {

  //Configure Pins
  pinMode(clockIn, INPUT);
  pinMode(clockOut, OUTPUT);
  pinMode(dirSW1, INPUT);
  pinMode(dirSW2, INPUT);
  pinMode(playBut, INPUT);
  pinMode(playLED, OUTPUT);
  pinMode(reset, INPUT);
  pinMode(rateKnob, INPUT);

  pinMode(gate1, OUTPUT);
  pinMode(gate2, OUTPUT);
  pinMode(gate3, OUTPUT);
  pinMode(gate4, OUTPUT);
  pinMode(gate5, OUTPUT);
  pinMode(gate6, OUTPUT);
  pinMode(gate7, OUTPUT);
  pinMode(gate8, OUTPUT);

  pinMode(muxA, OUTPUT);
  pinMode(muxB, OUTPUT);
  pinMode(muxC, OUTPUT);

  /* The whole of the module is control by the clock in, which is attached to an interrupt. Even the internal clock out loops out of the ATMEGA and back into this interrupt.
  This keeps the whole module in rock solid time.*/
  attachInterrupt(digitalPinToInterrupt(clockIn), clockChange, CHANGE);
  
  //When powered off the module remembers whether it's in Play or Pause Mode
  mode = EEPROM[0];
}

/* As the code is all controlled by the clock into the interrupt, the main loop() routinge is really simple.
First it polls the direction swithc and sets the direction.
It then polls the play button and finally checks if it needs to toggle the internal clock.*/
void loop() {

  //Read Direction switch and set dir variable
  if (digitalRead(dirSW2) == HIGH) {
    dir = 1;
  } else if (digitalRead(dirSW1) == HIGH) {
    dir = 2;
  } else {
    dir = 3;
  }

  //Read the playButton and toggle the mode varable
  pollPlayButton();

  //Check the internal clock
  internalClock();

}

/*As the Pure Sequencer needs to stay in time with the clock, the clock is attached to an interrupt function.
The Gate On and Gate Off for each note are also controlled by the Clock In, so we need to know when we change to each phase of the clock pulse, so the interrupt is called
when the clock changes from low to high and high to low. This means we need to check which way the clock pulse is going.

Also when the Sequencer is Paused, the clock keeps counting, so when we press play the sequence is still in time.*/
void clockChange() {
  if (digitalRead(clockIn) == LOW) {
    if (mode == 1) {
      stepper(stepNumber);
    } else {
      byte PAUSED_stepNumber = map(analogRead(rateKnob), 0, 1024, 8, 0);
      stepper(PAUSED_stepNumber);
    }
  } else {
    gateOff(stepNumber);
  }
}

/*At each step, the */
void stepper(byte i) {

  //Set Fader Multiplexer to the correct channel and turn on the correct gate

  switch (i) {
    case 1:
      digitalWrite(muxA, LOW);
      digitalWrite(muxB, LOW);
      digitalWrite(muxC, LOW);

      digitalWrite(gate1, HIGH);
      digitalWrite(gate2, LOW);
      digitalWrite(gate3, LOW);
      digitalWrite(gate4, LOW);
      digitalWrite(gate5, LOW);
      digitalWrite(gate6, LOW);
      digitalWrite(gate7, LOW);
      digitalWrite(gate8, LOW);

      break;

    case 2:
      digitalWrite(muxA, HIGH);
      digitalWrite(muxB, LOW);
      digitalWrite(muxC, LOW);

      digitalWrite(gate1, LOW);
      digitalWrite(gate2, HIGH);
      digitalWrite(gate3, LOW);
      digitalWrite(gate4, LOW);
      digitalWrite(gate5, LOW);
      digitalWrite(gate6, LOW);
      digitalWrite(gate7, LOW);
      digitalWrite(gate8, LOW);

      break;

    case 3:
      digitalWrite(muxA, LOW);
      digitalWrite(muxB, HIGH);
      digitalWrite(muxC, LOW);

      digitalWrite(gate1, LOW);
      digitalWrite(gate2, LOW);
      digitalWrite(gate3, HIGH);
      digitalWrite(gate4, LOW);
      digitalWrite(gate5, LOW);
      digitalWrite(gate6, LOW);
      digitalWrite(gate7, LOW);
      digitalWrite(gate8, LOW);

      break;

    case 4:
      digitalWrite(muxA, HIGH);
      digitalWrite(muxB, HIGH);
      digitalWrite(muxC, LOW);

      digitalWrite(gate1, LOW);
      digitalWrite(gate2, LOW);
      digitalWrite(gate3, LOW);
      digitalWrite(gate4, HIGH);
      digitalWrite(gate5, LOW);
      digitalWrite(gate6, LOW);
      digitalWrite(gate7, LOW);
      digitalWrite(gate8, LOW);

      break;

    case 5:
      digitalWrite(muxA, LOW);
      digitalWrite(muxB, LOW);
      digitalWrite(muxC, HIGH);

      digitalWrite(gate1, LOW);
      digitalWrite(gate2, LOW);
      digitalWrite(gate3, LOW);
      digitalWrite(gate4, LOW);
      digitalWrite(gate5, HIGH);
      digitalWrite(gate6, LOW);
      digitalWrite(gate7, LOW);
      digitalWrite(gate8, LOW);

      break;

    case 6:
      digitalWrite(muxA, HIGH);
      digitalWrite(muxB, LOW);
      digitalWrite(muxC, HIGH);

      digitalWrite(gate1, LOW);
      digitalWrite(gate2, LOW);
      digitalWrite(gate3, LOW);
      digitalWrite(gate4, LOW);
      digitalWrite(gate5, LOW);
      digitalWrite(gate6, HIGH);
      digitalWrite(gate7, LOW);
      digitalWrite(gate8, LOW);

      break;

    case 7:
      digitalWrite(muxA, LOW);
      digitalWrite(muxB, HIGH);
      digitalWrite(muxC, HIGH);

      digitalWrite(gate1, LOW);
      digitalWrite(gate2, LOW);
      digitalWrite(gate3, LOW);
      digitalWrite(gate4, LOW);
      digitalWrite(gate5, LOW);
      digitalWrite(gate6, LOW);
      digitalWrite(gate7, HIGH);
      digitalWrite(gate8, LOW);

      break;

    case 8:
      digitalWrite(muxA, HIGH);
      digitalWrite(muxB, HIGH);
      digitalWrite(muxC, HIGH);

      digitalWrite(gate1, LOW);
      digitalWrite(gate2, LOW);
      digitalWrite(gate3, LOW);
      digitalWrite(gate4, LOW);
      digitalWrite(gate5, LOW);
      digitalWrite(gate6, LOW);
      digitalWrite(gate7, LOW);
      digitalWrite(gate8, HIGH);

      break;

    case 16:
      digitalWrite(muxA, LOW);
      digitalWrite(muxB, LOW);
      digitalWrite(muxC, LOW);

      digitalWrite(gate1, HIGH);
      digitalWrite(gate2, LOW);
      digitalWrite(gate3, LOW);
      digitalWrite(gate4, LOW);
      digitalWrite(gate5, LOW);
      digitalWrite(gate6, LOW);
      digitalWrite(gate7, LOW);
      digitalWrite(gate8, LOW);

      break;

    case 15:
      digitalWrite(muxA, HIGH);
      digitalWrite(muxB, LOW);
      digitalWrite(muxC, LOW);

      digitalWrite(gate1, LOW);
      digitalWrite(gate2, HIGH);
      digitalWrite(gate3, LOW);
      digitalWrite(gate4, LOW);
      digitalWrite(gate5, LOW);
      digitalWrite(gate6, LOW);
      digitalWrite(gate7, LOW);
      digitalWrite(gate8, LOW);

      break;

    case 14:
      digitalWrite(muxA, LOW);
      digitalWrite(muxB, HIGH);
      digitalWrite(muxC, LOW);

      digitalWrite(gate1, LOW);
      digitalWrite(gate2, LOW);
      digitalWrite(gate3, HIGH);
      digitalWrite(gate4, LOW);
      digitalWrite(gate5, LOW);
      digitalWrite(gate6, LOW);
      digitalWrite(gate7, LOW);
      digitalWrite(gate8, LOW);

      break;

    case 13:
      digitalWrite(muxA, HIGH);
      digitalWrite(muxB, HIGH);
      digitalWrite(muxC, LOW);

      digitalWrite(gate1, LOW);
      digitalWrite(gate2, LOW);
      digitalWrite(gate3, LOW);
      digitalWrite(gate4, HIGH);
      digitalWrite(gate5, LOW);
      digitalWrite(gate6, LOW);
      digitalWrite(gate7, LOW);
      digitalWrite(gate8, LOW);

      break;

    case 12:
      digitalWrite(muxA, LOW);
      digitalWrite(muxB, LOW);
      digitalWrite(muxC, HIGH);

      digitalWrite(gate1, LOW);
      digitalWrite(gate2, LOW);
      digitalWrite(gate3, LOW);
      digitalWrite(gate4, LOW);
      digitalWrite(gate5, HIGH);
      digitalWrite(gate6, LOW);
      digitalWrite(gate7, LOW);
      digitalWrite(gate8, LOW);

      break;

    case 11:
      digitalWrite(muxA, HIGH);
      digitalWrite(muxB, LOW);
      digitalWrite(muxC, HIGH);

      digitalWrite(gate1, LOW);
      digitalWrite(gate2, LOW);
      digitalWrite(gate3, LOW);
      digitalWrite(gate4, LOW);
      digitalWrite(gate5, LOW);
      digitalWrite(gate6, HIGH);
      digitalWrite(gate7, LOW);
      digitalWrite(gate8, LOW);

      break;

    case 10:
      digitalWrite(muxA, LOW);
      digitalWrite(muxB, HIGH);
      digitalWrite(muxC, HIGH);

      digitalWrite(gate1, LOW);
      digitalWrite(gate2, LOW);
      digitalWrite(gate3, LOW);
      digitalWrite(gate4, LOW);
      digitalWrite(gate5, LOW);
      digitalWrite(gate6, LOW);
      digitalWrite(gate7, HIGH);
      digitalWrite(gate8, LOW);

      break;

    case 9:
      digitalWrite(muxA, HIGH);
      digitalWrite(muxB, HIGH);
      digitalWrite(muxC, HIGH);

      digitalWrite(gate1, LOW);
      digitalWrite(gate2, LOW);
      digitalWrite(gate3, LOW);
      digitalWrite(gate4, LOW);
      digitalWrite(gate5, LOW);
      digitalWrite(gate6, LOW);
      digitalWrite(gate7, LOW);
      digitalWrite(gate8, HIGH);

      break;
  }

  //Check for Gate on reset
  if (digitalRead(reset) == LOW && dir == 1) {
    stepNumber = 0;
  } else if (digitalRead(reset) == LOW && dir == 2) {
    stepNumber = 17 - stepNumber;
  }
}

void gateOff(byte i) {
  //Turn all Gates off
  digitalWrite(gate1, LOW);
  digitalWrite(gate2, LOW);
  digitalWrite(gate3, LOW);
  digitalWrite(gate4, LOW);
  digitalWrite(gate5, LOW);
  digitalWrite(gate6, LOW);
  digitalWrite(gate7, LOW);
  digitalWrite(gate8, LOW);

  //Figure out next step
  switch (dir) {
    case 1:
      if (stepNumber == 8) {
        stepNumber = 1;
      } else {
        stepNumber++;
      }
      break;

    case 2:
      if (stepNumber == 16) {
        stepNumber = 1;
      } else {
        stepNumber++;
      }
      break;

    case 3:
      stepNumber = random(9);
      break;
  }
}

void internalClock() {
  //Read Rate Knob and scale unless knob is being used in the Pause Mode
  if (mode) {
    RATE = fscale(0, 1024, 0, 1024, analogRead(rateKnob), -5); //fast, slow
  }
  //Check if last clockState change was longer than rate and toggle Clock Out
  if ((millis() - THEN) > RATE) {
    digitalWrite(clockOut, (clockState) ? HIGH : LOW);
    clockState = !clockState;
    THEN = millis();
  }
}

void pollPlayButton() {
  int reading = digitalRead(playBut);

  if ((millis() - lastDebounceTime) > debounceDelay) {

    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == HIGH) {
        mode = !mode;
        EEPROM[0] = mode;
        lastDebounceTime = millis();
      }
    }
  }
  lastButtonState = reading;
  
  if (mode == 0) {
    digitalWrite(playLED, HIGH);
  } else {
    digitalWrite(playLED, LOW);
  }
}
