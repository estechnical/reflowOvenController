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


MenuItemDouble::MenuItemDouble (char *newName, double *targetDouble, double min, double max) {	
	strncpy (this->Name, newName, 22);
	// Terminate name string if it's too long for the whole string to have fitted
	if (strlen(newName) >= 22) {
		this->Name[21] = '\0';
	}
	// strcpy(this->Name, "Foo Bar");
	this->Next = NULL;
	this->Previous = NULL;
	this->TargetDouble = targetDouble;
	myMin = min;
	myMax = max;
}

void MenuItemDouble::select (MenuDisplay *controller) {
	if (controller->Editing == NULL) {
		controller->Editing = this;
		this->CurrentValue = *(this->TargetDouble);
	} else {
		*(this->TargetDouble) = this->CurrentValue;
		controller->Editing = NULL;
	}
}

void MenuItemDouble::getValueString (char *String) {
	int fraction = (this->CurrentValue - ((int) this->CurrentValue)) * 10;
	sprintf (String, "%d.%d", (int)this->CurrentValue, fraction);// sprintf doesn't do doubles/float on arduino
	//Serial.println(this->CurrentValue);
	return;
}

void MenuItemDouble::exit (MenuDisplay *controller) {
	if (controller->Editing != NULL) {
		controller->Editing = NULL;
	} else {
		if (this->Parent != NULL) {
			controller->Current = this->Parent;
		}
	}
}

void MenuItemDouble::inc (MenuDisplay *controller) {
	this->CurrentValue += 0.10;
	if(CurrentValue > myMax) CurrentValue = myMax;
	
}

void MenuItemDouble::dec (MenuDisplay *controller) {
	this->CurrentValue -= 0.10;
	if(CurrentValue < myMin) CurrentValue = myMin;
}


