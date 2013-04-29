/************************************
*
* Serial Menu display.
*
* Used to display the Menus over a serial port.
* 
*************************************
* (C) Toby Wilkinson - tobes@tobestool.net - 2013.
************************************/
#ifndef MenuItemSelect_h
#define MenuItemSelect_h

#include "../MenuBase/MenuBase.h"
//#include <WProgram.h>

class MenuItemSelectOption {
public: 
	int Value;
	char Name[20];
	MenuItemSelectOption *Next;
};

class MenuItemSelect: public MenuItem {
public: 
	int *TargetInteger;
	MenuItemSelectOption *RootOption;
	MenuItemSelectOption *CurrentOption;
	MenuItemSelect (char *newName, int *targetInt);
	void AddOption (char *Name, int Value);
	void getValueString (char *);
	void select(MenuDisplay *controller);
	void exit(MenuDisplay *controller);
	void inc(MenuDisplay *controller);
	void dec(MenuDisplay *controller);
	
};

#endif
