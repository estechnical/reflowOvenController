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

MenuItemAction::MenuItemAction (char *newName, void (*myAction)()) {	
	strncpy (this->Name, newName, 20);
	// Terminate name string if it's too long for the whole string to have fitted
	if (strlen(newName) > 20) {
		this->Name[19] = '\0';
	}
	// strcpy(this->Name, "Foo Bar");
	this->Next = NULL;
	this->Previous = NULL;
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
