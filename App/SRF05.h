#pragma once
//
//    FILE: SRF05.h
//  AUTHOR: Rob Tillaart
// VERSION: 0.3.2
//    DATE: 2021-05-17
// PURPOSE: Arduino library for SRF05 distance sensor
//     URL: https://github.com/RobTillaart/SRF05


#include "main.h"


#define SRF05_LIB_VERSION                 (F("0.3.2"))


const uint8_t SRF05_MODE_SINGLE      = 0;
const uint8_t SRF05_MODE_AVERAGE     = 1;
const uint8_t SRF05_MODE_MEDIAN      = 2;
const uint8_t SRF05_MODE_RUN_AVERAGE = 3;


class SRF05
{
public:
  explicit SRF05(GPIO_TypeDef* trigger_port,const uint16_t trigger_pin ,GPIO_TypeDef* echo_port, const uint16_t echo_pin);


  //  configuration
  void     setSpeedOfSound(float speedOfSound = 340);  //  meter/sec
  float    getSpeedOfSound();

  //  adjust timing
  bool     setCorrectionFactor(float factor = 1);
  float    getCorrectionFactor();


  //  operational mode
  void     setModeSingle();  //  default
  void     setModeAverage(uint8_t count);
  void     setModeMedian(uint8_t count);
  void     setModeRunningAverage(float alpha);
  uint8_t  getOperationalMode();
  //  interval of average and median mode
  void     setSampleInterval(uint16_t microSeconds = 1000);
  uint16_t getSampleInterval();


  //  get distance
  uint32_t getTime();         //  uSec
  uint32_t getMillimeter();   //  mm
  float    getCentimeter();   //  cm
  float    getMeter();        //  m
  float    getInch();         //  inch = 2.54 cm
  float    getFeet();         //  feet = 12 inch
  float    getYards();        //  yard = 3 feet = 36 inch


  //  Experimental - calibration
  //  The distance is averaged over 64 measurements.
  //  blocks for 70-80 ms.
  //  distance in meters (1 meter = 3.333 feet)
  //  returns speed in m/s.
  float    determineSpeedOfSound(float distance, uint8_t count = 64);


  //  Experimental - adjust trigger length
  //  - gain is a few microseconds at best.
  //  - 10 microseconds is advised minimum
  //  - to be investigated.
  void     setTriggerLength(uint8_t length = 10);
  uint8_t  getTriggerLength();


  //  TIMING
  uint32_t lastTime();

  //  helper function.
  //  temperature and humidity to be determined by a sensor e.g. DHT22 or SHT85
  //  returned value must be set explicitly by setSpeedOfSound().
  float calculateSpeedOfSound(float temperature, float humidity);


private:
  GPIO_TypeDef*  _trigger_port;
  GPIO_TypeDef*  _echo_port;
  uint16_t _trigger_pin;
  uint16_t _echo_pin;

  uint8_t  _mode  = SRF05_MODE_SINGLE;
  uint8_t  _count = 1;
  float    _alpha = 1.0;
  float    _value = 0;
  float    _correctionFactor = 1;
  uint16_t  _triggerLength    = 10;       //  microSeconds
  float    _speedOfSound     = 340;      //  15°C  0%RH  Sea level
  uint32_t _lastTime = 0;
  uint16_t _sampleInterval = 1000;       //  microSeconds

  uint32_t _read();
  void     _insertSort(uint32_t * array, uint8_t size);
  unsigned long pulseIn(unsigned long timeout);
};


//  wrapper classes with other names?


//  -- END OF FILE --

