/************************************
*
* Serial Menu display.
*
* Used to display the Menus over a serial port.
* 
*************************************
* (C) Toby Wilkinson - tobes@tobestool.net - 2013.
************************************/
#ifndef MenuItemInteger_h
#define MenuItemInteger_h

#include "../MenuBase/MenuBase.h"
//#include <WProgram.h>

class MenuItemInteger: public MenuItem {
public: 
	MenuItemInteger (void);
	MenuItemInteger (const char *newName, int *targetInt, const int min, const int max, const bool rollOver);
	MenuItemInteger (const char *newName, int *targetInt, const int min , const int max, const bool rollover, const bool liveUpdate);
	void init(const char *newName, int *targetInt, const int min, const int max, const bool rollOver);
	void init(const __FlashStringHelper *newName_P, int *targetInt, const int min, const int max, const bool rollOver);
	void init(const __FlashStringHelper *newName_P, int *targetInt, const int min, const int max, const bool rollOver, const bool liveUpdate);
	void getValueString (char *);
	void select(MenuDisplay *controller);
	void exit(MenuDisplay *controller);
	void inc(MenuDisplay *controller);
	void dec(MenuDisplay *controller);
	void printItemInfo();

private:
	int *TargetInteger;
	int myMin,myMax;
	int CurrentValue;
	bool rollOver;
	bool liveUpdate;
};

#endif
