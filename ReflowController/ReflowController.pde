/*

 ESTechnical Reflow Oven Controller
 
 Ed Simmons 2012-2013
 
 http://www.estechnical.co.uk
 
 http://www.estechnical.co.uk/reflow-controllers/t962a-reflow-oven-controller-upgrade
 
 http://www.estechnical.co.uk/reflow-ovens/estechnical-reflow-oven
 
 */

//#define DEBUG

String ver = "2.6"; // bump minor version number on small changes, major on large changes, eg when eeprom layout changes

// for Albert Lim's version, extra features: outputs a pulse on the TTL serial 
// port to open the drawer automatically at the beginning of ramp down
//#define OPENDRAWER 

#include "temperature.h"

tcInput A, B; // the two structs for thermocouple data

// data type for the values used in the reflow profile
struct profileValues {
  int soakTemp;
  int soakDuration;
  int peakTemp;
  int peakDuration;
  double rampUpRate;
  double rampDownRate;

};

profileValues activeProfile; // the one and only instance



int idleTemp = 50; // the temperature at which to consider the oven safe to leave to cool naturally

int fanAssistSpeed = 50; // default fan speed


// do not edit below here unless you know what you are doing!
#ifdef DEBUG
#include <MemoryFree.h>
#endif
const unsigned int offsetFanSpeed_ = 30*16+1; // one byte
const unsigned int offsetProfileNum_ = 30*16+2;//one byte


int profileNumber = 0;

boolean thermocoupleOneActive = true; // this is used to keep track of which thermocouple input is used for control

//SPI Bus
#define DATAOUT 11//MOSI
#define SPICLOCK  13//sck
#define CHIPSELECT1 10
#define CHIPSELECT2 2
byte clr;

#include <EEPROM.h>

#include <PID_v1.h>

#include <LiquidCrystal.h>

LiquidCrystal lcd(19,18,17,16,15,14);

#include <MenuItemSelect.h>
#include <MenuItemInteger.h>
#include <MenuItemDouble.h>
#include <MenuItemAction.h>
#include <MenuItemIntegerAction.h>
#include <MenuBase.h>
#include <LCDMenu.h>
#include <MenuItemSubMenu.h>

LCDMenu myMenu;

// reflow profile menu items

MenuItemAction control;

MenuItemSubMenu profile;
MenuItemDouble rampUp_rate; 
MenuItemInteger soak_temp;
MenuItemInteger soak_duration;
MenuItemInteger peak_temp;
MenuItemInteger peak_duration;
MenuItemDouble rampDown_rate;

MenuItemSubMenu profileLoadSave;
MenuItemIntegerAction profileLoad;
MenuItemIntegerAction profileSave;
MenuItemInteger profile_number;
MenuItemAction save_profile;
MenuItemAction load_profile;

MenuItemSubMenu fan_control;
MenuItemInteger idle_speed;
MenuItemAction save_fan_speed;

MenuItemAction factory_reset;

//Define Variables we'll be connecting to
double Setpoint, Input, Output;

unsigned int WindowSize = 100;
unsigned long windowStartTime;

unsigned long startTime, stateChangedTime = 0, lastUpdate = 0, lastDisplayUpdate = 0, lastSerialOutput = 0; // a handful of timer variables

//Define the PID tuning parameters
double Kp=4, Ki=0.05, Kd=2;
double fanKp = 1, fanKi = 0.03, fanKd=10;

//Specify the links and initial tuning parameters
PID PID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

unsigned int fanValue, heaterValue;

void loadProfile(unsigned int);
void saveProfile(unsigned int);

//bits for keeping track of the temperature ramp
#define NUMREADINGS 10
double airTemp[NUMREADINGS];
double runningTotalRampRate; 
double rampRate = 0;

double rateOfRise = 0; // the result that is displayed

double readingsT1[NUMREADINGS];                // the readings used to make a stable temp rolling average
double readingsT2[NUMREADINGS];
unsigned short index = 0;                            // the index of the current reading
double totalT1 = 0;                            // the running total
double totalT2 = 0;
double averageT1 = 0;                          // the average
double averageT2 = 0;

boolean lastStopPin = true; // this is a flag used to store the state of the stop key pin on the last cycle through the main loop
// if the stop key state changes, we perform an action, not EVERY time we find the key is down... this is to prevent multiple
// triggers from a single keypress


#ifdef OPENDRAWER
boolean openedDrawer=false;
#endif


// state machine bits

enum state {
  idle,
  rampToSoak,
  soak,
  rampUp,
  peak,
  rampDown,
  coolDown
};

state currentState = idle, lastState = idle;
boolean stateChanged = false;

void abortWithError(int error){
  // set outputs off for safety.
  digitalWrite(8,LOW);
  digitalWrite(9,LOW);

  lcd.clear();

  switch(error){
  case 1:
    lcd.print("Temperature"); 
    lcd.setCursor(0,1);
    lcd.print("following error");
    lcd.setCursor(0,2);
    lcd.print("during heating");
    break;
  case 2:
    lcd.print("Temperature"); 
    lcd.setCursor(0,1);
    lcd.print("following error");
    lcd.setCursor(0,2);
    lcd.print("during cooling");
    break;
  case 3:
    lcd.print("Thermocouple input"); 
    lcd.setCursor(0,1);
    lcd.print("open circuit");
    lcd.setCursor(0,2);
    lcd.print("Power off &");
    lcd.setCursor(0,3);
    lcd.print("check connections");
    break;
  }
  while(1){ // and stop forever...
  }
}

void displayThermocoupleData(struct tcInput* input){
  switch(input->stat){
  case 0:

    lcd.print(input->temperature,1);
    lcd.print((char)223);// degrees symbol!
    lcd.print("C");

    break;

  case 1:
    lcd.print("---");
    break;

  }

}

void updateDisplay(){
  lcd.clear();

  displayThermocoupleData(&A);

  lcd.print(" ");

  displayThermocoupleData(&B);

  if(currentState!=idle){
    lcd.setCursor(16,0);
    lcd.print((millis() - startTime)/1000);
    lcd.print("S");
  }

  lcd.setCursor(0,1);
  switch(currentState){
  case idle:
    lcd.print("Idle ");
    break;
  case rampToSoak:
    lcd.print("Ramp ");
    break;
  case soak:
    lcd.print("Soak ");
    break;
  case rampUp:
    lcd.print("Ramp Up ");
    break;
  case peak:
    lcd.print("Peak ");
    break;
  case rampDown:
    lcd.print("Ramp Down ");
    break;
  case coolDown:
    lcd.print("Cool Down ");
    break;
  }

  lcd.print("Sp=");
  lcd.print(Setpoint,1);
  lcd.print((char)223);// degrees symbol!
  lcd.print("C");
  lcd.setCursor(0,2);
  lcd.print("Heat=");
  lcd.print((int)heaterValue);
  lcd.setCursor(10,2);
  lcd.print("Fan=");
  lcd.print((int)fanValue);
  lcd.setCursor(0,3);
  lcd.print("Ramp=");
  lcd.print(rampRate,1);
  lcd.print((char)223);// degrees symbol!
  lcd.print("C/S");
}

boolean getJumperState(){
  boolean result = false; // jumper open
  unsigned int val = analogRead(7);
  if(val < 500) result = true;
  return result;
}

void setup()
{
  A.chipSelect = 10; // define the chip select pins for the two MAX31855 ICs
  B.chipSelect = 2;

  boolean jumperState = getJumperState(); // open for T962(A/C) use, closed for toaster conversion kit keypad
  myMenu.init(&control, &lcd, jumperState);

  // initialise the menu strings (stored in the progmem), min and max values, pointers to variables etc
  control.init (F("Cycle start"),  &cycleStart);
  profile.init (F("Edit Profile"));
  rampUp_rate.init(F("Ramp up rate (C/S)"), &activeProfile.rampUpRate, 0.1, 5.0);
  soak_temp.init(F("Soak temp (C)"),  &activeProfile.soakTemp, 50, 180,false);
  soak_duration.init(F("Soak time (S)"), &activeProfile.soakDuration,10,300,false);
  peak_temp.init(F("Peak temp (C)"), &activeProfile.peakTemp,100,300,false);
  peak_duration.init(F("Peak time (S)"), &activeProfile.peakDuration,5,60,false);
  rampDown_rate.init(F("Ramp down rate (C/S)"), &activeProfile.rampDownRate, 0.1, 10);
  profileLoad.init(F("Load Profile"), &loadProfile, &profileNumber, F("Select Profile"), 0, 29,true);
  profileSave.init(F("Save Profile"), &saveProfile, &profileNumber, F("Select Profile"), 0, 29,true);
  fan_control.init(F("Fan settings"));
  idle_speed.init(F("Idle speed"),  &fanAssistSpeed, 0, 70,false);
  save_fan_speed.init(F("Save"),  &saveFanSpeed);
  factory_reset.init(F("Factory Reset"),  &factoryReset);

  // initialise the menu structure
  control.addItem(&profile);
  profile.addChild(&rampUp_rate);
  rampUp_rate.addItem(&soak_temp);
  soak_temp.addItem(&soak_duration);
  soak_duration.addItem(&peak_temp);
  peak_temp.addItem(&peak_duration);
  peak_duration.addItem(&rampDown_rate);


  // this needs to be replaced with a better menu structure. This relies on being able to 
  // have a menu item that allows the user to choose a number then be sent to another menu item
  // toby... over to you.
  control.addItem(&profileLoad);
  control.addItem(&profileSave);
  //  profileLoadSave.addChild(&profile_number);
  //  profile_number.addItem(&load_profile);
  //  load_profile.addItem(&save_profile);


  // fan speed control
  control.addItem(&fan_control);
  fan_control.addChild(&idle_speed);
  idle_speed.addItem(&save_fan_speed);

  //factory reset function
  control.addItem(&factory_reset);


  // set up the LCD's number of columns and rows:
  lcd.begin(20, 4);


  Serial.begin(57600);

#ifdef OPENDRAWER
  pinMode(1,OUTPUT);
  digitalWrite(1,LOW);
#endif

  if(firstRun()){
    factoryReset();
    loadParameters(0);
  } 
  else {
    loadLastUsedProfile();
  }

  loadFanSpeed();

  // setting up SPI bus  
  digitalWrite(A.chipSelect, HIGH);
  digitalWrite(B.chipSelect, HIGH);
  pinMode(A.chipSelect, OUTPUT);
  pinMode(B.chipSelect, OUTPUT);
  pinMode(DATAOUT, OUTPUT);
  pinMode(SPICLOCK,OUTPUT);
  //pinMode(10,OUTPUT);
  //digitalWrite(10,HIGH); // set the pull up on the SS pin (SPI doesn't work otherwise!!)

  clr = 0;
  //The SPI control register (SPCR) has 8 bits, each of which control a particular SPI setting.

  // SPCR
  // | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |0000000000000000000
  // | SPIE | SPE | DORD | MSTR | CPOL | CPHA | SPR1 | SPR0 |

  // SPIE - Enables the SPI interrupt when 1
  // SPE - Enables the SPI when 1
  // DORD - Sends data least Significant Bit First when 1, most Significant Bit first when 0
  // MSTR - Sets the Arduino in master mode when 1, slave mode when 0
  // CPOL - Sets the data clock to be idle when high if set to 1, idle when low if set to 0
  // CPHA - Samples data on the falling edge of the data clock when 1, rising edge when 0'
  // SPR1 and SPR0 - Sets the SPI speed, 00 is fastest (4MHz) 11 is slowest (250KHz)

  SPCR = (1<<SPE)|(1<<MSTR)|(1<<CPHA);// SPI enable bit set, master, data valid on falling edge of clock

  clr=SPSR;
  clr=SPDR;
  delay(10);

  pinMode(8, OUTPUT);
  pinMode(9,OUTPUT);

  PID.SetOutputLimits(0, WindowSize);
  //turn the PID on
  PID.SetMode(AUTOMATIC);

  readThermocouple(&A);

  if(A.stat !=0){
    abortWithError(3);
  }
  runningTotalRampRate = A.temperature * NUMREADINGS;
  for(int i =0; i<NUMREADINGS; i++){
    airTemp[i]=A.temperature; 
  }


  lcd.clear();
  lcd.print(" ESTechnical.co.uk");
  lcd.setCursor(0,1);
  lcd.print(" Reflow controller");
  lcd.setCursor(0,2);
  lcd.print("      v");
  lcd.print(ver);
#ifdef OPENDRAWER
  lcd.setCursor(0,3);
  lcd.print(" Albert Lim version");
#endif
  delay(7500);

  myMenu.showCurrent();

}


void loop()
{

  if(millis() - lastUpdate >= 100){
#ifdef DEBUG
    Serial.print("freeMemory()=");
    Serial.println(freeMemory());
#endif
    lastUpdate = millis();

    readThermocouple(&A);// read the thermocouple
    readThermocouple(&B);// read the thermocouple

    if(A.stat != 0){
      abortWithError(3);
    }

    // keep a rolling average of the temp
    totalT1 -= readingsT1[index];               // subtract the last reading
    totalT2 -= readingsT2[index];

    readingsT1[index] = A.temperature; 
    if(B.stat ==0){
      readingsT2[index] = B.temperature; 
    } 
    else {
      readingsT2[index] = 0;
    }

    totalT1 += readingsT1[index];               // add the reading to the total
    totalT2 += readingsT2[index]; 
    index++;                    // advance to the next index

    if (index >= NUMREADINGS)               // if we're at the end of the array...
      index = 0;                            // ...wrap around to the beginning

    averageT1 = (totalT1 / NUMREADINGS);    // calculate the average temp
    averageT2 = (totalT2 / NUMREADINGS);

    // need to keep track of a few past readings in order to work out rate of rise
    for(int i =1; i< NUMREADINGS; i++){ // iterate over all previous entries, moving them backwards one index
      airTemp[i-1] = airTemp[i];
    }


    airTemp[NUMREADINGS-1] = averageT1; // update the last index with the newest average

    rampRate = (airTemp[NUMREADINGS-1] - airTemp[0]); // subtract earliest reading from the current one
    // this gives us the rate of rise in degrees per polling cycle time/ num readings

    Input = airTemp[NUMREADINGS-1]; // update the variable the PID reads
    //Serial.print("Temp1= ");
    //Serial.println(readings[index]);


    if(currentState == idle){
      myMenu.poll();
    } 
    else {
      if(millis() - lastDisplayUpdate > 250){ // 4hz display during reflow cycle
        lastDisplayUpdate = millis();

        updateDisplay();

      }
    }

    if(millis() - lastSerialOutput > 250){
      lastSerialOutput = millis();

      if (currentState == idle)
      {
        Serial.print("0,0,0,0,0,"); 
        Serial.print(averageT1); 
        Serial.print(",");
        if(B.stat ==0){ // only print the second C data if the input is valid
          Serial.print(averageT2); 
        } 
        else {
          Serial.print("999"); 
        }
#ifdef DEBUG
        Serial.print(",");
        Serial.print(freeMemory());
#endif
        Serial.println();
      } 
      else {

        Serial.print((millis() - startTime));
        Serial.print(",");
        Serial.print((int)currentState);
        Serial.print(",");
        Serial.print(Setpoint); 
        Serial.print(",");
        Serial.print(heaterValue); 
        Serial.print(",");
        Serial.print(fanValue); 
        Serial.print(",");
        Serial.print(averageT1); 
        Serial.print(",");
        if(B.stat ==0){ // only print the second C data if the input is valid
          Serial.print(averageT2); 
        } 
        else {
          Serial.print("999"); 
        }
#ifdef DEBUG
        Serial.print(",");
        Serial.print(freeMemory());
#endif
        Serial.println();
      }
    }

    // check for the stop or back key being pressed

    boolean stopPin = digitalRead(7); // check the state of the stop key
    if(stopPin == LOW && lastStopPin != stopPin){ // if the state has just changed
      if(currentState == coolDown){
        currentState = idle;
      } 
      else if (currentState != idle) {
        currentState = coolDown;
      }
    }
    lastStopPin = stopPin;

    // if the state has changed, set the flags and update the time of state change
    if(currentState != lastState){
      lastState = currentState;
      stateChanged = true;
      stateChangedTime = millis();
    }

    switch(currentState){
    case idle:
      break;

    case rampToSoak:
      if(stateChanged){
        PID.SetMode(MANUAL);
        Output = 50;
        PID.SetMode(AUTOMATIC);
        PID.SetControllerDirection(DIRECT);
        PID.SetTunings(Kp,Ki, Kd);
        Setpoint = airTemp[NUMREADINGS-1];
        stateChanged = false;
      }    
      Setpoint += (activeProfile.rampUpRate/10); // target set ramp up rate

      if(Setpoint >= activeProfile.soakTemp - 1){
        currentState=soak;
      }
      break;

    case soak:
      if(stateChanged){
        Setpoint = activeProfile.soakTemp;
        stateChanged = false;
      }
      if(millis() - stateChangedTime >= (unsigned long) activeProfile.soakDuration*1000){
        currentState = rampUp;
      }
      break;

    case rampUp:
      if(stateChanged){
        stateChanged = false;
      }

      Setpoint += (activeProfile.rampUpRate/10); // target set ramp up rate

      if(Setpoint >= activeProfile.peakTemp - 1){ // seems to take arodun 8 degrees rise to tail off to 0 rise
        Setpoint = activeProfile.peakTemp;
        currentState = peak;
      }
      break;

    case peak:
      if(stateChanged){
        Setpoint = activeProfile.peakTemp;
        stateChanged = false;
      }

      if(millis() - stateChangedTime >= (unsigned long) activeProfile.peakDuration*1000){
        currentState = rampDown;
      }
      break;

    case rampDown:
      if(stateChanged){
        PID.SetControllerDirection(REVERSE);
        PID.SetTunings(fanKp,fanKi, fanKd);
        stateChanged = false;
        Setpoint = activeProfile.peakTemp -15; // get it all going with a bit of a kick! v sluggish here otherwise, too hot too long
      }

#ifdef OPENDRAWER
      if(!openedDrawer){
        openedDrawer=true;
        digitalWrite(1,HIGH);
        delay(5);
        digitalWrite(1,LOW);
      }
#endif

      Setpoint -= (activeProfile.rampDownRate/10); 

      if(Setpoint <= idleTemp){
        currentState = coolDown;
        //PID.SetControllerDirection(DIRECT); // flip the PID the right way up again
      }
      break;

    case coolDown:
      if(stateChanged){
        PID.SetControllerDirection(REVERSE);
        PID.SetTunings(fanKp,fanKi, fanKd);
        Setpoint = idleTemp;
      }
      if(Input < (idleTemp+5)){
        currentState = idle;
        PID.SetMode(MANUAL);
        Output =0;
      }
    }
  }

  // safety check that we're not doing something stupid. 
  // if the thermocouple is wired backwards, temp goes DOWN when it increases
  // during cooling, the t962a lags a long way behind, hence the hugely lenient cooling allowance.

  // both of these errors are blocking and do not exit!
#ifndef OPENDRAWER
  if(Setpoint > Input + 50) abortWithError(1);// if we're 50 degree cooler than setpoint, abort
#endif
  //if(Input > Setpoint + 50) abortWithError(2);// or 50 degrees hotter, also abort

  PID.Compute();

  //if(currentState!=idle){
  if(currentState != rampDown && currentState != coolDown && currentState != idle){ // decides which control signal is fed to the output for this cycle
    heaterValue = Output;
    fanValue = fanAssistSpeed;
  } 
  else {
    heaterValue = 0;
    fanValue = Output;
  }

  if(millis() - windowStartTime>WindowSize)
  { //time to shift the Relay Window
    windowStartTime += WindowSize;
  }

  if(heaterValue < millis() - windowStartTime){
    digitalWrite(9,LOW);
  } 
  else {
    digitalWrite(9,HIGH);
  }

  if(fanValue < millis() - windowStartTime){
    digitalWrite(8,LOW);
  }
  else{
    digitalWrite(8,HIGH);
  }
}



void cycleStart(){

  startTime = millis();
  currentState = rampToSoak;
#ifdef OPENDRAWER
  openedDrawer=false;
#endif
  lcd.clear();
  lcd.print("Starting cycle ");
  lcd.print(profileNumber);
  delay(1000);

}

void saveProfile(unsigned int targetProfile){
  profileNumber = targetProfile;
  lcd.clear();
  lcd.print("Saving profile ");
  lcd.print(profileNumber);

#ifdef DEBUG

  Serial.println("Check parameters:");
  Serial.print("idleTemp ");
  Serial.println(idleTemp);
  Serial.print("ramp Up rate ");
  Serial.println(activeProfile.rampUpRate);
  Serial.print("soakTemp ");
  Serial.println(activeProfile.soakTemp);
  Serial.print("soakDuration ");
  Serial.println(activeProfile.soakDuration);
  Serial.print("peakTemp ");
  Serial.println(activeProfile.peakTemp);
  Serial.print("peakDuration ");
  Serial.println(activeProfile.peakDuration);
  Serial.print("rampDownRate ");
  Serial.println(activeProfile.rampDownRate);
  Serial.println("About to save parameters");
#endif

  saveParameters(profileNumber); // profileNumber is modified by the menu code directly, this method is called by a menu action

  delay(500); 
}

void loadProfile(unsigned int targetProfile){
  // We may be able to do-away with profileNumber entirely now the selection is done in-function.
  profileNumber = targetProfile;
  lcd.clear();
  lcd.print("Loading profile ");
  lcd.print(profileNumber);
  saveLastUsedProfile();

#ifdef DEBUG

  Serial.println("Check parameters:");
  Serial.print("idleTemp ");
  Serial.println(idleTemp);
  Serial.print("ramp Up rate ");
  Serial.println(activeProfile.rampUpRate);
  Serial.print("soakTemp ");
  Serial.println(activeProfile.soakTemp);
  Serial.print("soakDuration ");
  Serial.println(activeProfile.soakDuration);
  Serial.print("peakTemp ");
  Serial.println(activeProfile.peakTemp);
  Serial.print("peakDuration ");
  Serial.println(activeProfile.peakDuration);
  Serial.print("rampDownRate ");
  Serial.println(activeProfile.rampDownRate);
  Serial.println("About to load parameters");
#endif

  loadParameters(profileNumber);

#ifdef DEBUG

  Serial.println("Check parameters:");
  Serial.print("idleTemp ");
  Serial.println(idleTemp);
  Serial.print("ramp Up rate ");
  Serial.println(activeProfile.rampUpRate);
  Serial.print("soakTemp ");
  Serial.println(activeProfile.soakTemp);
  Serial.print("soakDuration ");
  Serial.println(activeProfile.soakDuration);
  Serial.print("peakTemp ");
  Serial.println(activeProfile.peakTemp);
  Serial.print("peakDuration ");
  Serial.println(activeProfile.peakDuration);
  Serial.print("rampDownRate ");
  Serial.println(activeProfile.rampDownRate);
  Serial.println("after loading parameters");
#endif

  delay(500);
}


void saveParameters(unsigned int profile){

  unsigned int offset = 0;
  if(profile !=0) offset = profile*16;


  EEPROM.write(offset,lowByte(activeProfile.soakTemp));
  offset++;
  EEPROM.write(offset,highByte(activeProfile.soakTemp));
  offset++;

  EEPROM.write(offset,lowByte(activeProfile.soakDuration));
  offset++;
  EEPROM.write(offset,highByte(activeProfile.soakDuration));
  offset++;

  EEPROM.write(offset,lowByte(activeProfile.peakTemp));
  offset++;
  EEPROM.write(offset,highByte(activeProfile.peakTemp));
  offset++;

  EEPROM.write(offset,lowByte(activeProfile.peakDuration));
  offset++;
  EEPROM.write(offset,highByte(activeProfile.peakDuration));
  offset++;

  int temp = activeProfile.rampUpRate * 10;
  EEPROM.write(offset,(temp & 255));
  offset++;
  EEPROM.write(offset,(temp >> 8) & 255);
  offset++;

  temp = activeProfile.rampDownRate * 10;
  EEPROM.write(offset,(temp & 255));
  offset++;
  EEPROM.write(offset,(temp >> 8) & 255);
  offset++;

}

void loadParameters(unsigned int profile){
  unsigned int offset = 0;
  if(profile !=0) offset = profile*16;


  activeProfile.soakTemp = EEPROM.read(offset);
  offset++;
  activeProfile.soakTemp |= EEPROM.read(offset) << 8;
  offset++;

  activeProfile.soakDuration = EEPROM.read(offset);
  offset++;
  activeProfile.soakDuration |= EEPROM.read(offset) << 8;
  offset++;

  activeProfile.peakTemp = EEPROM.read(offset);
  offset++;
  activeProfile.peakTemp |= EEPROM.read(offset) << 8;
  offset++;

  activeProfile.peakDuration = EEPROM.read(offset);
  offset++;
  activeProfile.peakDuration |= EEPROM.read(offset) << 8;
  offset++;

  int temp = EEPROM.read(offset);
  offset++;
  temp |= EEPROM.read(offset) << 8;
  offset++;
  activeProfile.rampUpRate = ((double)temp /10);

  temp = EEPROM.read(offset);
  offset++;
  temp |= EEPROM.read(offset) << 8;
  offset++;
  activeProfile.rampDownRate = ((double)temp /10);

}


boolean firstRun(){ // we check the whole of the space of the 16th profile, if all bytes are 255, we are doing the very first run
  unsigned int offset = 16;
  for(unsigned int i = offset *15; i<(offset*15) + 16;i++){
    if(EEPROM.read(i) != 255) return false;
  }
  lcd.clear();
  lcd.print("First run...");
  delay(500);
  return true;
}

void factoryReset(){
  // clear any adjusted settings first, just to be sure...
  activeProfile.soakTemp = 130;
  activeProfile.soakDuration = 80;
  activeProfile.peakTemp = 220;
  activeProfile.peakDuration = 40; 

  activeProfile.rampUpRate = 0.80;
  activeProfile.rampDownRate = 2.0; 
  lcd.clear();
  lcd.print("Resetting...");

  // then save the same profile settings into all slots
  for(int i =0; i< 30; i++){
    saveParameters(i);
  }
  fanAssistSpeed = 50;
  saveFanSpeed();
  profileNumber = 0;
  saveLastUsedProfile();
  delay(500);
}

void saveFanSpeed(){
  unsigned int temp = (unsigned int) fanAssistSpeed;
  EEPROM.write(offsetFanSpeed_,(temp & 255));
  //Serial.print("Saving fan speed :");
  //Serial.println(temp);
  lcd.clear();
  lcd.print("Saving...");
  delay(250);

}

void loadFanSpeed(){
  unsigned int temp = 0;
  temp = EEPROM.read(offsetFanSpeed_);
  fanAssistSpeed = (int) temp;
  //Serial.print("Loaded fan speed :");
  //Serial.println(fanAssistSpeed);
}

void saveLastUsedProfile(){
  unsigned int temp = (unsigned int) profileNumber;
  EEPROM.write(offsetProfileNum_,(temp & 255));
  //Serial.print("Saving active profile number :");
  //Serial.println(temp);

}

void loadLastUsedProfile(){
  unsigned int temp = 0;
  temp = EEPROM.read(offsetProfileNum_);
  profileNumber = (int) temp;
  //Serial.print("Loaded last used profile number :");
  //Serial.println(temp);
  loadParameters(profileNumber);
}












