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
#include "MenuItemSubMenu.h"

MenuItem::MenuItem () {
	/* Initialise generic menu item. */
	this->HelpText = NULL;
}

void MenuItem::addItem (MenuItem *newItem) {
	/* Adds a new item to the end of a menu chain. If this is the last item in 
		the chain at the moment, it's added as the next element, otherwise it's 
		handed on to the next item in the chain. */
	if (this->Next == NULL) {
		this->Next = newItem;
		this->Next->Next = NULL;
		this->Next->Previous = this;		
		newItem->Parent = this->Parent;
	} else {
		this->Next->addItem (newItem);
	}
}

void MenuDisplay::moveToNext () {	
	/* Move to the next item in the menu, if there are any. Otherwise rewind to beginning. */
	if (this->Current->Next == NULL) {
		// At End of list - Rewind to beginning.
		while (this->Current->Previous != NULL) {
			this->Current = this->Current->Previous;
		}
	} else {
		this->Current = this->Current->Next;
	}		
}

void MenuDisplay::moveToPrev () {	
	/* Move to the previous item in the menu, if there are any. Otherwise wind to last. */
	if (this->Current->Previous == NULL) {
		// At Start of list - Wind to end.
		while (this->Current->Next != NULL) {
			this->Current = this->Current->Next;
		}
	} else {
		this->Current = this->Current->Previous;
	}		
}