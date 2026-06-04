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
#include <span>
#include "ButtonControl.hpp"

constexpr uint32_t LEVEL_L = (1u << 0);
constexpr uint32_t LEVEL_UNDER_M = (1u << 1);
constexpr uint32_t LEVEL_H = (1u << 2);

constexpr uint32_t LEVEL_L_CM =	10;
constexpr uint32_t LEVEL_M_CM =	30;
constexpr uint32_t LEVEL_H_CM =	100;


SRF05 EchoDriver(SRF05_PORT[TRIGER],SRF05_PIN[TRIGER],SRF05_PORT[ECHO],SRF05_PIN[ECHO]);

void App::init()
{
    // inicializace echo driveru a datového přenosu
	EchoDriver.setModeMedian(10);
	DataTransmit::GetInstance().Init(&Radio);
	
	printf("Init device\r");

	
	if(!param_storage.ReadRecord(Param)){
		led_controller.SetMode(LedController::Leds::Red, LedController::LedMode::Blink);
		printf("Calibration error\n");	
	}
	else{
		printf("Calibration loaded from flash: L_level=%d M_level=%d H_level=%d\n", Param["L_level"], Param["M_level"], Param["H_level"]);
		CalibrationError = false;
	}
}

void App::loop()
{

	for(;;){

		/* Kontrola stisknutých tlačítek */
		if(uint8_t button = button_monitor.ScanButton()) {
			printf("Button %u event: ", button);
			switch (button_monitor.GetLastEvent(button)) {
				case ButtonEvent::PRESSED:
					printf("PRESSED\n");
					break;
				case ButtonEvent::HELD:
					printf("HELD\n");
					break;
				case ButtonEvent::RELEASED:
					printf("RELEASED\n");
					break;
				default:
					printf("NONE\n");
					break;
			}
		}

		/* Kontrola přijatých dat */
		if(DataTransmit::DataAvailable){
			DataTransmit::DataAvailable = false; // Reset flag for next reception
			if(DataTransmit::GetInstance().GetReceivedDataType() == Packet::Level_request){
				printf("Received Level Request\n");
				uint16_t level = static_cast<uint16_t>(EchoDriver.getCentimeter());
				uint16_t status = 0;

				if(level <= LEVEL_L_CM ) status  |=LEVEL_L ;
				if(level < LEVEL_M_CM  && level >  LEVEL_L_CM) status   |=LEVEL_UNDER_M ;
				if(level >= LEVEL_H_CM ) status  |=LEVEL_H ;
				printf("Measured level: %u cm status :%u\n", level, status );
    

				std::vector<uint8_t> payload(sizeof(level)+sizeof(status));
				auto LevelArrea =  std::span{payload}.first(sizeof(level)); 
				auto StatusArrea = std::span{payload}.subspan(sizeof(status), sizeof(level)); 
				std::memcpy(LevelArrea.data(), &level, sizeof(level));
				std::memcpy(StatusArrea.data(),&status, sizeof(status ));
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

		/* led control */
		led_controller.Run();
		
	}

}
