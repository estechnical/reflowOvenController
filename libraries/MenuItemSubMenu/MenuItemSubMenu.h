/************************************
*
* Serial Menu display.
*
* Used to display the Menus over a serial port.
* 
*************************************
* (C) Toby Wilkinson - tobes@tobestool.net - 2013.
************************************/
#ifndef MenuItemSubMenu_h
#define MenuItemSubMenu_h

#include "../MenuBase/MenuBase.h"
//#include <WProgram.h>

class MenuItemSubMenu: public MenuItem {
public: 
	MenuItemSubMenu (char *newName);
	MenuItem *Child;
	void select(MenuDisplay *controller);
	void exit(MenuDisplay *controller);
	void inc(MenuDisplay *controller);
	void dec(MenuDisplay *controller);
	void getValueString (char *);
//	void select(MenuDisplay *controller);
//	void exit(MenuDisplay *controller);
	void addChild(MenuItem *newChild);
//	void init();
};

#endif
