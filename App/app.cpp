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
#include <cstdint>
#include <span>
#include <stm32wlxx_hal_uart.h>
#include "ButtonControl.hpp"
#include  "DataTransmit.hpp"
#include "timer_if.h"
#include "stm32_lpm_if.h"
#include "stm32_timer.h"
#include "timer_if.h"

constexpr uint32_t LEVEL_L = (1u << 0);
constexpr uint32_t LEVEL_UNDER_M = (1u << 1);
constexpr uint32_t LEVEL_H = (1u << 2);
constexpr uint32_t LEVEL_L_CM =	10;
constexpr uint32_t LEVEL_M_CM =	30;
constexpr uint32_t LEVEL_H_CM =	100;

extern IWDG_HandleTypeDef hiwdg;

SRF05 EchoDriver(SRF05_PORT[TRIGER],SRF05_PIN[TRIGER],SRF05_PORT[ECHO],SRF05_PIN[ECHO]);


/* level calibration  */
bool App::CalibrationRoutime (ButtonAssignment button){
	static CalibrationMode Mode = {CalibrationMode::MODE_NONE};	
	static uint32_t L_value =0;
	static uint32_t M_value =0;
	static uint32_t H_value =0;

	std::map<std::string,int32_t> CalibParams = {
        {"L_level", 0},
        {"M_level", 0},
        {"H_level", 0}
    };

	param_storage.ReadRecord(CalibParams);

	auto Event = button_monitor.GetLastEvent(static_cast<uint8_t>(button));

	/* clear calibration */
	if(Mode == CalibrationMode::MODE_NONE 
		&& button == ButtonAssignment::BUTTON_ESC
		&& Event == ButtonEvent::HELD){
		Mode = CalibrationMode::MODE_NONE; 
		led_controller.SetMode(LedController::Leds::Green, LedController::LedMode::OneShot);
		led_controller.SetMode(LedController::Leds::Blue,LedController::LedMode::OneShot);
		if(param_storage.Format()==true){
			printf("Calibration clear \n");
			CalibrationError = true;
			led_controller.SetMode(LedController::Leds::Red,LedController::LedMode::Blink);
		}
		return false;
	}

	/* start calibratinon */
	if(Mode == CalibrationMode::MODE_NONE){
		if(button == ButtonAssignment::BUTTON_SETUP_ENABLE) {
			if(Event == ButtonEvent::HELD){
				Mode = CalibrationMode::MODE_CALIBRATION_L;
				led_controller.SetMode(LedController::Leds::Green, LedController::LedMode::On);
				led_controller.SetMode(LedController::Leds::Red,LedController::LedMode::Off);
				CalibrationError = true;
				printf("Calibration mode started\n");
				return true;
			}
			return false;
	  	}

	} else { /* mode  run */			
				/*escape    */
				if(Mode != CalibrationMode::MODE_NONE 
					&& button == ButtonAssignment::BUTTON_ESC
					&& Event == ButtonEvent::HELD){
					Mode = CalibrationMode::MODE_NONE; 
					led_controller.SetMode(LedController::Leds::Green, LedController::LedMode::Off);
					led_controller.SetMode(LedController::Leds::Blue,LedController::LedMode::Off);
					led_controller.SetMode(LedController::Leds::Red,LedController::LedMode::Off);
					printf("Calibration mode exited\n");
					return false;
				}

			/* store param */
			if(Mode != CalibrationMode::MODE_NONE 
				&& 	button == ButtonAssignment::BUTTON_STORE
				&&  Event == ButtonEvent::PRESSED)
				{
					if(Mode == CalibrationMode::MODE_CALIBRATION_L)  L_value = GetDistance;
					if(Mode == CalibrationMode::MODE_CALIBRATION_M)  M_value = GetDistance;
					if(Mode == CalibrationMode::MODE_CALIBRATION_H)  H_value = GetDistance;
					led_controller.SetMode(LedController::Leds::Red,LedController::LedMode::OneShot);
					return true;
				}

			/* all param store to flash */
			if(Mode != CalibrationMode::MODE_NONE 
				&& button == ButtonAssignment::BUTTON_VALUE_STORE
				&& Event == ButtonEvent::HELD){
				if(L_value != 0 )	CalibParams["L_level"] = L_value;
				if(M_value != 0 )	CalibParams["M_level"] = M_value;
				if(H_value != 0 )	CalibParams["H_level"] = H_value;

				/*update work params*/
				Param["L_level"] = L_value;
				Param["M_level"] = M_value;
				Param["H_level"] = H_value;

				Mode = CalibrationMode::MODE_NONE; 

				led_controller.SetMode(LedController::Leds::Green, LedController::LedMode::Off);
				led_controller.SetMode(LedController::Leds::Blue,LedController::LedMode::Off);

				if(param_storage.WriteRecord(CalibParams)) {
				printf("Calibration values saved to flash.\n");
				} else {
						printf("Failed to save calibration values to flash.\n");
						}
				return false;
			}	

			/* select mode */
			if(button == ButtonAssignment::BUTTON_MODE_SELECT && Event == ButtonEvent::PRESSED) {
				/* L mode run - switch to M calibb  */
				if(Mode == CalibrationMode::MODE_CALIBRATION_L){
					Mode = CalibrationMode::MODE_CALIBRATION_M;
					led_controller.SetMode(LedController::Leds::Green, LedController::LedMode::Blink);
					printf("Switched to M level calibration\n");
					return true;
				}
				/* M mode run switch to H mode  */
				if(Mode == CalibrationMode::MODE_CALIBRATION_M){
					Mode = CalibrationMode::MODE_CALIBRATION_H;
					led_controller.SetMode(LedController::Leds::Green, LedController::LedMode::Off);
					led_controller.SetMode(LedController::Leds::Blue,LedController::LedMode::On);
					printf("Switched to H level calibration\n");
					return true;
				}
				/* H mode to switch L mode   */
				if(Mode == CalibrationMode::MODE_CALIBRATION_H){
					Mode = CalibrationMode::MODE_CALIBRATION_L;
					led_controller.SetMode(LedController::Leds::Green, LedController::LedMode::On);
					led_controller.SetMode(LedController::Leds::Blue,LedController::LedMode::Off);
					printf("Switched to L level calibration\n");
					return true;
				}
			}
		return false;		
	} 
	return false;
}

/*init app */
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
		printf("Calibration loaded from flash: L_level=%d M_level=%d H_level=%d\n", static_cast<int>(Param["L_level"]),
		 static_cast<int>(Param["M_level"]),
		  static_cast<int>(Param["H_level"]));
		CalibrationError = false;
	}
}

/* main app loop */
void App::loop()
{
	bool CalibrationRun=false;
	uint32_t CalibrationTimeStamp=0U;

	for(;;){

		/* Kontrola stisknutých tlačítek */
		auto button = static_cast<ButtonAssignment>(button_monitor.ScanButton());
		if(button != ButtonAssignment::NO_BUTTON) {
			if(button_monitor.GetLastEvent(static_cast<uint8_t>(button))!= ButtonEvent::RELEASED){
			CalibrationRun = CalibrationRoutime(button);
			}
		}
		

		/*calibration messure run */
		if(CalibrationRun== true){
			if(HAL_GetTick() >  CalibrationTimeStamp+CALIBRATION_RUN_PERIODE){ 
				 CalibrationTimeStamp = HAL_GetTick();
				 GetDistance = static_cast<uint16_t>(EchoDriver.getCentimeter());
				 printf("Calibration mode, measured level: %u cm\n", GetDistance);
			}
		}

		/* Kontrola přijatých dat */
		if(DataTransmit::DataAvailable){
			DataTransmit::DataAvailable = false; // Reset flag for next reception
			if(DataTransmit::GetInstance().GetReceivedDataType() == Packet::Level_request && CalibrationRun==false){
				printf("Received Level Request\n");
				led_controller.SetMode(LedController::Leds::Green, LedController::LedMode::OneShot);
				
				GetDistance = static_cast<uint16_t>(EchoDriver.getCentimeter());
				printf("Measured distance: %u cm\n", GetDistance);
				uint16_t status = 0;

				if(Param["L_level"]==0){
					 GetLevel = 0;
					}else {
						GetLevel = Param["L_level"] - GetDistance;
					}
						

				if(GetDistance >= Param["L_level"] ) status  |=LEVEL_L ;
				if(GetDistance > Param["M_level"]  && GetDistance <  Param["L_level"]) status   |=LEVEL_UNDER_M ;
				if(GetDistance <= Param["H_level"] ) status  |=LEVEL_H ;
				printf("Measured level: %u cm status :%u\n", GetLevel, status );
    

				std::vector<uint8_t> payload(sizeof(GetLevel)+sizeof(status));
				auto LevelArrea =  std::span{payload}.first(sizeof(GetLevel)); 
				auto StatusArrea = std::span{payload}.subspan(sizeof(GetLevel), sizeof(status)); 
				std::memcpy(LevelArrea.data(), &GetLevel, sizeof(GetLevel));
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
		#if WDT_ENABLE
		HAL_IWDG_Refresh(&hiwdg);
		#endif
		#if SLEEP_ENABLE
		if(CalibrationRun==false ){
			Sleep(0);
		}
		#endif
	}

}

/**
 * @brief Sleep the processor with minimal power consumption.
 *
 * This function configures an RTC alarm and enters STOP mode for ultra-low power operation.
 * The processor resumes execution immediately after the RTC alarm fires or when another
 * interrupt source (e.g., button press) triggers wakeup.
 * @param durationMs Sleep duration in milliseconds. If set to 0, the device wait indefinitely for an interrupt (OFF mode).
 * @note STOP Mode consumption: ~1.5 µA (core inactive, RTC and peripherals remain active)
 * @note Power domains maintained: VDDCORE (1.2V) and VDD (3.3V)
 * @note All clocks halted except RTC
 * @note Flash and SRAM retain content
 */
void App::Sleep(uint32_t durationMs)
{

    if (durationMs == 0) {
        // Force STOP mode by allowing Stop mode and disallowing OFF mode
		printf("Sleep: Entering STOP mode indefinitely (waiting for interrupt)...\n");
        UTIL_LPM_SetStopMode(1, UTIL_LPM_ENABLE);
        UTIL_LPM_SetOffMode(1, UTIL_LPM_DISABLE);
       // Enter STOP mode - CPU halts here
        UTIL_LPM_EnterLowPower();
		printf("Exiting STOP mode.\n");
	    // Restore OFF mode allowance for later low-power transitions
        UTIL_LPM_SetOffMode(1, UTIL_LPM_ENABLE);
    } else {
        // Convert milliseconds to RTC ticks and configure alarm
        uint32_t ticks = TIMER_IF_Convert_ms2Tick(durationMs);
       printf("Sleep: Entering STOP mode for %lu ms (%lu ticks).\n", 
             static_cast<unsigned long>(durationMs), 
              static_cast<unsigned long>(ticks));

        // Start RTC alarm for wakeup
		UTIL_TIMER_Status_t timer_status = TIMER_IF_StartTimer(ticks);
        if (timer_status != UTIL_TIMER_OK) {
            printf("Sleep: Failed to start timer. Status: %d\n", timer_status);
            return;
        }
	
        // Force STOP mode by allowing Stop mode and disallowing OFF mode
        UTIL_LPM_SetStopMode(1, UTIL_LPM_ENABLE);
        UTIL_LPM_SetOffMode(1, UTIL_LPM_DISABLE);

        // Enter STOP mode - CPU halts here
        UTIL_LPM_EnterLowPower();

        // Execution resumes here after RTC wakeup
        printf("Exiting STOP mode.\n");

        // Restore OFF mode allowance for later low-power transitions
        UTIL_LPM_SetOffMode(1, UTIL_LPM_ENABLE);
    }
}