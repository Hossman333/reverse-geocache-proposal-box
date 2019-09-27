/*
   Reverse Geocache Box adapted from all of these sources to create a unique
   enagement box. -Hossman333
   Spring 2019

   Credits:

   NeoGPS version without String by /dev
   Kenton Harris 11/12/2012: https://learn.adafruit.com/reverse-geocache-engagement-box/
   Thanks to bnordlund9 for much of the code. This is simplified version
   of his Geobox found here: http://www.youtube.com/watch?v=g0060tcuofg
   Reverse Geocache idea by Mikal Hart of http://arduiniana.org/
   And last but not least Adafruit for all the parts and guides.
*/

/*
    Master writer code for Arduino uno.
    - LCD
    - GPS
*/

/*   BEGIN GPS     */
#include <math.h>
#include "NMEAGPS.h"
#include "DMS.h"
#include <SoftwareSerial.h>
SoftwareSerial gps_port(3, 2);
using namespace NeoGPS;
NMEAGPS gps;
const float MILES_PER_FOOT = (1.0 / 5280.0);
const float CLOSE_ENOUGH = 20.0 * MILES_PER_FOOT; // = 0.037878787 miles
Location_t there(404483333, -1118769444);

/*   END GPS     */
#include <LiquidCrystal.h>
#include <Wire.h>

int messageCount;
int internalMessageCount;
int unlockedCount = 0;
bool initDisplayed = true;

LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

void setup()
{
  //  Serial.begin(9600);
  gps_port.begin(9600);
  gps_port.print(F("$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29\r\n" // RMC only...
                   "$PMTK220,1000*1F\r\n"));                               // ...and 1 Hz update rate

  Wire.begin();
  lcd.begin(16, 2);

  lcd.setCursor(0, 0);
  lcd.print("...beeep....boop");
  lcd.setCursor(0, 1);
  lcd.print("oh, you found me!");
  delay(5000);
}

void loop()
{
  if (unlockedCount == 0 && initDisplayed)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Don't look at me");
    lcd.setCursor(0, 1);
    lcd.print("Look at ur notes");
    initDisplayed = false;
  }
  if (unlockedCount == 2)
  {
    static uint8_t warningState = 0;
    static uint32_t lastFixTime, lastDotTime;
    while (gps.available(gps_port))
    {
      gps_fix fix = gps.read(); // save the latest
      // When we have a location, calculate how far away we are from "there".
      //      Serial.println("IN GPS CODE");
      if (fix.valid.location)
      {
        //        Serial.println("VALID FIX");
        lastFixTime = millis();
        float range = fix.location.DistanceMiles(there);
        float bearing = fix.location.BearingToDegrees(there);

        // Are we there?
        if (range < CLOSE_ENOUGH)
        {
          //          Serial.println("Unlocking!");
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Box unlocking...");
          Wire.beginTransmission(9);
          Wire.write(3);
          Wire.endTransmission();
          delay(3500);

          //          Serial.println("WILL YOU MARRY ME?!!");
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("The box is now");
          lcd.setCursor(0, 1);
          lcd.print("unlocked. Open!!");
          for (;;)
            ; // hang here
        }

        // Nope, just give a little feedback...
        //        Serial.println("WE GOT THE SIGNAL");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Signal Locked");

        lcd.setCursor(0, 1);
        lcd.print(range);
        lcd.print(" Miles");

        warningState = 0; // restart in case we lose the fix later
      }
    }
    // Display warnings when the GPS hasn't had a fix for a while

    if (millis() - lastFixTime > 2000UL)
    {

      if (warningState == 0)
      {

        // First, show the warning message...
        //        Serial.println("ACQUIRING SIGNAL");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Acquiring Signal");

        warningState = 1;
      }
      else if (warningState < 10)
      {
        //        Serial.println("BEEP BOOP");
        // ...then some dots, two per second...
        if (millis() - lastDotTime > 500UL)
        {
          lastDotTime = millis();
          lcd.setCursor(warningState - 1, 1);
          lcd.print('.');

          warningState++;
        }
      }
      else if (warningState == 10)
      {
        //        Serial.println("GO OUTSIDE");
        // ... then tell them what to do.
        lcd.setCursor(0, 1);
        lcd.print("Please wait.....");
        // Don't display anything else until location is valid
        warningState++; // 11
      }
    }
  }

  Wire.requestFrom(9, 1); // request 1 byte from slave device #9

  while (Wire.available())
  { // slave may send less than requested
    messageCount = Wire.read();
  }

  if (messageCount == 1 && internalMessageCount != 1)
  {
    lcd.clear();
    lcd.print("Phenomenal job!");
    internalMessageCount = 1;
    unlockedCount++;
    lcd.setCursor(0, 1);
    lcd.print(String(unlockedCount) + "/3 unlocked");
  }

  if (messageCount == 2 && internalMessageCount != 2)
  {
    lcd.clear();
    lcd.print("Fantastic job!");
    internalMessageCount = 2;
    unlockedCount++;
    lcd.setCursor(0, 1);
    lcd.print(String(unlockedCount) + "/3 unlocked");
  }
}

void printDMS(const Location_t &loc)
{
  DMS_t dms;
  dms.From(loc.lat());

  Serial.print(dms.NS());
  Serial << dms;

  dms.From(loc.lon());
  Serial.print(dms.EW());
  Serial << dms;
}
