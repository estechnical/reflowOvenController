/************************************
*
* LCD Menu display.
*
* Used to display the Menus on an LCD screen
* 
*************************************
* Based on work by Toby Wilkinson, this class extends the menu to allow LCD screen and use with four or five buttons.
* (C) Toby Wilkinson - tobes@tobestool.net - 2013.
* (C) Ed Simmons - ed@estechnical.co.uk - 2013 - 2014
************************************/
#include "Arduino.h"
#include "LCDMenu.h"

LCDMenu::LCDMenu () {
	lastKey = none;
	timeLastKeyPress = 0;
}

void LCDMenu::init (MenuItem *initial, LiquidCrystal *lcd, boolean fourkey) {
	this->LCD = lcd;
	this->fourkeys = fourkey;
	this->head = initial;
	this->Current = initial;
	this->Editing = NULL;
	this->update = true;
	this->encoder = NULL;
	this->encoderButtonPin = 0;
}

void LCDMenu::init(MenuItem *initial, LiquidCrystal *lcd, boolean fourkey, uint8_t encApin, uint8_t encBpin) {
	init (initial, lcd, fourkey);
	this->encoder = new Encoder(encApin, encBpin);
}

void LCDMenu::init(MenuItem *initial, LiquidCrystal *lcd, boolean fourkey, uint8_t encApin, uint8_t encBpin, uint8_t encoderButtonPin) {
	init (initial, lcd, fourkey, encApin, encBpin);
	this->encoderButtonPin = encoderButtonPin;
}

void LCDMenu::setCurrent(MenuItem *newCurrent){
	if(newCurrent != NULL){
		this->Current = newCurrent;
		this->Editing = NULL;
	}
	showCurrent();

}
  
void LCDMenu::showCurrent () {
	LCD->clear();
	if(this->Current->Name_P == NULL){
		LCD->print (this->Current->Name) ;
	} else {
		LCD->print (this->Current->Name_P) ;
	}
	
	LCD->setCursor(0,3);
	LCD->print ("OK  <   >   Back");
}

void LCDMenu::showCurrentValue () {
	char buffer[20];
	this->Current->getValueString(buffer);
	LCD->clear();
	if(this->Current->Name_P == NULL){
		LCD->print (this->Current->Name) ;
	} else {
		LCD->print (this->Current->Name_P) ;
	}
	LCD->setCursor(0,1);
	if(this->Current->HelpText == NULL){
		LCD->print("Editing ");
	} else {
		LCD->print (this->Current->HelpText) ;
	}
	LCD->setCursor(0,2);
	LCD->print (buffer);
	LCD->setCursor(0,3);
	LCD->print ("OK  -   +   Back");
	

}

void LCDMenu::poll () {
	// collect the state of PORTD to obtain the values of all the pins
	// keys start at PD3
#if defined (__AVR_ATmega640__)  \
|| defined (__AVR_ATmega1280__) \
|| defined (__AVR_ATmega1281__) \
|| defined (__AVR_ATmega2560__) \
|| defined (__AVR_ATmega2561__)

// Arduino Mega
	byte data = PINK;

#elif defined (__AVR_ATmega168__)  \
 || defined (__AVR_ATmega168P__) \
 || defined (__AVR_ATmega88P__)  \
 || defined (__AVR_ATmega88__)   \
 || defined (__AVR_ATmega48P__)  \
 || defined (__AVR_ATmega48__)   \
 || defined (__AVR_ATmega328P__)
 // other arduinos
 	byte data = PIND;
	data = data >> 3; // shift away the low three bits that we don't want
 
 #endif
	
	
	// go through the five bits that correspond to keys and decide if any key is pressed (only care about the first we find)
	// keys are pulled up and debounced, low bit means key is pressed.
	pressedKey = none;
	if((data & B11111) != B11111){
		if(fourkeys){
			if((data &1) == 0){ 
				pressedKey = up;
			} else if ((( data >>1 ) &1) == 0){
				pressedKey = back;
			} else if (((data >> 2) &1) == 0){
				pressedKey = ok;
			} else if (((data >> 3) &1) == 0){
				pressedKey = down;
			}
		}else{
			if((data &1) == 0){ 
				pressedKey = ok;
			} else if ((( data >>1 ) &1) == 0){
				pressedKey = down;
			} else if (((data >> 2) &1) == 0){
				pressedKey = up;
			} else if (((data >> 3) &1) == 0){
				pressedKey = back;
			} else if (((data >> 4) &1) == 0){
				pressedKey = stop;
			}
		}
	}
	if(pressedKey != lastKey && pressedKey != none){ // catch the first moment the key is pressed
	    pressedAt = millis(); // save the time this occured
	}
	
	int32_t count = 0;
	if(encoder != NULL){
		count = encoder->read();
		if(count != 0){
			encoder->write(0);
			if(count>0){
				pressedKey = up;
			} else {
				pressedKey = down;
			}
		}
	}
	
	if(encoderButtonPin != 0){
		if(digitalRead(encoderButtonPin) == LOW){
			pressedKey = ok;
		}
	}
	
	uint32_t pressedDuration = millis() - pressedAt;
	if(pressedKey != none){
	    if((pressedKey != lastKey ) || ( pressedKey == lastKey && pressedDuration >= 50 && (millis() - timeLastKeyPress >= 250)) )
	    	{ 
		    update = true;
		    lastKey = pressedKey;
		    timeLastKeyPress = millis(); // timestamp of the keypress
		    int i = 0;
		    do{
		    	i++;
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
				case back:
				case stop:
					this->Current->exit (this);
					break;
				case none:
					break;
				}
		    } while (i < abs(count));
	    }
	    if(update){
		    update = false;		
		    if (this->Editing == NULL) {
			    this->showCurrent();
		    } else {
			    this->showCurrentValue();
		    }
	    }
	}
}

void LCDMenu::printMenuStructure(){
	MenuItem *temp = head;
	Serial.println("About to print the menu structure:");
	while(temp !=NULL){
		temp->printItemInfo();
		temp = temp->Next;
	}
}

uint32_t LCDMenu::getTimeLastKeyPressed(){
    return timeLastKeyPress;
}
  
