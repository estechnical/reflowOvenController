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
	int *TargetInteger;
	int myMin,myMax;
	int CurrentValue;
	bool rollOver;
	MenuItemInteger (char *newName, int *targetInt, int min, int max, bool rollOver);
	void getValueString (char *);
	void select(MenuDisplay *controller);
	void exit(MenuDisplay *controller);
	void inc(MenuDisplay *controller);
	void dec(MenuDisplay *controller);
	
};

#endif
