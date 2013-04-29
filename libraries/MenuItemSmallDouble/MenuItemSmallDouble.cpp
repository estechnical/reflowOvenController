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


MenuItemSmallDouble::MenuItemSmallDouble (char *newName, double *targetDouble, double min, double max) {	
	strncpy (this->Name, newName, 20);
	// Terminate name string if it's too long for the whole string to have fitted
	if (strlen(newName) > 20) {
		this->Name[19] = '\0';
	}
	// strcpy(this->Name, "Foo Bar");
	this->Next = NULL;
	this->Previous = NULL;
	this->TargetDouble = targetDouble;
	myMin = min;
	myMax = max;
}

void MenuItemSmallDouble::select (MenuDisplay *controller) {
	if (controller->Editing == NULL) {
		controller->Editing = this;
		this->CurrentValue = *(this->TargetDouble);
	} else {
		*(this->TargetDouble) = this->CurrentValue;
		controller->Editing = NULL;
	}
}

void MenuItemSmallDouble::getValueString (char *String) {
	int fraction = (this->CurrentValue - ((int) this->CurrentValue)) * 100;
	if(fraction < 10){
		sprintf (String, "%d.0%d", (int)this->CurrentValue, fraction);// sprintf doesn't do doubles/float on arduino
	} else {
		sprintf (String, "%d.%d", (int)this->CurrentValue, fraction);// sprintf doesn't do doubles/float on arduino
	}
	//Serial.println(this->CurrentValue);
	return;
}

void MenuItemSmallDouble::exit (MenuDisplay *controller) {
	if (controller->Editing != NULL) {
		controller->Editing = NULL;
	} else {
		if (this->Parent != NULL) {
			controller->Current = this->Parent;
		}
	}
}

void MenuItemSmallDouble::inc (MenuDisplay *controller) {
	this->CurrentValue += 0.01;
	if(CurrentValue > myMax) CurrentValue = myMax;
	
}

void MenuItemSmallDouble::dec (MenuDisplay *controller) {
	this->CurrentValue -= 0.01;
	if(CurrentValue < myMin) CurrentValue = myMin;
}


