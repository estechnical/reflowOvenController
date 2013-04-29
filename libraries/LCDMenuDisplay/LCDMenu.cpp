/************************************
*
* LCD Menu display.
*
* Used to display the Menus on an LCD screen
* 
*************************************
* Based on work by Toby Wilkinson, this class extends the menu to allow LCD screen and use with four or five buttons.
* (C) Toby Wilkinson - tobes@tobestool.net - 2013.
* (C) Ed Simmons - ed@estechnical.co.uk - 2013
************************************/
#include "Arduino.h"
#include "LCDMenu.h"


LCDMenu::LCDMenu () {
	lastKey = none;
}

void LCDMenu::init (MenuItem *initial, LiquidCrystal *lcd) {
	// todo: pass the pins in for the LCD screen
	LCD = lcd;
		
	// todo: sort out how button pin mapping works
	// in the reflow controller, for speed of reading, all the buttons are on portD
	// hence can be read in one go...
		
	this->Current = initial;
	this->Editing = NULL;
}
  
void LCDMenu::showCurrent () {
	//char buffer[64];
	LCD->clear();
	LCD->print (this->Current->Name) ;
	//LCD->setCursor(0,2);
	//this->Current->getValueString(buffer);
	//LCD->print (buffer);
	LCD->setCursor(0,3);
	LCD->print ("OK  <   >   Back");
}

void LCDMenu::showCurrentValue () {
	char buffer[20];
	this->Current->getValueString(buffer);
	LCD->clear();
	LCD->print (this->Current->Name) ;
	LCD->setCursor(0,1);
	LCD->print("Editing ");
	LCD->setCursor(0,2);
	LCD->print (buffer);
	LCD->setCursor(0,3);
	LCD->print ("OK  -   +   Back");
	

}

void LCDMenu::poll () {
	// collect the state of PORTD to obtain the values of all the pins
	// keys start at PD3
	byte pd = PIND;
	pd = pd >> 3; // shift away the low three bits that we don't want
	
	// go through the five bits that correspond to keys and decide if any key is pressed (only care about the first we find)
	// keys are pulled up and debounced, low bit means key is pressed.
	
	if((pd &1) == 0){ 
		pressedKey = ok;
	} else if ((( pd >>1 ) &1) == 0){
		pressedKey = down;
	} else if (((pd >> 2) &1) == 0){
		pressedKey = up;
	} else if (((pd >> 3) &1) == 0){
		pressedKey = back;
	} else if (((pd >> 4) &1) == 0){
		pressedKey = stop;
	} else {
		pressedKey = none;
	}
	
	if(pressedKey != lastKey){
	lastKey = pressedKey;
		switch (pressedKey) {
		case up:
			if (this->Editing == NULL) {
				this->moveToNext();
			} else { 
				this->Editing->inc(this);
			}
			break;
		case down:
			if (this->Editing == NULL) {
				this->moveToPrev();
			} else {
				this->Editing->dec(this);
			}
			break;
		case ok:
			this->Current->select (this);
			break;
		case back: case stop:
			this->Current->exit (this);
			break;
		case none:
			break;
		}
	}		
	if (this->Editing == NULL) {
		this->showCurrent();
	} else {
		this->showCurrentValue();
	}
	
}

  
