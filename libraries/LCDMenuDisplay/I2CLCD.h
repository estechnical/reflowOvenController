/*
Copyright Ed Simmons 2015
ed@estechnical.co.uk
http://www.estechnical.co.uk

Wrapper library for LCD displays using the PCF8574 I2C io expander.

*/

#ifndef _I2CLCD_H_
#define _I2CLCD_H_

#include "LCDDevice.h"
#include <LiquidCrystal_I2C.h>

class I2cLCD : public LCDDevice
{
	public:
		I2cLCD(uint8_t lcd_Addr,uint8_t lcd_cols,uint8_t lcd_rows)
		{
			this->cols = lcd_cols;
			this->rows = lcd_rows;
			lcd = new LiquidCrystal_I2C(lcd_Addr, lcd_cols, lcd_rows);
		}
		~I2cLCD() {};
		
		void begin(uint8_t cols, uint8_t rows)
		{
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
		I2cLCD();
		LiquidCrystal_I2C *lcd;
		uint8_t rows;
		uint8_t cols;
};

#endif
