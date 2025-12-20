#include "potentiometer.h"

 #define RAW_MIN 0
#define RAW_MAX 4095        // Assuming 12-bit ADC
#define PERCENT_MIN 0
#define PERCENT_MAX 100
#define VOLTAGE_MIN 0       // in mV
#define VOLTAGE_MAX 3300
#define MAPPED_MIN 0
#define MAPPED_MAX 9

void Potentiometer_test(){
 
    POT_Init();
    uint16_t rawValue;
    uint8_t percentage;
    uint32_t voltage;
    uint32_t mapped;
    
    uint32_t mappedMin = 0;
    uint32_t mappedMax = 9;

    printf("Starting Potentiometer Unit Test...\n");


    
    rawValue = POT_ReadRaw();
    if(rawValue > 0) printf("RAW: %u -> PASS\n", rawValue);
    
    else printf("RAW: %u -> FAIL\n", rawValue);


    percentage = POT_ReadPercentage();
    
    
    if(percentage <= 100) printf("PERCENTAGE: %u%% -> PASS\n", percentage);
    else printf("PERCENTAGE: %u%% -> FAIL\n", percentage);


    voltage = POT_ReadMillivolts();
    if(voltage <= 3300) printf("VOLTAGE: %lu mV -> PASS\n", voltage);
    else printf("VOLTAGE: %lu mV -> FAIL\n", voltage);

    

    mapped = mappedMin + ((uint32_t)rawValue * (mappedMax - mappedMin)) / 1023;
    if(mapped >= mappedMin && mapped <= mappedMax)
        printf("MAPPED: %lu -> PASS\n", mapped);
    
    else {
      printf("MAPPED: %lu -> FAIL\n", mapped);}

    printf("Potentiometer Unit Test Complete.\n");
}
