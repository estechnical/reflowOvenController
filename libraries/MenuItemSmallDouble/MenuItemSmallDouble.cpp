/************************************
*
* SubMenu Menu Item.
*
* Used to Give a submenu.
* 
*************************************
* This class added by Ed Simmons - Copyright Ed Simmons 2013
* (C) Toby Wilkinson - tobes@tobestool.net - 2013.
************************************/
#include "Arduino.h"
#include "MenuItemSmallDouble.h"

MenuItemSmallDouble::MenuItemSmallDouble (void) {	
	this->Name = NULL;
	this->Name_P = NULL;
	this->Next = NULL;
	this->Previous = NULL;
	this->TargetDouble = NULL;
}

MenuItemSmallDouble::MenuItemSmallDouble (const char *newName, double *targetDouble, const double min, const double max) {	
	this->Name = newName;
	this->Name_P = NULL;
	this->Next = NULL;
	this->Previous = NULL;
	this->TargetDouble = targetDouble;
	myMin = min;
	myMax = max;
}

void MenuItemSmallDouble::init (const char *newName, double *targetDouble, const double min, const double max) {	
	this->Name = newName;
	this->TargetDouble = targetDouble;
	myMin = min;
	myMax = max;
}

void MenuItemSmallDouble::init (const __FlashStringHelper *newName_P, double *targetDouble, const double min, const double max) {	
	this->Name_P = newName_P;
	this->TargetDouble = targetDouble;
	myMin = min;
	myMax = max;
}

void MenuItemSmallDouble::select (MenuDisplay *controller) {
	if (controller->Editing == NULL) {
		controller->Editing = this;
		CurrentValue = *(this->TargetDouble);
	} else {
		*(this->TargetDouble) = CurrentValue;
		controller->Editing = NULL;
	}
	return;
}

void MenuItemSmallDouble::getValueString (char *String) {
	int fraction = (CurrentValue - (int)CurrentValue) * 100;
	if(fraction < 10){
		sprintf_P (String, (const char*)F("%d.0%d"), (int)CurrentValue, fraction);// sprintf doesn't do doubles/float on arduino
	} else {
		sprintf_P (String, (const char*)F("%d.%d"), (int)CurrentValue, fraction);// sprintf doesn't do doubles/float on arduino
	}
	//Serial.println(this->CurrentValue);
	return;
}

void MenuItemSmallDouble::exit (MenuDisplay *controller) {
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

void MenuItemSmallDouble::inc (MenuDisplay *controller) {
	if(CurrentValue < myMax)
		CurrentValue += (double)0.01;	
	return;
}

void MenuItemSmallDouble::dec (MenuDisplay *controller) {
	if(CurrentValue > myMin)
		CurrentValue -= (double)0.01;
	return;
}


