/************************************
*
* SubMenu Menu Item.
*
* Used to Give a submenu.
* 
*************************************
* (C) Toby Wilkinson - tobes@tobestool.net - 2213.
************************************/
#include "Arduino.h"
#include "MenuItemSubMenu.h"

MenuItemSubMenu	::MenuItemSubMenu (char *newName) {	
	strncpy (this->Name, newName, 22);
	// Terminate name string if it's too long for the whole string to have fitted
	if (strlen(newName) > 22) {
		this->Name[21] = '\0';
	}
	// strcpy(this->Name, "Foo Bar");
	this->Next = NULL;
	this->Previous = NULL;
}

void MenuItemSubMenu::addChild (MenuItem *newChild) {
	this->Child = newChild;
	this->Child->Parent = this;
}

void MenuItemSubMenu::select (MenuDisplay *controller) {
	if (this->Child != NULL) {
		controller->Current = this->Child;
	}
}
void MenuItemSubMenu::exit (MenuDisplay *controller) {
	if (this->Parent != NULL) {
		controller->Current = this->Parent;
	}
}

void MenuItemSubMenu::inc(MenuDisplay *controller) {
	return;
}

void MenuItemSubMenu::dec(MenuDisplay *controller) {
	return;
}

void MenuItemSubMenu::getValueString (char *string) {
	return;
}
