#include "SPI.h"

float aPot, aCoeff, enVal = 0, dPot, dCoeff, sPot, sCoeff, sVal, rPot, rCoeff;
boolean gate = 0, rising = 0;
int buttonState, lastButtonState = HIGH, loopStage = 0, x = 0;
long lastDebounceTime = 0, debounceDelay = 500;

void setup() {
  //DAC Comms
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);

  //Pins
  pinMode(10, OUTPUT); //DAC CS
  pinMode(2, INPUT); //TRIGGER IN
  pinMode(3, INPUT); //BUTTON TRIGGER
  pinMode(4, INPUT); //Button
  pinMode(5, INPUT); //Gate In
  pinMode(6, INPUT); //MODE SW1
  pinMode(7, INPUT); //MODE SW2
  digitalWrite(10, HIGH);

  //Interupts
  attachInterrupt(digitalPinToInterrupt(2), gateOn, FALLING);

}

void loop() {

  //Fast attack label
attack:

  //Check if not in Trap Mode
  if (digitalRead(6) != HIGH) {

    //Check if Gate is On
    if (digitalRead(4) == LOW || digitalRead(5) == LOW) {

      //Attack
      if (rising) {
        mcpWrite((int)enVal);
        //delay(5);
        if (enVal == 4095) {
          rising = 0;
        }
      }

      //Not Attack
      if (!rising) {

        //Check if in AR Mode and goto release
        if (digitalRead(6) == LOW && digitalRead(7) == LOW) {
          goto rlease;
        }

        //else continue with decay to sustain
        dPot = map(analogRead(A2), 0, 1024, 200, 0);
        float dCoeff = dPot / 16384;
        sPot = map(analogRead(A1), 0, 1024, 0, 4096);
        enVal += dCoeff * (sPot - enVal);
        mcpWrite((int)enVal);
      }
    }

    // If no Gate, write release values
    else {

      //Quick release label
rlease:

      rPot = map(analogRead(A0), 0, 1024, 80, 0);
      float rCoeff = rPot / 16384;
      enVal += rCoeff * (-100 - enVal);
      if (enVal < 0) {
        enVal = 0;
      }
      mcpWrite((int)enVal);
    }

    //Poll Trigger Button, debounce, initialise and goto Attack
    int reading = digitalRead(4);

    if ((millis() - lastDebounceTime) > debounceDelay) {

      if (reading != buttonState) {
        buttonState = reading;
        if (buttonState == LOW) {
          enVal = 0;
          rising = 1;
          rising = 1;
          lastDebounceTime = millis();
          goto attack;
        }
      }
    }
    lastButtonState = reading;

    //Get Attack Values. Moved down here to enable fast attack
    if (rising) {
      aPot = map(analogRead(A3), 0, 1024, 1024, 0);
      if (aPot == 1024) {
        enVal = 4095;
      } else {
        float aCoeff = aPot / 16384;
        enVal += aCoeff * (4311 - enVal);
        if (enVal > 4095) {
          enVal = 4095;
        }
      }
    }
    delayMicroseconds(50);

  } else {
    //Trap Mode
    switch (loopStage) {

      //if Up
      case 0:
        {
          //if enVal < 4096
          //write enVal
          aPot = map(analogRead(A3), 0, 1024, 200, 0);
          float aCoeff = aPot / 16384;
          enVal += aCoeff * (4311 - enVal);
          if (enVal > 4096) {
            enVal = 4095;
            mcpWrite((int)enVal);
            loopStage = 1;
            break;
          }
          mcpWrite((int)enVal);
        }
        break;

      //If Hold
      case 1:
        {
          //if x < dPot
          dPot = map(analogRead(A2), 0, 1024, 0, 30000);
          mcpWrite((int)enVal);
          x++;
          if (x > dPot) {
            mcpWrite((int)enVal);
            x = 0;
            loopStage = 2;
            break;
          }
        }
        break;

      //if Down
      case 2:

        {
          //if enVal > 0
          //write enVal
          sPot = map(analogRead(A1), 0, 1024, 200, 0);
          float sCoeff = sPot / 16384;
          enVal += sCoeff * (-100 - enVal);
          if (enVal < 0) {
            enVal = 0;
            mcpWrite((int)enVal);
            loopStage = 3;
            break;
          }
          mcpWrite((int)enVal);
        }
        break;

      //if Off
      case 3:
        {
          rPot = map(analogRead(A0), 0, 1024, 0, 32800);
          if (x < rPot) {
            //write hold value
            mcpWrite((int)enVal);
            x ++;
          } else {
            //else Turn on Down
            x = 0;
            loopStage = 0;
            break;
          }
        }
        break;

    }

    //Poll Trigger Button, debounce, initialise Up phase
    int reading = digitalRead(4);

    if ((millis() - lastDebounceTime) > debounceDelay) {

      if (reading != buttonState) {
        buttonState = reading;
        if (buttonState == LOW) {
          enVal = 0;
          x = 0;
          loopStage = 0;
          lastDebounceTime = millis();
        }
      }
    }
    lastButtonState = reading;
    delayMicroseconds(50);
  }
}

//Interrupt routine for rising edge of Gate
void gateOn() {
  enVal = 0;
  rising = 1;
  loopStage = 0;
  x = 0;
}

//Function for writing value to DAC. 0 = Off 4095 = Full on.

void mcpWrite(int value) {

  //CS
  digitalWrite(10, LOW);

  //DAC1 write

  //set top 4 bits of value integer to data variable
  byte data = value >> 8;
  data = data & B00001111;
  data = data | B00110000;
  SPI.transfer(data);

  data = value;
  SPI.transfer(data);

  // Set digital pin 10 HIGH
  digitalWrite(10, HIGH);
}

//Test function for flashing the led. Value = no of flashes, time = time between flashes in mS
void flash(int value, int time) {
  int x = 0;
  while (x < value) {
    mcpWrite(4000);
    delay(time);
    mcpWrite(0);
    delay(200);
    x++;
  }
}
