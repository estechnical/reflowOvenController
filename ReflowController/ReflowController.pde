

/*
 
 
 Ed's reflow oven controller
 
 Basic theory:
 T962A reflow oven controller is awful - this provides a drop in replacement for the t962A control.
 
 Two thermocouples are mounted in the top of the oven by the manufacturer to measure the internal temperature, these are good for measuring the air temp but don't give good results alone.
 
 Variable duty cycle control of fan.
 
 Replace the exhaust fan control with another SSR to simplify the control and allow fan PWM control.
 
 Default settings:
 
 Soak setpoint & duration (150 degrees for 120 seconds max)
 Peak setpoint & duration (ramp up to 225 degrees peak air temp and back to under 200 in less than 100 seconds)
 Ideally, time above liquidous (217 degrees) is less that 60 seconds over the whole board, but adequate heat must be supplied to properly flow all joints
 
 */

/*

 ToDo:
 make menu more sensible for choosing the profile number to save/load
 fan idle speed during a cycle should be an adjustable parameter
 remap fan speed to be more linear, 0 - 100 needs to map 0=0, 1=60,then linear to 100= 100
 
 */
// all is currently running from the first thermocouple.
// this is the front one nearest the opening of the drawer.

// the temp here is lower than reported at the board surface in the centre of the oven by about 20 degrees.
// the fan being run gently all the time made more even heat but appeared to increase the disparity between 
// thermocouple reading at the front and on the board
// the peak temp measured at the board during testing with 'sensible looking' setpoints gave a very high temp (nearly 300!)
// 185 !! peak goives good results, just hits 220 on the board
// 175 peak, does not get board up to temp at all


// this is for Albert Lim's version, it outputs a pulse on the TTL serial port to open the drawer at the beginning of ramp down
//#define OPENDRAWER 

//#define DEBUG

struct profileValues {
  int soakTemp;
  int soakDuration;
  int peakTemp;
  int peakDuration;
  double rampUpRate;
  double rampDownRate;

};

profileValues activeProfile;


int idleTemp = 50;

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

//volatile boolean cycleStart = false;

//Define the tuning parameters
double Kp=4, Ki=0.05, Kd=2;

double fanKp = 1, fanKi = 0.03, fanKd=10;

//Specify the links and initial tuning parameters
PID PID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

unsigned int fanValue, heaterValue;


//bits for keeping track of the temperature ramp
#define NUMREADINGS 10
double airTemp[NUMREADINGS];
double runningTotalRampRate; 
double rampRate = 0;

double rateOfRise = 0;

double temp1, temp2;
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


char spi_transfer(volatile char data)
{
  SPDR = data; // Start the transmission
  while (!(SPSR & (1<<SPIF))) // Wait the end of the transmission
  {
  };
  return SPDR; // return the received byte
}



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


double getTemperature(){
  // this does not do chip select for you, chip select first, then call getTemperature() for the result from the selected IC, dont' 
  //forget to release chip select when done
  // we simply read four bytes from SPI here...
  // bit 31 is temperature sign, 30-18 are 14 bit thermocouple reading, 17 is reserved, 16 is fault bit, 15 is internal reference sign,

  // 14-4 internal ref 12 bit reading, 3 reserved, 2 short to vcc bit, 1 short to gnd bit, 0 is open circuit bit 
  //(last three being set = error!!)
  // we;re being incredibly lazy and only reading the first two bytes

  uint16_t result = 0x0000;
  byte reply = 0;

  char data = 0; // dummy data to write
  //spi_transfer(data);
  reply = spi_transfer(data);
  result = reply << 8;
  reply = spi_transfer(data);
  result = result | reply;

  spi_transfer(data);
  reply = spi_transfer(data); // get the last byte, we care about the error bits
  if(reply & 1){
    abortWithError(3);

  }
  //lcd.clear();
  //lcd.print(result, BIN);
  result = (uint16_t)result >> 2;
  //lcd.clear();

  result = result * 0.25;

  return result;

}


double getAirTemperature1(){
  digitalWrite(CHIPSELECT1, LOW);
  double temp = getTemperature();
  digitalWrite(CHIPSELECT1, HIGH);
  return temp;
}

double getAirTemperature2(){
  digitalWrite(CHIPSELECT2, LOW);
  double temp = getTemperature();
  digitalWrite(CHIPSELECT2, HIGH);
  return temp;
}

boolean getJumperState(){
  boolean result = false; // jumper open
  unsigned int val = analogRead(7);
  if(val < 500) result = true;
  return result;
}

void updateDisplay(){
  lcd.clear();

  lcd.print(averageT1,1);
  lcd.print((char)223);// degrees symbol!
  lcd.print("C ");

  lcd.print(averageT2,1);
  lcd.print((char)223);// degrees symbol!
  lcd.print("C");

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



void setup()
{
  boolean state = getJumperState();
  myMenu.init(&control, &lcd, state);

  // initialise the menu strings, now stored in the progmem
  control.init (F("Cycle start"),  &cycleStart);
  profile.init (F("Edit Profile"));
  rampUp_rate.init(F("Ramp up rate (C/S)"), &activeProfile.rampUpRate, 0.1, 5.0);
  soak_temp.init(F("Soak temp (C)"),  &activeProfile.soakTemp, 50, 180,false);
  soak_duration.init(F("Soak time (S)"), &activeProfile.soakDuration,10,300,false);
  peak_temp.init(F("Peak temp (C)"), &activeProfile.peakTemp,100,300,false);
  peak_duration.init(F("Peak time (S)"), &activeProfile.peakDuration,5,60,false);
  rampDown_rate.init(F("Ramp down rate (C/S)"), &activeProfile.rampDownRate, 0.1, 10);
  profileLoadSave.init(F("Load/Save Profile"));
  profile_number.init(F("Profile number"),  &profileNumber, 0, 29,true);
  save_profile.init(F("Save"),  &saveProfile);
  load_profile.init(F("Load"),  &loadProfile);
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


  //old one
  control.addItem(&profileLoadSave);
  profileLoadSave.addChild(&profile_number);
  profile_number.addItem(&load_profile);
  load_profile.addItem(&save_profile);


  // fan speed control
  control.addItem(&fan_control);
  fan_control.addChild(&idle_speed);
  idle_speed.addItem(&save_fan_speed);

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
  } 
  else {
    loadParameters(0); // on normal startups load the first profile
  }

  loadLastUsedProfile();
  loadFanSpeed();

  // setting up SPI bus  
  digitalWrite(CHIPSELECT1, HIGH);
  digitalWrite(CHIPSELECT2, HIGH);
  pinMode(CHIPSELECT1, OUTPUT);
  pinMode(CHIPSELECT2, OUTPUT);
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

  SPCR = (1<<SPE)|(1<<MSTR)|(1<<CPHA)|(1<<SPR1)|(1<<SPR0);// SPI enable bit set, master, data valid on falling edge of clock

  clr=SPSR;
  clr=SPDR;
  delay(10);

  pinMode(8, OUTPUT);
  pinMode(9,OUTPUT);

  PID.SetOutputLimits(0, WindowSize);
  //turn the PID on
  PID.SetMode(AUTOMATIC);

  int temp = getAirTemperature1();
  runningTotalRampRate = temp * NUMREADINGS;
  for(int i =0; i<NUMREADINGS; i++){
    airTemp[i]=temp; 
  }

  myMenu.showCurrent();

  lcd.clear();
  lcd.print(" ESTechnical.co.uk");
  lcd.setCursor(0,1);
  lcd.print(" Reflow controller");
  lcd.setCursor(0,2);
  lcd.print("      v2.3");
#ifdef OPENDRAWER
  lcd.setCursor(0,3);
  lcd.print(" Albert Lim version");
#endif
  delay(7500);

}


void loop()
{

  if(millis() - lastUpdate >= 100){
#ifdef DEBUG
    Serial.print("freeMemory()=");
    Serial.println(freeMemory());
#endif
    lastUpdate = millis();

    temp1 = getAirTemperature1();
    temp2 = getAirTemperature2();
    // keep a rolling average of the temp
    totalT1 -= readingsT1[index];               // subtract the last reading
    totalT2 -= readingsT2[index];

    readingsT1[index] = temp1; // read the thermocouple
    readingsT2[index] = temp2; // read the thermocouple

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
        Serial.println(averageT2); 
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
        Serial.println(averageT2);
      }
    }


    if(currentState != lastState){
      lastState = currentState;
      stateChanged = true;
      stateChangedTime = millis();
    }
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

void saveProfile(){
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

void loadProfile(){
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
}







