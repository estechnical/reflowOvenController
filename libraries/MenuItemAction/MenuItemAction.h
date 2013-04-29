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
	void (*TargetAction) ();
	void getValueString (char *string);
	MenuItemAction (char *newName, void (*myAction)());
	void select(MenuDisplay *controller);
	void exit(MenuDisplay *controller);
	void inc(MenuDisplay *controller);
	void dec(MenuDisplay *controller);
};

#endif
