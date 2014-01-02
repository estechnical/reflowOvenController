/************************************
*
* MenuItemIntegerAction.
*
* Used to Provide a method for calling an action that requires an int parameter.
* 
*************************************
* (C) Toby Wilkinson - tobes@tobestool.net - 2013.
************************************/
#ifndef MenuItemIntegerAction_h
#define MenuItemIntegerAction_h

#include "../MenuBase/MenuBase.h"
//#include <WProgram.h>

class MenuItemIntegerAction: public MenuItem {
public: 
	MenuItemIntegerAction (void);
	MenuItemIntegerAction (const char *newName, void (*myAction)(unsigned int), const int min, const int max, const bool rollOver);
	void init(const __FlashStringHelper *newName_P, void (*myAction)(unsigned int), int *myDefault, const __FlashStringHelper *helpText_P, const int min, const int max, const bool rollOver);
	void getValueString (char *);
	void select(MenuDisplay *controller);
	void exit(MenuDisplay *controller);
	void inc(MenuDisplay *controller);
	void dec(MenuDisplay *controller);

private:
	void (*TargetAction)(unsigned int);
	int *defaultInteger;
	int myMin,myMax;
	int CurrentValue;
	bool rollOver;
};

#endif
