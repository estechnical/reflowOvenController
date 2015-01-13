#include "MAX31855.h"

#if defined (__AVR_ATmega640__)  \
|| defined (__AVR_ATmega1280__) \
|| defined (__AVR_ATmega1281__) \
|| defined (__AVR_ATmega2560__) \
|| defined (__AVR_ATmega2561__)

// Arduino Mega
#define DATAIN 50//MISO
#define DATAOUT 51//MOSI
#define SPICLOCK  52//sck
#define SS 53

#elif defined (__AVR_ATmega168__)  \
 || defined (__AVR_ATmega168P__) \
 || defined (__AVR_ATmega88P__)  \
 || defined (__AVR_ATmega88__)   \
 || defined (__AVR_ATmega48P__)  \
 || defined (__AVR_ATmega48__)   \
 || defined (__AVR_ATmega328P__)

#define DATAIN 12//MISO
#define DATAOUT 11//MOSI
#define SPICLOCK  13//sck
#define SS 10

#endif

byte clr;

MAX31855::MAX31855(uint8_t csPin, uint32_t updateIntervalMs){
    this->status.csPin = csPin;
    
    this->status.stat = 0;
    this->status.temperature = 0.0;
    
    
    this->updateIntervalMs = updateIntervalMs;
    this->lastUpdate = 0;
    
    for(uint8_t i = 0; i< NUMTCAVGSAMPLES; i++){
    	readings[i] = 0;
    }
    counter = 0;
}

void MAX31855::setup(){
	if(!setupDone){
		setupDone = true;
		pinMode(DATAOUT, OUTPUT);
		pinMode(SPICLOCK,OUTPUT);
		pinMode(DATAIN, INPUT);
		//pinMode(10,OUTPUT);
		digitalWrite(SS,HIGH); // set the pull up on the SS pin (SPI doesn't work otherwise!!)

		clr = 0;
		//The SPI control register (SPCR) has 8 bits, each of which control a particular SPI setting.

		// SPCR
		// | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |0000000000000000000
		// | SPIE | SPE | DORD | MSTR | CPOL | CPHA | SPR1 | SPR0 |

		// SPIE - Enables the SPI interrupt when 1
		// SPE - Enables the SPI when 1
		// DORD - Sends data least Significant Bit First when 1, most Significant Bit first when 0
		// MSTR - Sets the Arduino in master mode when 1, slave mode when 0
		// CPOL - Sets the data clock to be idle when high if set to 1, idle when low if set to 0
		// CPHA - Samples data on the falling edge of the data clock when 1, rising edge when 0'
		// SPR1 and SPR0 - Sets the SPI speed, 00 is fastest (4MHz) 11 is slowest (250KHz)

		SPCR = (1<<SPE)|(1<<MSTR)|(1<<CPHA);// SPI enable bit set, master, data valid on falling edge of clock

		clr=SPSR;
		clr=SPDR;
		//Serial.println("setup done");
	}
	pinMode(status.csPin, OUTPUT);
	digitalWrite(status.csPin, HIGH);
	
	// take one reading and fill the array
	readThermocouple(&status);
	
	for(uint8_t i = 0; i< NUMTCAVGSAMPLES; i++){
    	readings[i] = status.temperature;
    }
}

char MAX31855::spi_transfer(volatile char data)
{
  SPDR = data; // Start the transmission
  while (!(SPSR & (1 << SPIF))) // Wait the end of the transmission
  {
  };
  return SPDR; // return the received byte
}

void MAX31855::readThermocouple(struct max31855Status* input) {
  digitalWrite(input->csPin, LOW);

  uint32_t result = 0x0000;
  byte reply = 0;

  char data = 0; // dummy data to write

  for (int i = 0; i < 4; i++) { // read the 32 data bits from the MAX31855
    reply = spi_transfer(data);
    result = result << 8;
    result |= reply;
  }
  //Serial.print("Read result is: ");
  //Serial.print(result, BIN);

  result >>= 18;

  uint16_t value = 0xFFF & result; // mask off the sign bit and shit to the correct alignment for the temp data

  input->stat = reply & B111;

  input->temperature = value * 0.25;

  digitalWrite(input->csPin, HIGH);

}

void MAX31855::update(){
    if(millis() - lastUpdate >= updateIntervalMs){
        lastUpdate = millis();
        
        readThermocouple(&status);
        
        //remove this after testing! haha
        //status.temperature = random(25, 30);
        //status.stat = 0;
        
        readings[counter] = status.temperature;
        counter++;
        if(counter>=NUMTCAVGSAMPLES){
        	counter = 0;
       	}
       	double total = 0;
       	for(uint8_t i = 0; i< NUMTCAVGSAMPLES; i++){
        	total += readings[i];
        }
        temperature = total / NUMTCAVGSAMPLES;
    }
}
