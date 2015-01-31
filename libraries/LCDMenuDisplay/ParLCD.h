#ifndef _PARLCD_H_
#define _PARLCD_H_

#include "LCDDevice.h"
#include <LiquidCrystal.h>

class ParLCD : public LCDDevice
{
	public:
		ParLCD(uint8_t rs, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3)
		{
			lcd = new LiquidCrystal(rs, enable, d0, d1, d2, d3);
		}
		~ParLCD() {};
		void begin(uint8_t cols, uint8_t rows)
		{
			this->cols = cols;
			this->rows = rows;
			lcd->begin(cols,rows);
		}
		void print(char c)
		{
			lcd->print(c);
		}
		void print(char * c)
		{
			lcd->print(c);
		}
		void print(const char * c)
		{
			lcd->print(c);
		}
		void print(const __FlashStringHelper * f)
		{
			lcd->print(f);
		}
		void print(String s)
		{
			lcd->print(s);
		}
		void print(int i)
		{
			lcd->print(i);
		}
		void print(unsigned int i)
		{
			lcd->print(i);
		}
		void clear()
		{
			lcd->clear();
		}
		/*
		void home();
		void noDisplay();
		void display();
		void noBlink();
		void blink();
		void noCursor();
		void cursor();
		void scrollDisplayLeft();
		void scrollDisplayRight();
		void printLeft();
		void printRight();
		void leftToRight();
		void rightToLeft();
		void shiftIncrement();
		void shiftDecrement();
		void noBacklight();
		void backlight();
		void autoscroll();
		void noAutoscroll(); 
		*/
		void createChar(uint8_t location, uint8_t charmap[])
		{
			lcd->createChar(location, charmap);
		}
		void setCursor(uint8_t col, uint8_t row)
		{
			lcd->setCursor(col,row);
		} 
		size_t write(uint8_t byte)
		{
			return lcd->write(byte);
		}
		uint8_t getRows()
		{
			return rows;
		}
		uint8_t getCols()
		{
			return cols;
		}
	private:
		ParLCD();
		LiquidCrystal *lcd;
		uint8_t rows;
		uint8_t cols;

};

#endif
