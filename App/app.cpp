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

		
		if(DataTransmit::DataAvailable){
			DataTransmit::DataAvailable = false; // Reset flag for next reception
			if(DataTransmit::GetInstance().GetReceivedDataType() == Packet::Level_request){
				printf("Received Level Request\n");
				uint16_t level = static_cast<uint16_t>(EchoDriver.getCentimeter());
				printf("Measured level: %u cm\n", level);
				std::vector<uint8_t> payload(sizeof(level));
				std::memcpy(payload.data(), &level, sizeof(level));
				DataTransmit::GetInstance().SendData(Packet::Level_response, payload);
			}
			else if(DataTransmit::GetInstance().GetReceivedDataType() == Packet::Battery_request){
				printf("Received Battery Request\n");
				// Simulace úrovně baterie (např. 75%)
				float battery_level = 75.0f;
				std::vector<uint8_t> battery_payload(sizeof(battery_level));
				std::memcpy(battery_payload.data(), &battery_level, sizeof(battery_level));
				DataTransmit::GetInstance().SendData(Packet::Battery_response, battery_payload);
			}
			else{
				printf("Received unknown packet type\n");
			}
		}
		/*
    // tvoje hlavní logika
	BSP_LED_Toggle(LED_GREEN);
	float value = EchoDriver.getCentimeter();
	if(value==0){
		printf("Chyba mereni\r\n");

	}
	else {
		printf("Vzdalenost %d cm\r\n", static_cast<int>(value));
	}
	BSP_LED_Toggle(LED_GREEN);
	HAL_Delay(500);
	*/
	}

}
