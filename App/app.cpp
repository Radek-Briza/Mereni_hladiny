/*
 * app.cpp
 *
 *  Created on: 11. 4. 2026
 *      Author: radek
 */
#include "main.h"
#include  "App.hpp"
#include "stdlib.h"
#include "SRF05.h"
#include "Board_hw_specific.h"
#include "radio.h"	

 

SRF05 EchoDriver(SRF05_PORT[TRIGER],SRF05_PIN[TRIGER],SRF05_PORT[ECHO],SRF05_PIN[ECHO]);

void App::init()
{
    // inicializace (např. periferií)
	EchoDriver.setModeMedian(10);
	DataTransmit::GetInstance().Init(&Radio);
	printf("Init device\r");
}
	


void App::loop()
{
	for(;;){
    // tvoje hlavní logika
	BSP_LED_Toggle(LED_GREEN);
	float value = EchoDriver.getCentimeter();
	if(value==0){
		printf("Chyba mereni\r\n");

	}
	else {
		printf("Vzdalenost %d cm\r\n",(uint32_t)abs(value));
	}
	BSP_LED_Toggle(LED_GREEN);
	HAL_Delay(500);
	}

}
