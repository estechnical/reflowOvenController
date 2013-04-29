/************************************
*
* Serial Menu display.
*
* Used to display the Menus over a serial port.
* 
*************************************
* (C) Toby Wilkinson - tobes@tobestool.net - 2013.
************************************/
#ifndef MenuItemSmallDouble_h
#define MenuItemSmallDouble_h

#include "../MenuBase/MenuBase.h"
//#include <WProgram.h>

class MenuItemSmallDouble: public MenuItem {
public: 
	double *TargetDouble;
	double CurrentValue;
	double myMin, myMax;
	MenuItemSmallDouble (char *newName, double *targetDouble, double min, double max);
	void getValueString (char *);
	void select(MenuDisplay *controller);
	void exit(MenuDisplay *controller);
	void inc(MenuDisplay *controller);
	void dec(MenuDisplay *controller);
	
};

#endif
