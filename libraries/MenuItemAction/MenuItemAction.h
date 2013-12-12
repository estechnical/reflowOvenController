/************************************
*
* Serial Menu display.
*
* Used to display the Menus over a serial port.
* 
*************************************
* (C) Toby Wilkinson - tobes@tobestool.net - 2013.
************************************/
#ifndef MenuItemAction_h
#define MenuItemAction_h

#include "../MenuBase/MenuBase.h"
//#include <WProgram.h>

class MenuItemAction: public MenuItem {
public: 
	MenuItemAction (void);
	MenuItemAction (const char *newName, void (*myAction)());
	void init (const char *newName, void (*myAction)());
	void init (const __FlashStringHelper *newName_P, void (*myAction)());
	void select(MenuDisplay *controller);
	void exit(MenuDisplay *controller);
	void inc(MenuDisplay *controller);
	void dec(MenuDisplay *controller);

private:
	void (*TargetAction)();
	void getValueString(char *string);
};

#endif
