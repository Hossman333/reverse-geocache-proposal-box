/*
 * Reverse Geocache Box adapted from all of these sources to create a unique
 * enagement box. -Hossman333
 * Spring 2019
 * 
 * Credits:
 * 
 * NeoGPS version without String by /dev
 * Kenton Harris 11/12/2012: https://learn.adafruit.com/reverse-geocache-engagement-box/
 * Thanks to bnordlund9 for much of the code. This is simplified version
 * of his Geobox found here: http://www.youtube.com/watch?v=g0060tcuofg
 * Reverse Geocache idea by Mikal Hart of http://arduiniana.org/
 * And last but not least Adafruit for all the parts and guides.
 */

/* 
  *  Slave Receiver code for Arduino uno.
  *  - Keypad
  *  - 4 pots 
  *  - 2 servos
  *  The slave will send data to the master, so the screen can display sentences. Such as:
  *  You’ve unlocked one!
  *  1/3 locks unlocked!
  *  The only data needed to be passed is for the screen.
*/

#include "Arduino.h"
#include "Keypad.h"
#include <Servo.h>
#include <Wire.h>

/*
 * 
 * ========================================================================
 * ============================START KEYPAD INIT===========================
 * ========================================================================
 * 
 * 
 */
const byte ROWS = 4;
const byte COLS = 3;
// My keypad has a few malfunctioning keys. I just remapped some of them to work.
// I only need a few of them to work anyways.
char keys[ROWS][COLS] = {
    {'0', '2', '*'},
    {'4', '3', '6'},
    {'8', '8', '7'},
    {'5', '0', '4'}};
byte rowPins[ROWS] = {5, 6, 7, 8};
byte colPins[COLS] = {2, 3, 4};
char *secretKeypadCode = (char *)"873450";
char *secretCodeToUnlockAll = (char *)"333777";
int keypadPosition = 0;
bool keypadIsLocked;

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

int keypadServoPin = 9;
Servo KeypadServo;

/*
 * 
 * ========================================================================
 * =============================END KEYPAD INIT============================
 * ========================================================================
 * 
 * 
 */

/*
 * 
 * ========================================================================
 * ===========================POTENTIOMETER INIT===========================
 * ========================================================================
 * 
 * 
 */
#define POT_ONE A3
#define POT_TWO A2
#define POT_THREE A1
#define POT_FOUR A0

int potOneValue = 0;
int potTwoValue = 0;
int potThreeValue = 0;
int potFourValue = 0;

// Left, Up, Down, Right
int potOneSecret = 29;
int potTwoSecret = 363;
int potThreeSecret = 1014;
int potFourSecret = 734;

int potBuffer = 40;
bool potIsLocked;

int potServoPin = 10;
Servo PotServo;
bool gpsServoIsLocked;
/*
 * 
 * ========================================================================
 * =========================END POTENTIOMETER INIT=========================
 * ========================================================================
 * 
 * 
 */
int gpsServoPin = 13;
Servo GpsServo;
int slaveResp;

void setup()
{
  Serial.begin(9600);
  Wire.begin(9);
  Wire.onRequest(requestEvent);
  Wire.onReceive(receiveEvent);
  KeypadServo.attach(keypadServoPin);
  PotServo.attach(potServoPin);
  GpsServo.attach(gpsServoPin);
  setKeypadLocked(true);
  setPotLocked(true);
  setGpsLocked(true);

  pinMode(POT_ONE, INPUT);
  pinMode(POT_TWO, INPUT);
  pinMode(POT_THREE, INPUT);
  pinMode(POT_FOUR, INPUT);
}

void loop()
{

  if (keypadIsLocked)
  {
    keypadCode();
  }

  if (potIsLocked)
  {
    potCode();
  }

  if (slaveResp == 3 || slaveResp == '3')
  {
    setGpsLocked(false);
  }

  delay(50);
}

void potCode()
{
  potOneValue = analogRead(POT_ONE);
  potTwoValue = analogRead(POT_TWO);
  potThreeValue = analogRead(POT_THREE);
  potFourValue = analogRead(POT_FOUR);
  //  Avoid math and other functions inside of ABS method.
  int potADiff = potOneValue - potOneSecret;
  int potBDiff = potTwoValue - potTwoSecret;
  int potCDiff = potThreeValue - potThreeSecret;
  int potDDiff = potFourValue - potFourSecret;

  if (abs(potADiff) < potBuffer && abs(potBDiff) < potBuffer && abs(potCDiff) < potBuffer && abs(potDDiff) < potBuffer)
  {
    setPotLocked(false);
  }
}

void keypadCode()
{
  char key = keypad.getKey();

  if (key == '*')
  {
    keypadPosition = 0;
  };

  if (key == secretKeypadCode[keypadPosition] || key == secretCodeToUnlockAll[keypadPosition])
  {
    keypadPosition = keypadPosition + 1;
  }
  else if (key == NO_KEY)
  {
    return;
  }
  else
  {
    keypadPosition = 0;
  };

  if (keypadPosition == 6 && key == secretCodeToUnlockAll[5])
  {
    KeypadServo.write(5);
    delay(50);
    PotServo.write(5);
    delay(50);
    GpsServo.write(5);
  };

  if (keypadPosition == 6)
  {
    setKeypadLocked(false);
  };
}

void setKeypadLocked(int locked)
{
  if (locked)
  {
    //    Serial.println("KEYPAD LOCKED.");
    KeypadServo.write(45);
    keypadIsLocked = true;
  }
  else
  {
    //    Serial.println("YAY, KEYPAD UNLOCKED.");
    KeypadServo.write(5);
    keypadIsLocked = false;
    slaveResp = 1;
  }
}

void setPotLocked(int locked)
{
  if (locked)
  {
    //    Serial.println("POT LOCKED.");
    PotServo.write(45);
    potIsLocked = true;
  }
  else
  {
    //    Serial.println("YAY, POT UNLOCKED.");
    PotServo.write(5);
    potIsLocked = false;
    slaveResp = 2;
  }
}

void setGpsLocked(int locked)
{
  if (locked)
  {
    //    Serial.println("LOCK THIS S.O.B");
    GpsServo.write(45);
    gpsServoIsLocked = true;
  }
  else
  {
    //    Serial.println("UNLOCK UNLOCK");
    GpsServo.write(5);
    gpsServoIsLocked = false;
  }
}

void requestEvent()
{
  Wire.write(slaveResp);
}

void receiveEvent(int bytes)
{
  slaveResp = Wire.read();
}
