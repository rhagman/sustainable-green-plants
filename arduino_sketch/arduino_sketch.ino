#include <Arduino.h>
#include <DHT.h>        // For the DHT11 temperature and humidity sensor
#include <Time.h>       // For timekeeping
#include <Wire.h>       // For I2C

//Define LED pins (PWM pins: 3,5,6,9,10,11 on arduino UNO)
const int RED   = 11;
const int GREEN = 10;
const int BLUE  = 9;
const int PUMP  = 2;
const int NOZZLE = 12;
const int FAN = 7;
const int BOARDLED = 13; //Pin 13 and Arduino UNO board LED
const int LIGHT = A0;           // Light sensor, Analog pin 0
const int runInterval = 1;      // Define delay between the run and measure interval in seconds
const int measureInterval = 3600;


// Define DHT11 sensor
#define DHTPIN 8         // what pin we're connected to
#define DHTTYPE DHT11    // DHT 11
DHT dht(DHTPIN, DHTTYPE);// make the object

/**
 * User schedule of all settings
 * (7 settings, 1-9 predefined settinggroups, 1-7 days with 1h resolution)
 */
int num_usr_settings = 2;     // Setting sets available (Default: 2)
int num_usr_settings_day = 1; // Day setting sets available (Default: 1)
const int NUM_SETTINGS = 7;         // Day setting sets
const int NUM_SETTINGS_DAY = 24;    // Hour setting sets
// Setting sets
int usrSch[9][NUM_SETTINGS] = {
  {0,0,0,0,10,2,0}, // Spray the roots at a set interval and duration (Default)
  {0,0,0,1,10,2,0}, // Run the pump (Default)
  {25,25,0,0,20,2,0},
  {25,25,25,0,20,2,0},
  {50,50,50,0,20,2,0},
  {100,100,100,0,20,2,0},
  {100,100,100,1,20,2,0},
  {100,100,100,0,20,2,0},
  {100,100,100,1,20,2,0}
};
// Day setting sets (0:00-23:00)
int usrSchInd[NUM_SETTINGS][NUM_SETTINGS_DAY] = {
  {0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0}, // Pump at 10:00 and 20:00
  {0,0,0,0,1,2,3,4,5,6,5,5,5,5,5,5,4,3,2,1,0,0,0,0},
  {0,0,0,0,1,2,3,4,5,6,5,5,5,5,5,5,4,3,2,1,0,0,0,0},
  {0,0,0,0,1,2,3,4,5,6,5,5,5,5,5,5,4,3,2,1,0,0,0,0},
  {0,0,0,0,1,2,3,4,5,6,5,5,5,5,5,5,4,3,2,1,0,0,0,0},
  {0,0,0,0,1,2,3,4,5,6,5,5,5,5,5,5,4,3,2,1,0,0,0,0},
  {0,0,0,0,1,2,3,4,5,6,5,5,5,5,5,5,4,3,2,1,0,0,0,0}
};

// Variables for RGB levels
int rval = 0;
int gval = 0;
int bval = 0;

// Variable for pump
int pumpOn = 0; //0 off, 1 on
int nozzleInterval = 10; //min interval between nozzle activation >60 == off
int nozzleDuration = 2; //2 sec active nozzle <0 == off
int fanStatus = 0; //0 off, 1 on

// Run preprogrammed setup, oneReport after nozzle on
int runProgram = 1;
int oneReport = 0;

// Time
time_t t = 0;
time_t tDelay = runInterval;
time_t tDelayMin = measureInterval;

// Sensor data
int sensorTempData[] = {0,0,0}; // temp, humid, light

void setup() {
  Wire.begin();       // conects I2C
  Serial.begin(9600); //Serial port at 9600 baud
  dht.begin();        //Start the dht object

  // Set the time
  setTime(10,0,0,1,1,2014); // hour,min,sec,day,month,year

  // Set pins as outputs
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);
  pinMode(PUMP, OUTPUT);
  pinMode(NOZZLE, OUTPUT);
  pinMode(FAN, OUTPUT);
  pinMode(BOARDLED, OUTPUT);
  digitalWrite(BOARDLED, LOW);
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
    setLight();
    setFan(fanStatus);

    /**
     * The scheduling is per hour, but running the pump for that long is
     * unnessesary since it only takes about 30s to fill the pressure
     * accumulator.
     * Only run the pump during the first minute of the scheduled hour.
     */
    if (pumpOn == 1 && minute(t) > 0) {
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
  if (t >= tDelayMin) {
    readSensors();

    if (sensorTempData[0] >= 30 || sensorTempData[1] >= 50) {
      setFan(1);
    }
    if(sensorTempData[0] < 30 && sensorTempData[1] < 50) {
      setFan(0);
    }

    tDelayMin = t + measureInterval;
  }

  tDelay = t + runInterval;
}

/*
 * Messages consist of the letter S followed by
 * int(0-255),int(0-255),int(0-255),int(0-1),int(0-30),int(0-10),int(0-1)
 * S,255,255,255,0,20,5,0 (R,G,B,pump,nozzlieInterval,nozzleDuration,fan)
 */
void serialControl()
{
    char c = Serial.read();
    switch(c){
      case 'S':
      {
        rval   = Serial.parseInt(); //First valid integer
        gval   = Serial.parseInt(); //Second valid integer
        bval   = Serial.parseInt(); //Third valid integer
        pumpOn = Serial.parseInt(); //Fourth valid integer
        nozzleInterval = Serial.parseInt(); //Fifth valid integer
        nozzleDuration = Serial.parseInt(); //sixth valid integer
        fanStatus = Serial.parseInt(); //seventh valid integer

        setLight();
        setPump();
        setNozzle();
        setFan(fanStatus);
        break;
      }
      case 'R':
      {
        readSensors();
        serialReport();
        break;
      }
      case 'Q':
      {
        if(runProgram == 1)
        {
          runProgram = 0;
          break;
        }
        if(runProgram == 0)
        {
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
        setTime(h,m,0,d,1,2014); //hour,min,sec,day,month,year
        t = now();
        tDelay = runInterval;
        tDelayMin = measureInterval;
        break;
      }
      case 'U':
      {
        int k = 0;
        num_usr_settings = Serial.parseInt();  //The first int is the number of user settings from the user
        while(k < num_usr_settings){
          for(int i = 0; i < NUM_SETTINGS; i++){
            usrSch[k][i] = Serial.parseInt();
            Serial.print(usrSch[k][i]);
            if(i==NUM_SETTINGS-1){
              k = k++;
              Serial.println("");
            }
            else{
              Serial.print(",");
            }
          }
        }
        break;
      }
      case 'I':
      {
        int k = 0;
        num_usr_settings_day = Serial.parseInt();  //The first int is the number of user settings for a day from the user
        while(k < num_usr_settings_day){
          for(int i = 0; i < NUM_SETTINGS_DAY; i++){
            usrSchInd[k][i] = Serial.parseInt();
            Serial.print(usrSchInd[k][i]);
            if(i==NUM_SETTINGS_DAY-1){
              k = k++;
              Serial.println("");
            }
            else{
              Serial.print(",");
            }
          }
        }
        break;
      }
    }
}

void scheduleFromUser() {
  // Change every hour
  int x = hour(t); //0-23
  int y = day(t) % num_usr_settings_day; // Rotate the days
  int z = usrSchInd[y][x];

  rval = usrSch[z][0];
  gval = usrSch[z][1];
  bval = usrSch[z][2];
  pumpOn = usrSch[z][3];
  nozzleInterval = usrSch[z][4];
  nozzleDuration = usrSch[z][5];
  fanStatus = usrSch[z][6];
}

void serialReport() {
  String reportValues;

  reportValues = "R";
  reportValues = reportValues + rval;
  reportValues = reportValues + ",";
  reportValues = reportValues + gval;
  reportValues = reportValues + ",";
  reportValues = reportValues + bval;
  reportValues = reportValues + ",";
  reportValues = reportValues + pumpOn;
  reportValues = reportValues + ",";
  reportValues = reportValues + nozzleInterval;
  reportValues = reportValues + ",";
  reportValues = reportValues + nozzleDuration;
  reportValues = reportValues + ",";
  reportValues = reportValues + fanStatus;

  reportValues = timeReport() + "S" + sensorTempData[0] + "," + sensorTempData[1] + ","
                                    + sensorTempData[2] + reportValues;
  Serial.println(reportValues);
}

void setLight()
{
  //Set LED (value between 0 and 255)
  analogWrite(RED, rval);
  analogWrite(GREEN, gval);
  analogWrite(BLUE, bval);
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
  if(nozzleDuration == 0)
  {
    digitalWrite(NOZZLE, LOW);
  }
}

void setFan(int setStatus)
{
  if(setStatus == 1)
  {
    digitalWrite(FAN, HIGH);
  }
  if(setStatus == 0)
  {
    digitalWrite(FAN, LOW);
  }
}

String timeReport() {
  String timeString;

  timeString = "T";
  timeString = timeString + day(t);
  timeString = timeString + ":";
  timeString = timeString + hour(t);
  timeString = timeString + ":";
  timeString = timeString + minute(t);
  timeString = timeString + ":";
  timeString = timeString + second(t);

  return timeString;
}

void readSensors()
{
  dhtSensor();
  lightSensor();
}

void lightSensor()
{
  //int MIN_LIGHT = 200;
  //int MAX_LIGHT = 900;
  int val;

  val = analogRead(LIGHT);
  //val = map(val,MIN_LIGHT,MAX_LIGHT,0,255); // map analog 1024 -> digital pwm 256
  //val = constrain(val,0,255); // lowest and highest value of val allowed

  sensorTempData[2] = val;
}

void dhtSensor()
{
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // check if returns are valid, if they are NaN (not a number) then something went wrong!
  if (isnan(t) || isnan(h))
  {
    sensorTempData[0] = 0;
    sensorTempData[1] = 0;
  }
  else
  {
    sensorTempData[0] = (int)t;
    sensorTempData[1] = (int)h;
  }
}
