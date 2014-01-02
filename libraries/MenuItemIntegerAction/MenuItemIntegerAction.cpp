/************************************
*
* MenuItemIntegerAction Menu Item.
*
* Used to Give a way to call a method with a selected value as a parameter.
* 
*************************************
* (C) Toby Wilkinson - tobes@tobestool.net - 2013.
************************************/
#include "Arduino.h"
#include "MenuItemIntegerAction.h"

MenuItemIntegerAction::MenuItemIntegerAction () {	
	this->Name = NULL;
	this->Name_P = NULL;
	this->Next = NULL;
	this->Previous = NULL;
	this->TargetAction = NULL;
}

MenuItemIntegerAction::MenuItemIntegerAction (const char *newName, void (*myAction)(unsigned int), const int min , const int max, const bool rollover) {	
	this->Name = newName;
	this->Name_P = NULL;
	this->Next = NULL;
	this->Previous = NULL;
	this->TargetAction = myAction;
	myMin = min;
	myMax = max;
	rollOver = rollover;
}

void MenuItemIntegerAction::init (const __FlashStringHelper *newName_P, void (*myAction)(unsigned int), int *myDefault, const __FlashStringHelper *helpText_P, const int min , const int max, const bool rollover) {
	this->Name_P = newName_P;
	this->HelpText = helpText_P;
	this->TargetAction = myAction;
	this->defaultInteger = myDefault;
	myMin = min;
	myMax = max;
	rollOver = rollover;
}

void MenuItemIntegerAction::select (MenuDisplay *controller) {
	if (controller->Editing == NULL) {
		controller->Editing = this;
		CurrentValue = *(this->defaultInteger);
	}
	else {
		*(this->defaultInteger) = CurrentValue;
		(*TargetAction) (CurrentValue);
		controller->Editing = NULL;
	}
}

void MenuItemIntegerAction::getValueString (char *String) {
	sprintf_P (String, (const char*)F("%d"), CurrentValue);
	return;
}

void MenuItemIntegerAction::exit (MenuDisplay *controller) {
	if (controller->Editing != NULL) {
		controller->Editing = NULL;
	} else {
		if (this->Parent != NULL) {
			controller->Current = this->Parent;
		}
	}
}

void MenuItemIntegerAction::inc (MenuDisplay *controller) {
	CurrentValue++;
	if(rollOver){
		if(CurrentValue > myMax) CurrentValue = myMin;
	} else {
		if(CurrentValue > myMax) CurrentValue = myMax;
	}
}

void MenuItemIntegerAction::dec (MenuDisplay *controller) {
	CurrentValue--;
	if(rollOver){
		if(CurrentValue < myMin) CurrentValue = myMax;
	} else {
		if(CurrentValue < myMin) CurrentValue = myMin;
	}
}
