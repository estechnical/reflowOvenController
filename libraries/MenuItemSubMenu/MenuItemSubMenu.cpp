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

MenuItemSubMenu::MenuItemSubMenu () {	
	this->Name = NULL;
	this->Name_P = NULL;
	this->Next = NULL;
	this->Previous = NULL;
}

MenuItemSubMenu::MenuItemSubMenu (const char *newName) {	
	this->Name = newName;
	this->Name_P = NULL;
	this->Next = NULL;
	this->Previous = NULL;
}

void MenuItemSubMenu::init (const char *newName) {	
	this->Name = newName;
}

void MenuItemSubMenu::init (const __FlashStringHelper *newName_P)  {	
	this->Name_P = newName_P;
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
	string="";
	return;
}

void MenuItemSubMenu::printItemInfo(){
	Serial.println("Sub menu item");
	Serial.print("Name:");
	if(Name_P == NULL){
		Serial.print(Name);
	} else {
		Serial.print(Name_P);
	}
	Serial.println();

	Serial.println("Sub menu structure");
	MenuItem *temp = this->Child;
	int i = 0;
	while(temp !=NULL){
		Serial.println(i++);
		temp->printItemInfo();
		temp = temp->Next;
	}
	Serial.println("End of submenu");
}

