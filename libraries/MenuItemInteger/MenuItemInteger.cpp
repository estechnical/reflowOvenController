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

MenuItemInteger::MenuItemInteger () {	
	this->Name = NULL;
	this->Name_P = NULL;
	this->Next = NULL;
	this->Previous = NULL;
	this->TargetInteger = NULL;
}

MenuItemInteger::MenuItemInteger (const char *newName, int *targetInt, const int min , const int max, const bool rollover) {	
	this->Name = newName;
	this->Name_P = NULL;
	this->Next = NULL;
	this->Previous = NULL;
	this->TargetInteger = targetInt;
	myMin = min;
	myMax = max;
	rollOver = rollover;
}

MenuItemInteger::MenuItemInteger (const char *newName, int *targetInt, const int min , const int max, const bool rollover, const bool liveUpdate) {
	this->liveUpdate = liveUpdate;	
	MenuItemInteger (newName, targetInt, min , max, rollover);
}

void MenuItemInteger::init (const char *newName, int *targetInt, const int min , const int max, const bool rollover) {	
	this->Name = newName;
	this->TargetInteger = targetInt;
	myMin = min;
	myMax = max;
	rollOver = rollover;
}

void MenuItemInteger::init (const __FlashStringHelper *newName_P, int *targetInt, const int min , const int max, const bool rollover) {	
	this->Name_P = newName_P;
	this->TargetInteger = targetInt;
	myMin = min;
	myMax = max;
	rollOver = rollover;
}

void MenuItemInteger::init (const __FlashStringHelper *newName_P, int *targetInt, const int min , const int max, const bool rollover, const bool liveUpdate) {	
	this->liveUpdate = liveUpdate;	
	init (newName_P, targetInt, min , max, rollover);
}

void MenuItemInteger::select (MenuDisplay *controller) {
	if (controller->Editing == NULL) {
		controller->Editing = this;
		CurrentValue = *(this->TargetInteger);
	}
	else {
		*(this->TargetInteger) = CurrentValue;
		controller->Editing = NULL;
	}
}

void MenuItemInteger::getValueString (char *String) {
	sprintf_P (String, (const char*)F("%d"), CurrentValue);
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
	CurrentValue++;
	
	if(rollOver){
		if(CurrentValue > myMax) CurrentValue = myMin;
	}
	else {
		if(CurrentValue > myMax) CurrentValue = myMax;
	}
	if(liveUpdate) *TargetInteger = CurrentValue;
}

void MenuItemInteger::dec (MenuDisplay *controller) {
	CurrentValue--;
	if(rollOver){
		if(CurrentValue < myMin) CurrentValue = myMax;
	}
	else {
		if(CurrentValue < myMin) CurrentValue = myMin;
	}
	if(liveUpdate) *TargetInteger = CurrentValue;
}

void MenuItemInteger::printItemInfo(){
	Serial.println("This is a MenuItemInteger");
	Serial.print("Name:");
	if(Name_P == NULL){
		Serial.print(Name);
	} else {
		Serial.print(Name_P);
	}
	Serial.println();
}
