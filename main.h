#include <MsTimer2.h>    // Timer interrupt function library.
#include <SPI.h>    // SPI library.
#include <Wire.h>    // i2c library.
#include <Adafruit_GFX.h>    // Adafruits graphics library.
#include <Adafruit_SSD1306.h>    // Adafruits OLED library.

#define OLED_RESET 4

Adafruit_SSD1306 display(OLED_RESET);
int pbIn = 0;                    // Define interrupt 0 that is digital pin 2.
int ledOut = 13;                 // Define the indicator LED pin digital pin 13.
int number = 0;                  // Interrupt times.
volatile int state = LOW;         // Defines the indicator LED state, the default is not bright.

int maxRange = 400;
int screenHeight = 31;    // Screen positions are indexed to zero so number is pixels - 1.
int screenWidth = 127;    // Screen positions are indexed to zero so number is pixels - 1.

// Front ultrasonic pins
int trigPinF = 11;    // Trig.
int echoPinF = 12;    // Echo.

// Back ultrasonic pins
int trigPinB = 9;    // Trig.
int echoPinB = 10;    // Echo.

// Right ultrasonic pins
int trigPinR = 7;    // Trig.
int echoPinR = 8;    // Echo.

// Left ultrasonic pins
int trigPinL = 5;    // Trig.
int echoPinL = 6;    // Echo.

// Setup variables
long duration, cm, inches;    // Using longs for calculations and raw measurements
double cmRangeFrontDown, cmRangeBackDown, cmRangeRightDown, cmRangeLeftDown;    // Using doubles for division in calculations.
int cmRangeFront, cmRangeBack, cmRangeRight, cmRangeLeft, maxRangeM;    // Integers for final use.

boolean debugMode = false;    // Set to true to display real time numbers of screen coordinates calculated from the ultrasonic distance data.

void setup()
{
  pinMode(ledOut, OUTPUT);
  attachInterrupt(pbIn, stateChange, FALLING); // Set the interrupt function, interrupt pin is digital pin D2, interrupt service function is stateChange (), when the D2 power change from high to low , the trigger interrupts.
  MsTimer2::set(1000, Handle); // Set the timer interrupt function, running once Handle() function per 1000ms.
  MsTimer2::start();// Start timer interrupt function.
  // Define inputs and outputs.
  pinMode(trigPinF, OUTPUT);
  pinMode(echoPinF, INPUT);
  pinMode(trigPinB, OUTPUT);
  pinMode(echoPinB, INPUT);
  pinMode(trigPinR, OUTPUT);
  pinMode(echoPinR, INPUT);
  pinMode(trigPinL, OUTPUT);
  pinMode(echoPinL, INPUT);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // Initialize with the I2C addr 0x3C (for the 128x32).

  // Set default text size and colour.
  display.setTextSize(1);
  display.setTextColor(WHITE);
}

void loop()
{

  // Clear display for loop beginning.
  display.clearDisplay();

  // The below code takes the centimeter distance from the ultrasonic sensors and divides them down into ranges to display the walls detected onto the screen as lines, that move proportional to the ranges of the walls being detected.
  // It is calculated in a way that the walls can be shown as lines that move closer to the centre of the screen, the closer the walls physically are to the user.

  // Measure distance to object from front-facing ultrasonic sensor.
  cm = distance(trigPinF, echoPinF); // Call distance measure function with trigger/echo pins for front sensor
  cmRangeFrontDown = (((double)cm / (double)maxRange) * (screenHeight / 2)) + 0.5; // Calculate the 0 > 1 decimal of the range against the maximum range of the sensor and times by half the screen height - finally adding 0.5 to round up.
  cmRangeFront = 16 - (int)cmRangeFrontDown; // Finally minus the number calculated above from half the screen height, so that the line will move closer to the center of the screen - the center being 0cm range.

  if (debugMode == true) 
  {
    display.setCursor(0, 20);
    display.println((String)cm);
  }

  // Measure distance to object from back-facing ultrasonic sensor.
  cm = distance(trigPinB, echoPinB); // Call distance measure function with trigger/echo pins for back sensor
  cmRangeBackDown = (((double)cm / (double)maxRange) * (screenHeight / 2)) + 0.5; // Calculate the 0 > 1 decimal of the range against the maximum range of the sensor and times by half the screen height - finally adding 0.5 to round up.
  cmRangeBack = 15 + (int)cmRangeBackDown; // Finally add the number calculated above to half the screen height, so that the line will move closer to the center of the screen - the center being 0cm range.

  if (debugMode == true) 
  {
    display.setCursor(30, 20);
    display.println((String)cm);
  }

  // Measure distance to object from right-facing ultrasonic sensor.
  cm = distance(trigPinR, echoPinR); // Call distance measure function with trigger/echo pins for right sensor
  cmRangeRightDown = (((double)cm / (double)maxRange) * (screenWidth / 2)) + 0.5; // Calculate the 0 > 1 decimal of the range against the maximum range of the sensor and times by half the screen width - finally adding 0.5 to round up.
  cmRangeRight = 64 + (int)cmRangeRightDown; // Finally add the number calculated above to half the screen width, so that the line will move closer to the center of the screen - the center being 0cm range.

  if (debugMode == true) 
  {
    display.setCursor(60, 20);
    display.println((String)cm);
  }

  // Measure distance to object from left-facing ultrasonic sensor.
  cm = distance(trigPinL, echoPinL); // Call distance measure function with trigger/echo pins for left sensor
  cmRangeLeftDown = (((double)cm / (double)maxRange) * (screenWidth / 2)) + 0.5; // Calculate the 0 > 1 decimal of the range against the maximum range of the sensor and times by half the screen width - finally adding 0.5 to round up.
  cmRangeLeft = 63 - (int)cmRangeLeftDown; // Finally minus the number calculated above from half the screen width, so that the line will move closer to the center of the screen - the center being 0cm range.

  if (debugMode == true) 
  {
    display.setCursor(90, 20);
    display.println((String)cm);
  }

  // Debug mode to display numbers of positions on screen .
  if (debugMode == true) 
  {
    display.setCursor(0, 0);
    display.setCursor(0, 10);
    display.println((String)cmRangeFront + " " + cmRangeBack + " " + cmRangeRight + " " + cmRangeLeft);
  }

  // Pass along calcuted positions into line drawing function.
  drawLines(cmRangeFront, cmRangeBack, cmRangeRight, cmRangeLeft);

  // When a moving object is detected, the ledout is automatically closed after the light 2S, the next trigger can be carried out, and no need to reset. Convenient debugging.
  // When movement is detected display that motion has been detected on-screen.
  if (state == HIGH) 
  {
    state = LOW;
    digitalWrite(ledOut, state);    // Turn off led.
    display.setCursor(0, 0);
    display.println("+");
  }

  // In the bottom left hand corner display the range of the wall/motion detection.
  display.setCursor(116, 25);
  maxRangeM = maxRange / 100;
  display.println((String)maxRangeM+"M");
  display.drawPixel(63, 15, WHITE);

  // Finally display all information to the screen.
  display.display();
}

// Function for drawing both horizontal and both vertical lines by their x, y coords as calculated.
void drawLines(int cmRangeF, int cmRangeB, int cmRangeR, int cmRangeL) 
{
  display.drawLine(0, cmRangeF, 128, cmRangeF, WHITE);
  display.drawLine(0, cmRangeB, 128, cmRangeB, WHITE);
  display.drawLine(cmRangeR, 0, cmRangeR, 32, WHITE);
  display.drawLine(cmRangeL, 0, cmRangeL, 32, WHITE);
}

long distance(int triggerP, int echoP) 
{
  // The sensor is triggered by a HIGH pulse of 10 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse.
  digitalWrite(triggerP, LOW);
  delayMicroseconds(5);
  digitalWrite(triggerP, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerP, LOW);

  // Read the signal from the sensor: a HIGH pulse whose.
  // Duration is the time (in microseconds) from the sending.
  // Of the ping to the reception of its echo off of an object.
  pinMode(echoP, INPUT);
  duration = pulseIn(echoP, HIGH);

  // Convert the time into a distance.
  cm = (duration / 2) / 29.1;
  inches = (duration / 2) / 74;

  return cm;    // Return final centimeter calculation.
}

void stateChange()    // Interrupt service function.
{
  number++;    // Interrupted once, the number +1.
}

void Handle()    // Timer service function.
{
  if (number > 1) // If in the set of the interrupt time the number more than 1 times, then means have detect moving objects,This value can be adjusted according to the actual situation, which is equivalent to adjust the threshold of detection speed of moving objects.
  {
    state = HIGH;
    digitalWrite(ledOut, state);    // Light led.
    number = 0;  // Clear the number, so that it does not affect the next trigger.
  }
  else
    number = 0;  // If in the setting of the interrupt time, the number of the interrupt is not reached the threshold value, it is not detected the moving objects, clear the number.
}