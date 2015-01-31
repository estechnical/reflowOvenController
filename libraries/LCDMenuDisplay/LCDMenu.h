/************************************
*
* Serial Menu display.
*
* Used to display the Menus over a serial port.
* 
*************************************
* (C) Toby Wilkinson - tobes@tobestool.net - 2013.
************************************/
#ifndef LCDMenu_h
#define LCDMenu_h

#include "../MenuBase/MenuBase.h"

#include "LCDDevice.h"
#include <Encoder.h>



class LCDMenu: public MenuDisplay {
public: 
	LCDMenu ();
	void setCurrent(MenuItem *newCurrent);
	void showCurrent();
	void showCurrentValue();
	void init(MenuItem *initial, LCDDevice *lcd, boolean fourkey);
	void init(MenuItem *initial, LCDDevice *lcd, boolean fourkey, uint8_t encApin, uint8_t encBpin);
	void init(MenuItem *initial, LCDDevice *lcd, boolean fourkey, uint8_t encApin, uint8_t encBpin, uint8_t encoderButtonPin);
	void poll();
	void printMenuStructure();
	uint32_t getTimeLastKeyPressed();
	
private:
	// the pins that we poll for button presses
	//unsigned int okButton, upButton, downButton, backButton;
	// actually, sod that, buttons live on portD.
	
	enum buttons{
		none,
		ok,
		back,
		up,
		down,
		stop
	};
	
	Encoder *encoder;
	
	MenuItem *head; // the pointer to the topmost menu item
	buttons pressedKey, lastKey;
	//LiquidCrystal *LCD;
	LCDDevice *LCD;
	boolean fourkeys;
	long *encoderVal;
	long lastEncoderVal;
	uint8_t encoderIncrement;
	bool *encoderButtonVal;
	uint8_t encoderButtonPin;
	uint32_t pressedAt; // used internally to time key repeat
	uint32_t timeLastKeyPress; // used to store the time of last accepted keypress, returned to user by getTimeLastKeyPressed()
	
};

#endif
