#include <Servo.h> 
//
int servoPin = 9; 
Servo Servo1; 

void setup() { 
   Servo1.attach(servoPin); 
}
void loop(){ 
   Servo1.write(5); 
   delay(3000); 
   Servo1.write(45); 
   delay(6000);     
}

