#include <Arduino.h>
#include <TimeLib.h>  // For timekeeping

// LED pins (PWM pins: 3,5,6,9,10,11 on arduino UNO)
const int PUMP  = 4;  // Pin 4 on arduino maps to D2 on wemos
const int NOZZLE = 5; // Pin 5 on arduino maps to D1 on wemos
const int runInterval = 1;
const int pumpMaxRunTime = 9;

/**
 * Schedule of all settings
 */
int usrSch[3][3] = {
  {0,10,5}, // Spray the roots at a set interval and duration (Default)
  {1,10,5}, // Run the pump (Default)
  {0,10,5}  // Night setting (Default)
};
// Day setting set (0:00-23:00)
int usrSchInd[1][24] = {
  {2,2,2,2,2,2,2,2,2,0,0,0,1,0,0,0,0,0,0,0,0,2,2,2} // Pump at 12:00
};


// Variable for pump
int pumpOn = 0; // 0 off, 1 on
int nozzleInterval = 10; // min interval between nozzle activation >60 == off
int nozzleDuration = 5;  // sec active nozzle <0 == off

// Run preprogrammed setup, oneReport after nozzle on
int runProgram = 1;
int oneReport = 0;

// Time
time_t t = 0;
time_t tDelay = runInterval;

void setup() {
  Serial.begin(9600); // Serial port at 9600 baud

  // Set the time
  setTime(12,0,0,1,1,2014); // hour,min,sec,day,month,year

  // Set pins as outputs
  pinMode(PUMP, OUTPUT);
  pinMode(NOZZLE, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT); // Pin 13 on arduino and 2 on wemos
  digitalWrite(LED_BUILTIN, LOW);
}

void loop() {
  t = now();
  // Keep working as long as data is in the buffer
  while (Serial.available() > 0) {
    serialControl();
  }
  if (t < tDelay) {
    return;
  }

  if (runProgram == 1) {
    scheduleFromUser();

    /**
     * The scheduling is per hour, but running the pump for that long is
     * unnessesary so have a maximum run time that is less than 60 min.
     */
    if (pumpOn == 1 && minute(t) > pumpMaxRunTime) {
      pumpOn = 0;
    }
    setPump();
  }
  if (nozzleInterval-1 == minute(t) % nozzleInterval) {
    if (nozzleDuration > second(t)) {
      digitalWrite(NOZZLE, HIGH);
      oneReport = 1;
    }
    if (nozzleDuration <= second(t)) {
      digitalWrite(NOZZLE, LOW);
      if (oneReport == 1) {
        oneReport = 0;
        serialReport();
      }
    }
  }

  tDelay = t + runInterval;
}

/*
 * Messages consist of the letter S followed by
 * ,int(0-1),int(0-59),int(0-59)
 * S,0,20,5 (pump,nozzlieInterval,nozzleDuration)
 */
void serialControl()
{
    char c = Serial.read();
    switch (c) {
      case 'S':
      {
        pumpOn = Serial.parseInt();
        nozzleInterval = Serial.parseInt();
        nozzleDuration = Serial.parseInt();

        setPump();
        setNozzle();
        break;
      }
      case 'R':
      {
        serialReport();
        break;
      }
      case 'Q':
      {
        if (runProgram == 1) {
          runProgram = 0;
          break;
        }
        if (runProgram == 0) {
          runProgram = 1;
          break;
        }
        break;
      }
      case 'T':
      {
        int h  = Serial.parseInt(); //First valid integer
        int m  = Serial.parseInt(); //Second valid integer
        int d  = Serial.parseInt(); //Third valid integer
        setTime(h,m,0,d,1,2014);    //hour,min,sec,day,month,year
        t = now();
        tDelay = runInterval;
        break;
      }
    }
}

void scheduleFromUser() {
  // Change every hour
  int x = hour(t); // 0-23
  int z = usrSchInd[0][x];

  pumpOn = usrSch[z][0];
  nozzleInterval = usrSch[z][1];
  nozzleDuration = usrSch[z][2];
}

/**
 * Turn the pump on or off depending on the value of the global variable pumpOn.
 */
void setPump() {
  if (pumpOn == 1) {
    digitalWrite(PUMP, HIGH);
  }
  if (pumpOn == 0) {
    digitalWrite(PUMP, LOW);
  }
}

void setNozzle()
{
  if (nozzleDuration == 0) {
    digitalWrite(NOZZLE, LOW);
  }
}

void serialReport() {
  String reportValues;

  reportValues = "Settings:\n";
  reportValues = "Pump(0=off, 1=on):      ";
  reportValues = reportValues + pumpOn;
  reportValues = reportValues + "\nMinutes between sprays: ";
  reportValues = reportValues + nozzleInterval;
  reportValues = reportValues + "\nSeconds of spray:       ";
  reportValues = reportValues + nozzleDuration;

  reportValues = "-- Report ----\n" + timeReport() + reportValues;
  Serial.println(reportValues);
}

String timeReport() {
  String timeString;

  timeString = "Time(dd:hh:mm:ss):      ";
  timeString = timeString + day(t);
  timeString = timeString + ":";
  timeString = timeString + hour(t);
  timeString = timeString + ":";
  timeString = timeString + minute(t);
  timeString = timeString + ":";
  timeString = timeString + second(t);
  timeString = timeString + "\n";

  return timeString;
}
