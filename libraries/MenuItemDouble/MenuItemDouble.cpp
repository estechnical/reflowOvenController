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
	this->increment = 0.1;
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

MenuItemDouble::MenuItemDouble (const char *newName, double *targetDouble, const double min, const double max, const double increment) {	
	MenuItemDouble(newName, targetDouble, min, max);
	this->increment = increment;
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

void MenuItemDouble::init (const __FlashStringHelper *newName_P, double *targetDouble, const double min, const double max, const double increment) {	
	this->Name_P = newName_P;
	this->TargetDouble = targetDouble;
	myMin = min;
	myMax = max;
	this->increment = increment;
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
	CurrentValue += increment;
	if(CurrentValue > myMax)
		CurrentValue = myMax;
}

void MenuItemDouble::dec (MenuDisplay *controller) {
	CurrentValue -= increment;
	if(CurrentValue < myMin)
		(CurrentValue = myMin);
}

void MenuItemDouble::printItemInfo(){
	Serial.println(F("This is a MenuItemDouble"));
	Serial.print((F("Name:")));
	if(Name_P == NULL){
		Serial.print(Name);
	} else {
		Serial.print(Name_P);
	}
	Serial.println();
}


