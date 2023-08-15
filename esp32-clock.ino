#include <RTClib.h>
#include <Stepper.h>

// ULN2003 Motor Driver Pins
#define IN1 19
#define IN2 18
#define IN3 17
#define IN4 16

RTC_DS3231 DS3231;
RTC_DATA_ATTR int bootCount = 0;

const int stepsPerRevolution = 2048;  // change this to fit the number of steps per revolution

// initialize the stepper library
Stepper myStepper(stepsPerRevolution, IN1, IN3, IN2, IN4);

void setup()
{
  ++bootCount;
  pinMode(GPIO_NUM_33, INPUT_PULLUP);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, 0); //1 = High, 0 = Low
  
  //emulate psu button press
  if (bootCount > 1) {
    pinMode(GPIO_NUM_32, OUTPUT);
    digitalWrite(GPIO_NUM_32, HIGH);
    delay(100);
    digitalWrite(GPIO_NUM_32, LOW);
  }
  
  //Move motor
  myStepper.setSpeed(5);
  if (bootCount % 2 == 0) { //1 minute has passed
    myStepper.step(stepsPerRevolution/60);
  }

  if (bootCount % 120 == 0) { //1 hour has passed
    int stepsPerMinute = stepsPerRevolution / 60; // 1 rev = 60 minutes = 2048 steps => 34 (int) steps per minute instead of 34.133333
    int stepsPerHour = 60 * stepsPerMinute; // 34 * 60 = 2040 steps in 1 hour. => we are missing 8 steps every hour
    int missingSteps = stepsPerRevolution - stepsPerHour;  // 2048 - 2040 = 8
    myStepper.step(missingSteps);
  }

  if (DS3231.begin())
  {
    DS3231.disable32K();  //we don't need the 32K Pin, so disable it
    DS3231.writeSqwPinMode(DS3231_OFF);  // stop oscillating signals at SQW Pin, otherwise setAlarm1 will fail
    DS3231.clearAlarm(1);                // set alarm 1, 2 flag to false (so alarm 1, 2 didn't happen so far)
    DS3231.clearAlarm(2);
    DS3231.disableAlarm(2);              // turn off alarm 2 (in case it isn't off already)
    DS3231.setAlarm1(
      DS3231.now() + TimeSpan(30),   // schedule an alarm 30 seconds in the future
      DS3231_A1_Second            // this mode triggers the alarm when the seconds match
    );
  }

  esp_deep_sleep_start();
}

void loop() {
}
