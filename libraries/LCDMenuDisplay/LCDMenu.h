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
#include "../LiquidCrystal/LiquidCrystal.h"

class LCDMenu: public MenuDisplay {
public: 
	LCDMenu ();
	void showCurrent();
	void showCurrentValue();
	void init(MenuItem *initial, LiquidCrystal *lcd, boolean fourkey);
	void poll();
	
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
	buttons pressedKey, lastKey;
	LiquidCrystal *LCD;
	boolean fourkeys;
	
};

#endif
