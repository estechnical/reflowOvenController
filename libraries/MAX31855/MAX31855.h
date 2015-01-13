#ifndef MAX31855KASA_H_
#define MAX31855KASA_H_

#include <Arduino.h>

#define NUMTCAVGSAMPLES 10

static bool setupDone;

typedef struct max31855Status {
  double temperature;
  uint8_t stat;
  uint8_t csPin;
};

class MAX31855 {
    private:
        
        uint32_t updateIntervalMs, lastUpdate;
        max31855Status status;
        double temperature;
        
        uint8_t counter;
        double readings[NUMTCAVGSAMPLES];
        
        static char spi_transfer(volatile char);
        static void readThermocouple(struct max31855Status*);
        
        
    public:
        MAX31855(uint8_t, uint32_t);
        void setup();
        double getTemperature(){
            return temperature;
        }
        uint8_t getStatus(){
            return status.stat;
        }
        void update();
        
};

#endif
