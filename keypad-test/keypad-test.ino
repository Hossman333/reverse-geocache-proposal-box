#include "Arduino.h"
#include "Keypad.h"

const byte ROWS = 4;
const byte COLS = 3;
// My keypad has a few malfunctioning keys. I just remapped some of them to work. 
// I only need a few of them to work anyways.
char keys[ROWS][COLS] = {
  {'0','2','*'},
  {'4','3','6'},
  {'8','8','7'},
  {'5','0','4'}
};
byte rowPins[ROWS] = {5, 6, 7, 8};
byte colPins[COLS] = {2, 3, 4};
char* secretKeyPadCode = "873450";
int keyPadPosition = 0;

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

void setup(){
  Serial.begin(9600);
}
  
void loop(){
  char key = keypad.getKey();
  
  if (key == '*') {
    keyPadPosition = 0;
    setLocked(true);
  };
  
  if (key == secretKeyPadCode[keyPadPosition]) {
    keyPadPosition = keyPadPosition + 1;
  } else if(key == NO_KEY) {
    return;
  } else {
    keyPadPosition = 0;
  };

  if (keyPadPosition == 6) {
    setLocked(false);
  };
  
  delay(50);
}

void setLocked(int locked)
{
  if (locked) {
    Serial.println("Reset combination.");
  }
  else {
    Serial.println("YAY!!!!!!");
  }
}
