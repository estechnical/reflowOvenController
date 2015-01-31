/*
 ESTechnical Reflow Oven Controller

 Ed Simmons 2012-2015

 http://www.estechnical.co.uk

 http://www.estechnical.co.uk/reflow-controllers/t962a-reflow-oven-controller-upgrade

 http://www.estechnical.co.uk/reflow-ovens/estechnical-reflow-oven
 */

//#define DEBUG
#define stopKeyInputPin 7

#define fanOutPin 8
#define heaterOutPin 9

String ver = "2.7"; // bump minor version number on small changes, major on large changes, eg when eeprom layout changes

#include <MemoryFree.h>

#include <MAX31855.h>
#define tcUpdateInterval 100
#define cs1 10
#define cs2 2
MAX31855 tc1(cs1, tcUpdateInterval);
MAX31855 tc2(cs2, tcUpdateInterval);

// data type for the values used in the reflow profile
struct profileValues {
  int soakTemp;
  int soakDuration;
  int peakTemp;
  int peakDuration;
  double rampUpRate;
  double rampDownRate;
} activeProfile;

#define idleTemp 50 // the temperature at which to consider the oven safe to leave to cool naturally

int fanAssistSpeed = 35; // default fan speed

#define offsetFanSpeed 481 // 30 * 16 + 1 one byte wide
#define offsetProfileNum 482 // 30 * 16 + 2 one byte wide

int profileNumber = 0;

#include <EEPROM.h>

#include <PID_v1.h>

#include <LiquidCrystal.h>
#include <ParLCD.h>

ParLCD lcd(19, 18, 17, 16, 15, 14);   

#include <Encoder.h> // needed by the menu
#include <MenuItemSelect.h>
#include <MenuItemInteger.h>
#include <MenuItemDouble.h>
#include <MenuItemAction.h>
#include <MenuItemIntegerAction.h>
#include <MenuBase.h>
#include <LCDMenu.h>
#include <MenuItemSubMenu.h>

bool redrawDisplay = true;
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

// PID variables
double Setpoint, Input, Output;

#define WindowSize 100
unsigned long windowStartTime;

unsigned long startTime, stateChangedTime = 0, lastUpdate = 0, lastDisplayUpdate = 0, lastSerialOutput = 0; // a handful of timer variables

//Define the PID tuning parameters
double Kp = 4, Ki = 0.05, Kd = 2;
double fanKp = 1, fanKi = 0.03, fanKd = 10;

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

// state machine bits
enum state {
  idle,
  rampToSoak,
  soak,
  rampToPeak,
  peak,
  rampDown,
  coolDown
} currentState = idle, lastState = idle;

boolean stateChanged = false;
boolean lastStopPinState = true;

void abortWithError(int error) {
  // set outputs off for safety.
  digitalWrite(heaterOutPin, LOW);
  digitalWrite(fanOutPin, LOW);

  lcd.clear();

  switch (error) {
    case 1:
      lcd.print(F("Temperature"));
      lcd.setCursor(0, 1);
      lcd.print(F("following error"));
      lcd.setCursor(0, 2);
      lcd.print(F("during heating"));
      break;
      /*
      case 2:
       lcd.print("Temperature");
       lcd.setCursor(0,1);
       lcd.print("following error");
       lcd.setCursor(0,2);
       lcd.print("during cooling");
       break;
       */
    case 3:
      lcd.print(F("Thermocouple input"));
      lcd.setCursor(0, 1);
      lcd.print(F("open circuit"));
      lcd.setCursor(0, 2);
      lcd.print(F("Power off &"));
      lcd.setCursor(0, 3);
      lcd.print(F("check connections"));
      break;
  }
  while (1) { // and stop forever...
  }
}

void displayTemperature(double val)
{
  char tempStr[7];
  dtostrf(val, 5, 1, tempStr);
  displayPaddedString(tempStr, 5);
  lcd.print((char)223);// degrees symbol!
  lcd.print(F("C"));
}

void displayOutputPower(unsigned int val)
{
  if (val <= 99) lcd.print(" ");
  if (val <= 9) lcd.print(" ");
  lcd.print(val);
  lcd.print(F("%"));
}

void displayRampRate(double val)
{
  char tempStr[6];
  dtostrf(val, 4, 1, tempStr);
  lcd.write((uint8_t)0); // delta symbol
  lcd.print(F("Temp "));
  displayPaddedString(tempStr, 5);
  lcd.print((char)223);// degrees symbol!
  lcd.print(F("C/S"));
}

// prints a string of the defined length
// any space left after the string has been added to the string to print is filled with spaces
void displayPaddedString(char *str, uint8_t length)
{
  uint8_t srcLength = strlen(str);
  uint8_t spaces = length - srcLength;
  lcd.print(str);
  for(uint8_t i=0; i< spaces; i++)
  {
    lcd.print(" "); 
  }
}

void displayState()
{
  switch (currentState) {
    case idle:
      displayPaddedString("Idle", 9);
      break;
    case rampToSoak:
      displayPaddedString("Ramp Up", 9);
      break;
    case soak:
      displayPaddedString("Soak", 9);
      break;
    case rampToPeak:
      displayPaddedString("Ramp Up", 9);
      break;
    case peak:
      displayPaddedString("Peak", 9);
      break;
    case rampDown:
      displayPaddedString("Ramp Down", 9);
      break;
    case coolDown:
      displayPaddedString("Cool Down", 9);
      break;
  }
}

void displayCycleDuration()
{
  char buf[5];
  sprintf(buf, "%dS", (millis() - startTime) / 1000);
  lcd.print(buf);
}

void updateDisplay()
{
  if (redrawDisplay)
  {
    lcd.clear();
    redrawDisplay = false;

    // draw all the static bits now
    lcd.setCursor(10, 1);
    lcd.print(F("Sp"));

    lcd.setCursor(0, 2);
    lcd.print(F("Heat"));

    lcd.setCursor(10, 2);
    lcd.print(F("Fan"));
  }
  lcd.setCursor(0, 0);
  displayTemperature(tc1.getTemperature());

  lcd.setCursor(8, 0);
  if (tc2.getStatus() == 0)
  {
    displayTemperature(tc2.getTemperature());
  }
  else
  {
    lcd.print(F(" ---"));
  }

  lcd.setCursor(16, 0);
  displayCycleDuration();

  lcd.setCursor(0, 1);
  displayState();

  lcd.setCursor(12, 1);
  displayTemperature(Setpoint);

  lcd.setCursor(5, 2);
  displayOutputPower((unsigned int)heaterValue);

  lcd.setCursor(14, 2);
  displayOutputPower((unsigned int)fanValue);

  lcd.setCursor(0, 3);
  displayRampRate(rampRate);
}

boolean getJumperState() {
  boolean result = false; // jumper open
  unsigned int val = analogRead(7);
  if (val < 500) result = true;
  return result;
}

bool isStopKeyPressed()
{
  return !digitalRead(stopKeyInputPin);
}

void setupMenu()
{
  boolean jumperState = getJumperState(); // open for T962(A/C) use, closed for toaster conversion kit keypad
  myMenu.init(&control, &lcd, jumperState);

  // initialise the menu strings (stored in the progmem), min and max values, pointers to variables etc
  control.init (F("Cycle start"),  &cycleStart);
  profile.init (F("Edit Profile"));
  rampUp_rate.init(F("Ramp up rate (C/S)"), &activeProfile.rampUpRate, 0.1, 5.0);
  soak_temp.init(F("Soak temp (C)"),  &activeProfile.soakTemp, 50, 180, false);
  soak_duration.init(F("Soak time (S)"), &activeProfile.soakDuration, 10, 300, false);
  peak_temp.init(F("Peak temp (C)"), &activeProfile.peakTemp, 100, 300, false);
  peak_duration.init(F("Peak time (S)"), &activeProfile.peakDuration, 5, 60, false);
  rampDown_rate.init(F("Ramp down rate (C/S)"), &activeProfile.rampDownRate, 0.1, 10);
  profileLoad.init(F("Load Profile"), &loadProfile, &profileNumber, F("Select Profile"), 0, 29, true);
  profileSave.init(F("Save Profile"), &saveProfile, &profileNumber, F("Select Profile"), 0, 29, true);
  fan_control.init(F("Fan settings"));
  idle_speed.init(F("Idle speed"),  &fanAssistSpeed, 0, 70, false);
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


  control.addItem(&profileLoad);
  control.addItem(&profileSave);

  // fan speed control
  control.addItem(&fan_control);
  fan_control.addChild(&idle_speed);
  idle_speed.addItem(&save_fan_speed);

  //factory reset function
  control.addItem(&factory_reset);

  // set up the LCD's number of columns and rows:
  lcd.begin(20, 4);

  byte deltaChar[8] = {
    0b00000,
    0b00100,
    0b00100,
    0b01010,
    0b01010,
    0b10001,
    0b11111,
    0b00000
  };
  lcd.createChar(0, deltaChar);
}

void displaySplash()
{
  lcd.print(" ESTechnical.co.uk");
  lcd.setCursor(0, 1);
  lcd.print(" Reflow controller");
  lcd.setCursor(7, 2);
  lcd.print("v");
  lcd.print(ver);
}

void setup()
{
  Serial.begin(57600);
  
  setupMenu();
  
  lcd.clear();

  if (firstRun()) {
    factoryReset();
    loadParameters(0);
  }
  else {
    loadLastUsedProfile();
  }

  loadFanSpeed();
  
  displaySplash(); // nothing else modifies the LCD display until the loop runs...

  tc1.setup();
  tc2.setup();
  tc1.update();
  tc2.update();

  pinMode(fanOutPin, OUTPUT);
  pinMode(heaterOutPin, OUTPUT);

  PID.SetOutputLimits(0, WindowSize);
  //turn the PID on
  PID.SetMode(AUTOMATIC);

  if (tc1.getStatus() != 0) {
    abortWithError(3);
  }
  runningTotalRampRate = tc1.getTemperature() * NUMREADINGS;
  // not sure this is needed in this form any more...
  for (int i = 0; i < NUMREADINGS; i++) {
    airTemp[i] = tc1.getTemperature();
  }
  
  while ((millis() < 5000) && (isStopKeyPressed() == false)) {}; // interruptible delay to show the splash screen
  
  startTime = millis();
  myMenu.showCurrent();

}


void loop()
{

  if (millis() - lastSerialOutput > 250) {
    lastSerialOutput = millis();
    sendSerialUpdate();
  }

  if (millis() - lastUpdate >= 100) {
    stateChanged = false;
    lastUpdate = millis();

    tc1.update();
    tc2.update();

    if (tc1.getStatus() != 0) {
      abortWithError(3);
    }

    // need to keep track of a few past readings in order to work out rate of rise
    for (int i = 1; i < NUMREADINGS; i++) { // iterate over all previous entries, moving them backwards one index
      airTemp[i - 1] = airTemp[i];
    }
    airTemp[NUMREADINGS - 1] = tc1.getTemperature(); // update the last index with the newest average
    rampRate = (airTemp[NUMREADINGS - 1] - airTemp[0]); // subtract earliest reading from the current one
    // this gives us the rate of rise in degrees per polling cycle time/ num readings

    Input = tc1.getTemperature(); // update the variable the PID reads
    //Serial.print("Temp1= ");
    //Serial.println(readings[index]);

    // if the state has changed, set the flags and update the time of state change
    if (currentState != lastState) {
      lastState = currentState;
      stateChanged = true;
      stateChangedTime = millis();
    }

    if (currentState == idle) {
      if (stateChanged)
      {
        myMenu.showCurrent();
      }
      myMenu.poll();
    }
    else
    {
      if (millis() - lastDisplayUpdate > 250) { // 4hz display during reflow cycle
        lastDisplayUpdate = millis();
        updateDisplay();
      }
    }

    // check for the stop key being pressed

    boolean stopPin = digitalRead(7); // check the state of the stop key
    if (stopPin == LOW && lastStopPinState != stopPin) { // if the state has just changed
      if (currentState == coolDown) {
        currentState = idle;
      }
      else if (currentState != idle) {
        currentState = coolDown;
      }
    }
    lastStopPinState = stopPin;



    switch (currentState) {
      case idle:
        //Serial.println("Idle");
        break;

      case rampToSoak:
        //Serial.println("ramp");
        if (stateChanged) {
          PID.SetMode(MANUAL);
          Output = 50;
          PID.SetMode(AUTOMATIC);
          PID.SetControllerDirection(DIRECT);
          PID.SetTunings(Kp, Ki, Kd);
          Setpoint = airTemp[NUMREADINGS - 1];
          stateChanged = false;
        }
        Setpoint += (activeProfile.rampUpRate / 10); // target set ramp up rate

        if (Setpoint >= activeProfile.soakTemp - 1) {
          currentState = soak;
        }
        break;

      case soak:
        if (stateChanged) {
          Setpoint = activeProfile.soakTemp;
          stateChanged = false;
        }
        if (millis() - stateChangedTime >= (unsigned long) activeProfile.soakDuration * 1000) {
          currentState = rampToPeak;
        }
        break;

      case rampToPeak:
        if (stateChanged) {
          stateChanged = false;
        }

        Setpoint += (activeProfile.rampUpRate / 10); // target set ramp up rate

        if (Setpoint >= activeProfile.peakTemp - 1) { // seems to take arodun 8 degrees rise to tail off to 0 rise
          Setpoint = activeProfile.peakTemp;
          currentState = peak;
        }
        break;

      case peak:
        if (stateChanged) {
          Setpoint = activeProfile.peakTemp;
          stateChanged = false;
        }

        if (millis() - stateChangedTime >= (unsigned long) activeProfile.peakDuration * 1000) {
          currentState = rampDown;
        }
        break;

      case rampDown:
        if (stateChanged) {
          PID.SetControllerDirection(REVERSE);
          PID.SetTunings(fanKp, fanKi, fanKd);
          stateChanged = false;
          Setpoint = activeProfile.peakTemp - 15; // get it all going with a bit of a kick! v sluggish here otherwise, too hot too long
        }

        Setpoint -= (activeProfile.rampDownRate / 10);

        if (Setpoint <= idleTemp) {
          currentState = coolDown;
        }
        break;

      case coolDown:
        if (stateChanged) {
          PID.SetControllerDirection(REVERSE);
          PID.SetTunings(fanKp, fanKi, fanKd);
          Setpoint = idleTemp;
        }
        if (Input < (idleTemp + 5)) {
          currentState = idle;
          PID.SetMode(MANUAL);
          Output = 0;
        }
        break;
    }
  }

  // safety check that we're not doing something stupid.
  // if the thermocouple is wired backwards, temp goes DOWN when it increases
  // during cooling, the t962a lags a long way behind, hence the hugely lenient cooling allowance.

  // both of these errors are blocking and do not exit!
  if (Setpoint > Input + 50) abortWithError(1); // if we're 50 degree cooler than setpoint, abort
  //if(Input > Setpoint + 50) abortWithError(2);// or 50 degrees hotter, also abort

  PID.Compute();

  if (currentState != rampDown && currentState != coolDown && currentState != idle) { // decides which control signal is fed to the output for this cycle
    heaterValue = Output;
    fanValue = fanAssistSpeed;
  }
  else {
    heaterValue = 0;
    fanValue = Output;
  }

  if (millis() - windowStartTime > WindowSize)
  { //time to shift the Relay Window
    windowStartTime += WindowSize;
  }

  if (heaterValue < millis() - windowStartTime) {
    digitalWrite(heaterOutPin, LOW);
  }
  else {
    digitalWrite(heaterOutPin, HIGH);
  }

  if (fanValue < millis() - windowStartTime) {
    digitalWrite(fanOutPin, LOW);
  }
  else {
    digitalWrite(fanOutPin, HIGH);
  }
}



void cycleStart() {

  startTime = millis();
  currentState = rampToSoak;
  lcd.clear();
  lcd.print("Starting cycle ");
  lcd.print(profileNumber);
  delay(1000);
  redrawDisplay = true;

}

void saveProfile(unsigned int targetProfile) {
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

void loadProfile(unsigned int targetProfile) {
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


void saveParameters(unsigned int profile) {

  unsigned int offset = 0;
  if (profile != 0) offset = profile * 16;


  EEPROM.write(offset, lowByte(activeProfile.soakTemp));
  offset++;
  EEPROM.write(offset, highByte(activeProfile.soakTemp));
  offset++;

  EEPROM.write(offset, lowByte(activeProfile.soakDuration));
  offset++;
  EEPROM.write(offset, highByte(activeProfile.soakDuration));
  offset++;

  EEPROM.write(offset, lowByte(activeProfile.peakTemp));
  offset++;
  EEPROM.write(offset, highByte(activeProfile.peakTemp));
  offset++;

  EEPROM.write(offset, lowByte(activeProfile.peakDuration));
  offset++;
  EEPROM.write(offset, highByte(activeProfile.peakDuration));
  offset++;

  int temp = activeProfile.rampUpRate * 10;
  EEPROM.write(offset, (temp & 255));
  offset++;
  EEPROM.write(offset, (temp >> 8) & 255);
  offset++;

  temp = activeProfile.rampDownRate * 10;
  EEPROM.write(offset, (temp & 255));
  offset++;
  EEPROM.write(offset, (temp >> 8) & 255);
  offset++;

}

void loadParameters(unsigned int profile) {
  unsigned int offset = 0;
  if (profile != 0) offset = profile * 16;


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
  activeProfile.rampUpRate = ((double)temp / 10);

  temp = EEPROM.read(offset);
  offset++;
  temp |= EEPROM.read(offset) << 8;
  offset++;
  activeProfile.rampDownRate = ((double)temp / 10);

}


boolean firstRun() { // we check the whole of the space of the 16th profile, if all bytes are 255, we are doing the very first run
  unsigned int offset = 16;
  for (unsigned int i = offset * 15; i < (offset * 15) + 16; i++) {
    if (EEPROM.read(i) != 255) return false;
  }
  lcd.clear();
  lcd.print("First run...");
  delay(500);
  return true;
}

void factoryReset() {
  // clear any adjusted settings first, just to be sure...
  activeProfile.soakTemp = 130;
  activeProfile.soakDuration = 30;
  activeProfile.peakTemp = 220;
  activeProfile.peakDuration = 10;

  activeProfile.rampUpRate = 0.80;
  activeProfile.rampDownRate = 3.0;
  lcd.clear();
  lcd.print("Resetting...");

  // then save the same profile settings into all slots
  for (int i = 0; i < 30; i++) {
    saveParameters(i);
  }
  fanAssistSpeed = 35;
  saveFanSpeed();
  profileNumber = 0;
  saveLastUsedProfile();
  delay(500);
}

void saveFanSpeed() {
  unsigned int temp = (unsigned int) fanAssistSpeed;
  EEPROM.write(offsetFanSpeed, (temp & 255));
  //Serial.print("Saving fan speed :");
  //Serial.println(temp);
  lcd.clear();
  lcd.print("Saving...");
  delay(250);

}

void loadFanSpeed() {
  unsigned int temp = 0;
  temp = EEPROM.read(offsetFanSpeed);
  fanAssistSpeed = (int) temp;
  //Serial.print("Loaded fan speed :");
  //Serial.println(fanAssistSpeed);
}

void saveLastUsedProfile() {
  unsigned int temp = (unsigned int) profileNumber;
  EEPROM.write(offsetProfileNum, (temp & 255));
  //Serial.print("Saving active profile number :");
  //Serial.println(temp);

}

void loadLastUsedProfile() {
  unsigned int temp = 0;
  temp = EEPROM.read(offsetProfileNum);
  profileNumber = (int) temp;
  //Serial.print("Loaded last used profile number :");
  //Serial.println(temp);
  loadParameters(profileNumber);
}

void sendSerialUpdate()
{

  if (currentState == idle)
  {
    Serial.print("0,0,0,0,0,");
    Serial.print(tc1.getTemperature());
    Serial.print(",");
    if (tc2.getStatus() == 0)
    {
      Serial.print(tc2.getTemperature());
    }
    else
    {
      Serial.print("999");
    }
    Serial.println();
  }
  else
  {

    Serial.print(millis() - startTime);
    Serial.print(",");
    Serial.print((int)currentState);
    Serial.print(",");
    Serial.print(Setpoint);
    Serial.print(",");
    Serial.print(heaterValue);
    Serial.print(",");
    Serial.print(fanValue);
    Serial.print(",");
    Serial.print(tc1.getTemperature());
    Serial.print(",");
    if (tc2.getStatus() == 0)
    {
      Serial.print(tc2.getTemperature());
    }
    else
    {
      Serial.print("999");
    }
    Serial.println();
  }
}
