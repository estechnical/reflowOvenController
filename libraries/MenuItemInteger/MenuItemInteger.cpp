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
#include "MenuItemInteger.h"

MenuItemInteger::MenuItemInteger (char *newName, int *targetInt, int min , int max, bool rollover) {	
	strncpy (this->Name, newName, 20);
	// Terminate name string if it's too long for the whole string to have fitted
	if (strlen(newName) > 20) {
		this->Name[19] = '\0';
	}
	
	// strcpy(this->Name, "Foo Bar");
	this->Next = NULL;
	this->Previous = NULL;
	this->TargetInteger = targetInt;
	myMin = min;
	myMax = max;
	rollOver = rollover;
	
}

void MenuItemInteger::select (MenuDisplay *controller) {
	if (controller->Editing == NULL) {
		controller->Editing = this;
		this->CurrentValue = *(this->TargetInteger);
	} else {
		*(this->TargetInteger) = this->CurrentValue;
		controller->Editing = NULL;
	}
}

void MenuItemInteger::getValueString (char *String) {
	sprintf (String, "%d", this->CurrentValue);
	return;
}

void MenuItemInteger::exit (MenuDisplay *controller) {
	if (controller->Editing != NULL) {
		controller->Editing = NULL;
	} else {
		if (this->Parent != NULL) {
			controller->Current = this->Parent;
		}
	}
}

void MenuItemInteger::inc (MenuDisplay *controller) {
	this->CurrentValue++;
	if(rollOver){
		if(CurrentValue > myMax) CurrentValue = myMin;
	} else {
		if(CurrentValue > myMax) CurrentValue = myMax;
	}
}

void MenuItemInteger::dec (MenuDisplay *controller) {
	this->CurrentValue--;
	if(rollOver){
		if(CurrentValue < myMin) CurrentValue = myMax;
	} else {
		if(CurrentValue < myMin) CurrentValue = myMin;
	}
}
