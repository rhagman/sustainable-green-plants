/*
 * Work in progress, !!!bugg tDelay after a month and you change the date.!!!
 *                   !!!bugg fanStatus overwrite tempcheck > 30 only once every minute.!!!
 */
#include <DHT.h>        // For the DHT11 temperature and humidity sensor
#include <Time.h>       // For timekeeping
#include <Wire.h>       // I2C library
#include <avr/eeprom.h> // library to save calibration and configration information in EEPROM


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
const int measureInterval = 60;


// Define DHT11 sensor
#define DHTPIN 8         // what pin we're connected to
#define DHTTYPE DHT11    // DHT 11 
DHT dht(DHTPIN, DHTTYPE);// make the object


//----- pH probe code
#define Write_Check      0x1234
#define ADDRESS 0x4C // MCP3221 A5 in Dec 77 A0 = 72 A7 = 79)
                     // A0 = x48, A1 = x49, A2 = x4A, A3 = x4B,
                     // A4 = x4C, A5 = x4D, A6 = x4E, A7 = x4F
// Our parameter, for ease of use and eeprom access lets use a struct
struct parameters_T
{
  unsigned int WriteCheck;
  int pH7Cal, pH4Cal;
  float pHStep;
}
params;
 
float pH;
int adc_result;           // Used in function phProbe and case C for calibration of the probe
const float vRef = 4.096; // Our vRef into the ADC wont be exact Since you can run VCC lower than Vref its best to measure and adjust here
const float opampGain = 5.25; // What is our Op-Amps gain (stage 1)
//----- End pH probe code


// User schedule of all settings (7 settings, 1-9 predefined settinggroups, 1-7 days with 1h resolution)
int num_usr_settings = 9;     // number of elements in userSch[x][]
int NUM_SETTINGS = 7;         // number of elements in usrSch[][x]
int num_usr_settings_day = 7; // number of elements in usrSchInd[x][]
int NUM_SETTINGS_DAY = 24;    // number of elements in usrSchInd[][x]
char* userSchedule[] = {"0,0,0,0,20,2,0","25,0,0,0,20,2,0","25,25,0,0,20,2,0","25,25,25,0,20,2,0","50,50,50,0,20,2,0","100,100,100,0,20,2,0","100,100,100,1,20,2,0","100,100,100,0,20,2,0","100,100,100,1,20,2,0"};
int usrSch[9][7] = {{0,0,0,0,20,2,0},
                    {25,0,0,0,20,2,0},
                    {25,25,0,0,20,2,0},
                    {25,25,25,0,20,2,0},
                    {50,50,50,0,20,2,0},
                    {100,100,100,0,20,2,0},
                    {100,100,100,1,20,2,0},
                    {100,100,100,0,20,2,0},
                    {100,100,100,1,20,2,0}};
int usrSchInd[7][24] = {{0,0,0,0,1,2,3,4,5,6,5,5,5,5,5,5,4,3,2,1,0,0,0,0},
                        {0,0,0,0,1,2,3,4,5,6,5,5,5,5,5,5,4,3,2,1,0,0,0,0},
                        {0,0,0,0,1,2,3,4,5,6,5,5,5,5,5,5,4,3,2,1,0,0,0,0},
                        {0,0,0,0,1,2,3,4,5,6,5,5,5,5,5,5,4,3,2,1,0,0,0,0},
                        {0,0,0,0,1,2,3,4,5,6,5,5,5,5,5,5,4,3,2,1,0,0,0,0},
                        {0,0,0,0,1,2,3,4,5,6,5,5,5,5,5,5,4,3,2,1,0,0,0,0},
                        {0,0,0,0,1,2,3,4,5,6,5,5,5,5,5,5,4,3,2,1,0,0,0,0}};

//Variables for RGB levels
int rval = 0;
int gval = 0;
int bval = 0;

//Variable for pump
int pumpOn = 0; //0 off, 1 on
int nozzleInterval = 20; //min interval between nozzle activation >60 == off
int nozzleDuration = 1; //2 sec active nozzle <0 == off
int fanStatus = 0; //0 off, 1 on

//Run preprogrammed setup, oneReport after nozzle on
int runProgram = 0;
int oneReport = 0;

//Time
time_t t = 0;
time_t tDelay = runInterval;
time_t tDelayMin = measureInterval;

//Sensor data
int sensorTempData[] = {0,0,0,0,0}; // temp, humid, light, ph, phTwoDecPlaces

void setup()
{
  Wire.begin();       // conects I2C
  Serial.begin(9600); //Serial port at 9600 baud
  dht.begin();        //Start the dht object
  
  //Set pins as outputs
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);
  pinMode(PUMP, OUTPUT);
  pinMode(NOZZLE, OUTPUT);
  pinMode(FAN, OUTPUT);
  pinMode(BOARDLED, OUTPUT);
  digitalWrite(BOARDLED, LOW);
  
  // Read info from eeprom, contains calibration values from before. If this is the first run time, use default probe settings.
  eeprom_read_block(&params, (void *)0, sizeof(params));
  if (params.WriteCheck != Write_Check){
    reset_Params();
  }
}

void loop()
{
  t = now();
  
  while (Serial.available() > 0){ //Keep working as long as data is in the buffer
    serialControl();
  }
  
  if(t >= tDelay)
  {
    if(runProgram == 1)
    {
      scheduleFromUser();
      setLight();
      if(minute(t) == 0){ //Only run the pump for 1 min when scheduled for an hour
        setPump();
      }
      if(minute(t) > 0 && pumpOn == 1){
        pumpOn = 0;
        setPump();
      }
      setFan(fanStatus);
    }
    if(nozzleInterval-1 == minute(t) % nozzleInterval)
    {
      if(nozzleDuration > second(t))
      {
        digitalWrite(NOZZLE, HIGH);
        oneReport = 1;
      }
      if(nozzleDuration <= second(t))
      {
        digitalWrite(NOZZLE, LOW);
        if(oneReport == 1){
          oneReport = 0;
          serialReport();
        }
      }
    }
    if(t >= tDelayMin)
    {
      readSensors();
      if(sensorTempData[0] >= 30 || sensorTempData[1] >= 50)
      {
        setFan(1);
      }
      if(sensorTempData[0] < 30 && sensorTempData[1] < 50)
      {
        setFan(0);
      }
      tDelayMin = t + measureInterval;
    }
    tDelay = t + runInterval;
  }
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
      case 'P':
      {
        //Lets read in our parameters and spit out the info!
         eeprom_read_block(&params, (void *)0, sizeof(params));
         Serial.print("pH 7 cal: ");
         Serial.print(params.pH7Cal);
         Serial.print(" | ");
         Serial.print("pH 4 cal: ");
         Serial.print(params.pH4Cal);
         Serial.print(" | ");
         Serial.print("pH probe slope: ");
         Serial.print(params.pHStep);
         Serial.print(" | ");
         Serial.print("pH: ");
         Serial.println(pH);
         break;
      }
      case 'C':
      {
        //Which range?
        int calrange;
        calrange = Serial.parseInt();
        if( calrange == 4 ) calibratepH4(adc_result);
        if( calrange == 7 ) calibratepH7(adc_result);
        break;
      }
    }
}

void scheduleFromUser()
{
  //Change every hour
  int x = hour(t); //0-23
  int y = day(t) % num_usr_settings_day; // rotate the days
  int z = usrSchInd[y][x];
  
  rval = usrSch[z][0];
  gval = usrSch[z][1];
  bval = usrSch[z][2];
  pumpOn = usrSch[z][3];
  nozzleInterval = usrSch[z][4];
  nozzleDuration = usrSch[z][5]; 
  fanStatus = usrSch[z][6];
}

void setLight()
{
  //Set LED (value between 0 and 255)
  analogWrite(RED, rval);
  analogWrite(GREEN, gval);
  analogWrite(BLUE, bval);
}

void setPump()
{
  //Set pump (value 1 or 0)
  if (pumpOn == 1){
    digitalWrite(PUMP, HIGH);
  }
  if (pumpOn == 0){
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

void serialReport()
{
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
                                    + sensorTempData[2] + "," + sensorTempData[3] + "," 
                                    + sensorTempData[4] + reportValues;
  Serial.println(reportValues);
}

String timeReport()
{
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
  phProbe();
}

//----- pH functions
void phProbe()
{
  //This is our I2C ADC interface section
  //We'll assign 2 BYTES variables to capture the LSB and MSB(or Hi Low in this case)
  byte adc_high;
  byte adc_low;
   
  Wire.requestFrom(ADDRESS, 2);        //requests 2 bytes
  if(Wire.available())
  {
  while(Wire.available() < 2);         //while two bytes to receive
  //Set em
  adc_high = Wire.read();          
  adc_low = Wire.read();
  //now assemble them, remembering our byte maths a Union works well here as well
  adc_result = (adc_high * 256) + adc_low;
  //We have a our Raw pH reading fresh from the ADC now lets figure out what the pH is 
  calcpH(adc_result);

  sensorTempData[3] = (int) pH;
  sensorTempData[4] = ((int) (pH*100) - sensorTempData[3]*100); // Calculate the two decimals
  }
}

void calibratepH7(int calnum)
{
  params.pH7Cal = calnum;
  calcpHSlope();
  //write these settings back to eeprom
  eeprom_write_block(&params, (void *)0, sizeof(params));
}
void calibratepH4(int calnum)
{
  params.pH4Cal = calnum;
  calcpHSlope();
  //write these settings back to eeprom
  eeprom_write_block(&params, (void *)0, sizeof(params));
}
void calcpHSlope ()
{ 
  //RefVoltage * our deltaRawpH / 12bit steps *mV in V / OP-Amp gain /pH step difference 7-4
  params.pHStep = ((((vRef*(float)(params.pH7Cal - params.pH4Cal))/4096)*1000)/opampGain)/3;
}
void calcpH(int raw)
{
  float miliVolts = (((float)raw/4096)*vRef)*1000;
  float temp = ((((vRef*(float)params.pH7Cal)/4096)*1000)- miliVolts)/opampGain;
  pH = 7-(temp/params.pHStep);
}
void reset_Params(void)
{
  //Restore to default set of parameters!
  params.WriteCheck = Write_Check;
  params.pH7Cal = 2048; //assume ideal probe and amp conditions 1/2 of 4096
  params.pH4Cal = 1286; //using ideal probe slope we end up this many 12bit units away on the 4 scale
  params.pHStep = 59.16;//ideal probe slope
  eeprom_write_block(&params, (void *)0, sizeof(params)); //write these settings back to eeprom
}
//----- End pH functions

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
