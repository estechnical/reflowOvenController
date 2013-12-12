/************************************
*
* SubMenu Menu Item.
*
* Used to Give a submenu.
* 
*************************************
* This class added by Ed Simmons - Copyright Ed Simmons 2213
* (C) Toby Wilkinson - tobes@tobestool.net - 2213.
************************************/
#include "Arduino.h"
#include "MenuItemDouble.h"

MenuItemDouble::MenuItemDouble () {	
	this->Name = NULL;
	this->Name_P = NULL;
	this->Next = NULL;
	this->Previous = NULL;
	this->TargetDouble = NULL;
}

MenuItemDouble::MenuItemDouble (const char *newName, double *targetDouble, const double min, const double max) {	
	this->Name = newName;
	this->Name_P = NULL;
	this->Next = NULL;
	this->Previous = NULL;
	this->TargetDouble = targetDouble;
	myMin = min;
	myMax = max;
}

void MenuItemDouble::init (const char *newName, double *targetDouble, const double min, const double max) {	
	this->Name = newName;
	this->TargetDouble = targetDouble;
	myMin = min;
	myMax = max;
}

void MenuItemDouble::init (const __FlashStringHelper *newName_P, double *targetDouble, const double min, const double max) {	
	this->Name_P = newName_P;
	this->TargetDouble = targetDouble;
	myMin = min;
	myMax = max;
}

void MenuItemDouble::select (MenuDisplay *controller) {
	if (controller->Editing == NULL) {
		controller->Editing = this;
		CurrentValue = *(this->TargetDouble);
	} else {
		*(this->TargetDouble) = CurrentValue;
		controller->Editing = NULL;
	}
	return;
}

void MenuItemDouble::getValueString (char *String) {
	int fraction = (CurrentValue - (int) CurrentValue) * 10;
	sprintf_P (String, (const char*)F("%d.%d"), (int)CurrentValue, fraction);// sprintf doesn't do doubles/float on arduino
	//Serial.println(this->CurrentValue);
	return;
}

void MenuItemDouble::exit (MenuDisplay *controller) {
	if (controller->Editing != NULL) {
		controller->Editing = NULL;
	}
	else {
		if (this->Parent != NULL) {
			controller->Current = this->Parent;
		}
	}
	return;
}

void MenuItemDouble::inc (MenuDisplay *controller) {
	CurrentValue += (double)0.1;
	if(CurrentValue > myMax)
		CurrentValue = myMax;
}

void MenuItemDouble::dec (MenuDisplay *controller) {
	CurrentValue -= (double)0.1;
	if(CurrentValue < myMin)
		(CurrentValue = myMin);
}


