/*
Copyright Ed Simmons 2015
ed@estechnical.co.uk
http://www.estechnical.co.uk

To implement different LCD displays that are compatible with the LCD menu, inherit from the LCD device virtual class.

*/

#ifndef _LCDDEVICE_H_
#define _LCDDEVICE_H_

#include <Arduino.h>

class LCDDevice
{
	public:
		LCDDevice() {};
		virtual void begin(uint8_t, uint8_t);
		virtual void print(char);
		virtual void print(char *);
		virtual void print(const char *);
		virtual void print(const __FlashStringHelper *);
		virtual void print(String);
		virtual void print(int);
		virtual void print(unsigned int);
		virtual void clear();
		/*
		virtual void home();
		virtual void noDisplay();
		virtual void display();
		virtual void noBlink();
		virtual void blink();
		virtual void noCursor();
		virtual void cursor();
		virtual void scrollDisplayLeft();
		virtual void scrollDisplayRight();
		virtual void printLeft();
		virtual void printRight();
		virtual void leftToRight();
		virtual void rightToLeft();
		virtual void shiftIncrement();
		virtual void shiftDecrement();
		virtual void noBacklight();
		virtual void backlight();
		virtual void autoscroll();
		virtual void noAutoscroll(); 
		*/
		virtual void createChar(uint8_t, uint8_t[]);
		virtual void setCursor(uint8_t, uint8_t); 
		virtual size_t write(uint8_t);
		virtual uint8_t getRows();
		virtual uint8_t getCols();
		//virtual void command(uint8_t);
	private:
		
		
};

#endif
