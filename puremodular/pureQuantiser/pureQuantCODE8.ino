#include "SPI.h"

boolean trig = false;
unsigned long pMillis;
int outputPre = 0;

int equalTempered[13] =
{
  0, 68, 136, 205, 273, 341, 409, 478, 546, 614, 682, 751, 819
};


boolean flag[12] =
{
  true, true, true, true,
  true, true, true, true,
  true, true, true, true,
};

byte counter;

byte LEDs[2] =
{
  B11111111, B00001111,
};

byte frames[12] =
{

};

int i = 0;
int dacoutcs = 10;
int dacincs = 9;
int trigout = 8;
int lr3 = 7;
int br0 = 3;
int br1 = 4;
int br2 = 5;
int br3 = 6;

void setup() {
  SPI.begin();
  DDRC = B00111111;
  //pinMode(7, OUTPUT);
  //DAC CS
  pinMode(dacoutcs, OUTPUT);
  pinMode(dacincs, OUTPUT);
  digitalWrite(dacincs, HIGH);
  pinMode(trigout, OUTPUT);

  DDRD = B10000111;

  PORTC = B110001;
  digitalWrite(lr3, HIGH);

}

void loop() {

  //Read DAC and quantise it to equal temp
  int output = quantizeST(DAC_read(1));

  //Check if output value has changed
  if (output != outputPre) {
    outputPre = output;
    trig = true;
    pMillis = millis();
  }

  //If no LEDs lit, output 0;
  if (LEDs[0] + LEDs[1] == 0) {
    dacWrite(0);
  } else {

    //Else output CV
    dacWrite(output);
  }

  //If output value changed, write trigger out
  if (trig) {
    PORTB  |= (1);
    if (millis() - pMillis >= 15) {
      PORTB  &= ~(1);
      trig = false;
    }
  }

  //PORTB ^= (1);
  //delay(500);

  /*
    i++;
    if(i > 4095){
    i = 0;
    }
    delay(1);
  */
  frame(counter);
  buttonRead(counter);
  counter++;
  if (counter > 12) {
    counter = 0;
  }
}



void dacWrite(int value) {

  //MCP4921 Chip Select active LOW
  PORTB &= ~(1 << 2);
  //PORTB = PORTB & B11111011;
  //PORTB = PORTB | B00000000;

  //set top 4 bits of value integer to data variable
  byte data = value >> 8;
  data = data & B00001111;
  data = data | B00110000;
  SPI.transfer(data);

  data = value;
  SPI.transfer(data);

  //MCP4921 Chip Select reset High
  PORTB |= (1 << 2);
  //PORTB = PORTB & B11111011;
  //PORTB = PORTB | B00000100;

}

int DAC_read( byte channel ) {
  byte commandbits = B00001101;          //command bits - 0000, start, mode, chn, MSBF
  unsigned int b1 = 0;                   // get the return var's ready
  unsigned int b2 = 0;
  commandbits |= ((channel - 1) << 1);   // update the command bit to select either ch 1 or 2
  //digitalWrite(cs, LOW);                 // select the MCP3202
  PORTB &= ~(1 << 1);
  SPI.transfer(commandbits);             // send out the command bits
  const int hi = SPI.transfer(b1);       // read back the result high byte
  const int lo = SPI.transfer(b2);       // then the low byte
  //digitalWrite(cs, HIGH);                // let the DAC go, we'done
  PORTB |= (1 << 1);
  b1 = lo + (hi << 8);                   // assemble the two bytes into a word
  return (b1 >> 4);                      // We have got a 12bit answer but strip LSB's if
  // required >>4 ==10 bit (0->1024), >>2 ==12bit (0->4096)
}


void frame(byte frameCount) {

  //Read individual BITs from LED arrays. 1 = LED lit = state true
  boolean state = false;
  //led's C - G
  if (frameCount < 8) {
    if (bitRead(LEDs[0], frameCount) == 1) {
      state = true;

    }
  }
  //led's G# - B
  else {
    if (bitRead(LEDs[1], frameCount - 8) == 1) {
      state = true;
    }
  }

  //If no LEDs are lit, don't light any
  if (!state) {
    PORTC = B00000000;
    PORTD &= ~(1 << 7);
  }

  //else state is true, and framecounter is the LED to light.
  else {
    //C
    if (frameCount == 0) {
      PORTC = B00110100;
      PORTD |= (1 << 7);
    }
    //C#
    else if (frameCount == 1) {

      PORTC = B00101100;
      PORTD |= (1 << 7);
    }
    //D
    else if (frameCount == 2) {
      PORTC = B00011100;
      PORTD |= (1 << 7);
    }
    //D#
    else if (frameCount == 3) {
      PORTC = B00111100;
      PORTD &= ~(1 << 7);
    }
    //E
    else if (frameCount == 4) {
      PORTC = B00110010;
      PORTD |= (1 << 7);
    }
    //F
    else if (frameCount == 5) {
      PORTC = B00101010;
      PORTD |= (1 << 7);
    }
    //F#
    else if (frameCount == 6) {
      PORTC = B00011010;
      PORTD |= (1 << 7);
    }
    //G
    else if (frameCount == 7) {
      PORTC = B00111010;
      PORTD &= ~(1 << 7);
    }
    //G#
    else if (frameCount == 8) {
      PORTC = B00110001;
      PORTD |= (1 << 7);
    }
    //A
    else if (frameCount == 9) {
      PORTC = B00101001;
      PORTD |= (1 << 7);
    }
    //A#
    else if (frameCount == 10) {
      PORTC = B00011001;
      PORTD |= (1 << 7);
    }
    //B
    else if (frameCount == 11) {
      PORTC = B00111001;
      PORTD &= ~(1 << 7);
    }
  }


}



void buttonRead(byte count) {

  //
  //C, C#, D, D#
  if (count < 4) {
    //Turn on BC0 so we can read the row
    PORTD = PORTD & B11111000;
    PORTD = PORTD | B00000001;


    if (count == 0) {
      //Check the current state of the button and toggle if pressed
      if (digitalRead(br0) == HIGH && flag[count]) {
        flag[count] = false;
        LEDs[0] ^= (1 << count);


      }
      if (digitalRead(br0) == LOW && !flag[count]) {
        flag[count] = true;
      }

      //If LED lit set note offset in equalTempered array, else remove the offset
      if (bitRead(LEDs[0], count) == 1) {
        equalTempered[count] = 0;
      }
      else {
        equalTempered[count] = equalTempered[11];
      }

    }

    else if (count == 1) {
      if (digitalRead(br1) == HIGH && flag[count]) {
        flag[count] = false;
        LEDs[0] ^= (1 << count);



      }
      if (digitalRead(br1) == LOW && !flag[count]) {
        flag[count] = true;
      }


      if (bitRead(LEDs[0], count) == 1) {
        equalTempered[count] = 68;
      }
      else {
        equalTempered[count] = equalTempered[count - 1];
      }
    }

    else if (count == 2) {
      if (digitalRead(br2) == HIGH && flag[count]) {
        flag[count] = false;
        LEDs[0] ^= (1 << count);


      }
      if (digitalRead(br2) == LOW && !flag[count]) {
        flag[count] = true;
      }


      if (bitRead(LEDs[0], count) == 1) {
        equalTempered[count] = 136;
      }
      else {
        equalTempered[count] = equalTempered[count - 1];
      }
    }

    else if (count == 3) {
      if (digitalRead(br3) == HIGH && flag[count]) {
        flag[count] = false;
        LEDs[0] ^= (1 << count);



      }
      if (digitalRead(br3) == LOW && !flag[count]) {
        flag[count] = true;
      }


      if (bitRead(LEDs[0], count) == 1) {
        equalTempered[count] = 205;
      }
      else {
        equalTempered[count] = equalTempered[count - 1];
      }
    }
  }

  //G#, A, A#, B
  else if (count > 7) {
    PORTD = PORTD & B11111000;
    PORTD = PORTD | B00000100;

    if (count == 8) {
      if (digitalRead(br0) == HIGH && flag[count]) {
        flag[count] = false;
        LEDs[1] ^= (1 << count - 8);

      }
      if (digitalRead(br0) == LOW && !flag[count]) {
        flag[count] = true;
      }


      if (bitRead(LEDs[1], count - 8) == 1) {
        equalTempered[count] = 546;
      }
      else {
        equalTempered[count] = equalTempered[count - 1];
      }
    }

    else if (count == 9) {
      if (digitalRead(br1) == HIGH && flag[count]) {
        flag[count] = false;
        LEDs[1] ^= (1 << count - 8);

      }
      if (digitalRead(br1) == LOW && !flag[count]) {
        flag[count] = true;
      }


      if (bitRead(LEDs[1], count - 8) == 1) {
        equalTempered[count] = 614;
      }
      else {
        equalTempered[count] = equalTempered[count - 1];
      }
    }

    else if (count == 10) {
      if (digitalRead(br2) == HIGH && flag[count]) {
        flag[count] = false;
        LEDs[1] ^= (1 << count - 8);

      }
      if (digitalRead(br2) == LOW && !flag[count]) {
        flag[count] = true;
      }


      if (bitRead(LEDs[1], count - 8) == 1) {
        equalTempered[count] = 682;
      }
      else {
        equalTempered[count] = equalTempered[count - 1];
      }
    }

    else if (count == 11) {
      if (digitalRead(br3) == HIGH && flag[count]) {
        flag[count] = false;
        LEDs[1] ^= (1 << count - 8);

      }
      if (digitalRead(br3) == LOW && !flag[count]) {
        flag[count] = true;
      }


      if (bitRead(LEDs[1], count - 8) == 1) {
        equalTempered[count] = 751;
      }
      else {
        equalTempered[count] = equalTempered[count - 1];
      }
    }
  }

  //E, F, F#, G
  else {
    PORTD = PORTD & B11111000;
    PORTD = PORTD | B00000010;

    if (count == 4) {
      if (digitalRead(br0) == HIGH && flag[count]) {
        flag[count] = false;
        LEDs[0] ^= (1 << count);

      }
      if (digitalRead(br0) == LOW && !flag[count]) {
        flag[count] = true;
      }


      if (bitRead(LEDs[0], count) == 1) {
        equalTempered[count] = 273;
      }
      else {
        equalTempered[count] = equalTempered[count - 1];
      }
    }

    else if (count == 5) {
      if (digitalRead(br1) == HIGH && flag[count]) {
        flag[count] = false;
        LEDs[0] ^= (1 << count);

      }
      if (digitalRead(br1) == LOW && !flag[count]) {
        flag[count] = true;
      }


      if (bitRead(LEDs[0], count) == 1) {
        equalTempered[count] = 341;
      }
      else {
        equalTempered[count] = equalTempered[count - 1];
      }
    }

    else if (count == 6) {
      if (digitalRead(br2) == HIGH && flag[count]) {
        flag[count] = false;
        LEDs[0] ^= (1 << count);

      }
      if (digitalRead(br2) == LOW && !flag[count]) {
        flag[count] = true;
      }


      if (bitRead(LEDs[0], count) == 1) {
        equalTempered[count] = 409;
      }
      else {
        equalTempered[count] = equalTempered[count - 1];
      }
    }

    else if (count == 7) {
      if (digitalRead(br3) == HIGH && flag[count]) {
        flag[count] = false;
        LEDs[0] ^= (1 << count);

      }
      if (digitalRead(br3) == LOW && !flag[count]) {
        flag[count] = true;
      }


      if (bitRead(LEDs[0], count) == 1) {
        equalTempered[count] = 478;
      }
      else {
        equalTempered[count] = equalTempered[count - 1];
      }
    }
  }


}


int quantizeST(int val){
  int controlVoltage;
  controlVoltage = 819 * (val/205);
  controlVoltage += equalTempered[(val%205)/17];

  //limiter to max Value
  /*
  if(val == 1023){
    controlVoltage = 4095;
  }
  */
  return controlVoltage >> 1;
}















