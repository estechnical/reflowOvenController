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
#include "MenuItemSelect.h"

MenuItemSelect::MenuItemSelect (char *newName, int *targetInt) {	
	this->Name = newName;
	this->Next = NULL;
	this->Previous = NULL;
	this->TargetInteger = targetInt;
	this->RootOption = NULL;
}

void MenuItemSelect::AddOption (char *Name, int Value) {
	MenuItemSelectOption **Pointer;
	
	Pointer = &(this->RootOption);
	while (*Pointer != NULL) {
		Pointer = &((*Pointer)->Next);
//		Serial.print ("Moving to next");
	}
	*Pointer = new MenuItemSelectOption;
	(*Pointer)->Next = NULL;
	(*Pointer)->Name = Name;
	(*Pointer)->Value = Value;
	if (this->RootOption == NULL) {
		this->RootOption = *Pointer;
		this->CurrentOption = *Pointer;
	}
}
	
void MenuItemSelect::select (MenuDisplay *controller) {
	if (controller->Editing == NULL) {
		controller->Editing = this;
		this->CurrentOption = this->RootOption;
	} else {
		*(this->TargetInteger) = this->CurrentOption->Value;
		controller->Editing = NULL;
	}
}

void MenuItemSelect::getValueString (char *String) {
	strncpy (String, this->CurrentOption->Name, 20);
}

void MenuItemSelect::exit (MenuDisplay *controller) {
	if (controller->Editing != NULL) {
		controller->Editing = NULL;
	} else {
		if (this->Parent != NULL) {
			controller->Current = this->Parent;
		}
	}}

void MenuItemSelect::inc (MenuDisplay *controller) {
	if (this->CurrentOption->Next != NULL) {
		this->CurrentOption = this->CurrentOption->Next;
	} else {
		this->CurrentOption = this->RootOption;
	}
}

void MenuItemSelect::dec (MenuDisplay *controller) {
	//this->CurrentValue--;
}
