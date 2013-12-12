/************************************
*
* SubMenu Menu Item.
*
* Used to Give a submenu.
* 
*************************************
* (C) Toby Wilkinson - tobes@tobestool.net - 2013.
************************************/
#include "Arduino.h"
#include "MenuItemAction.h"

MenuItemAction::MenuItemAction () {	
	this->Name = NULL;
	this->Name_P = NULL;
	this->Next = NULL;
	this->Previous = NULL;
}

MenuItemAction::MenuItemAction (const char *newName, void (*myAction)()) {	
	MenuItemAction();
	this->Name = newName;
	this->Name_P = NULL;
	this->Next = NULL;
	this->Previous = NULL;
	TargetAction = myAction;
}

void MenuItemAction::init (const char *newName, void (*myAction)()) {	
	this->Name = newName;
	TargetAction = myAction;
}

void MenuItemAction::init (const __FlashStringHelper *newName_P, void (*myAction)()) {	
	this->Name_P = newName_P;
	TargetAction = myAction;
}

void MenuItemAction::select (MenuDisplay *controller) {
	(*TargetAction) ();
}

void MenuItemAction::exit (MenuDisplay *controller) {
	if (controller->Editing != NULL) {
		controller->Editing = NULL;
	} else {
		if (this->Parent != NULL) {
			controller->Current = this->Parent;
		}
	}}

void MenuItemAction::inc(MenuDisplay *controller) {
	return;
}

void MenuItemAction::dec(MenuDisplay *controller) {
	return;
}

void MenuItemAction::getValueString(char *string) { 
	string = "";
	return;
}
