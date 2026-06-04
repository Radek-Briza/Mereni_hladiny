
#
#include "LedController.hpp"
#include "main.h"

std::array<LedController::LedState, LedController::LED_COUNT> LedController::leds_;
std::array<LedController::LedCallback,3> LedController::callbacks_;

void LedController::Init(
    LedCallback led0,
    LedCallback led1,
    LedCallback led2){
    
    callbacks_[0] = led0;
    callbacks_[1] = led1;
    callbacks_[2] = led2;

    leds_[0].mode.store(
        LedMode::Off,
        std::memory_order_relaxed);
    
    leds_[1].mode.store(
        LedMode::Off,
        std::memory_order_relaxed);

    leds_[2].mode.store(
        LedMode::Off,
        std::memory_order_relaxed);
      
}

void LedController::SetMode(Leds led,LedMode mode){
    auto index = ToIndex(led);

   leds_[index].mode.store(
          mode,
            std::memory_order_relaxed);
    }
 

LedController::LedMode LedController::GetMode(Leds led){
    const auto index =
        static_cast<size_t>(led);

    return leds_[index].mode.load(
        std::memory_order_relaxed);
}


void LedController::Run(){ 
    static uint32_t lastWake = HAL_GetTick();

    static bool blinkState = false;
    static bool oneShotState = false;

    if(HAL_GetTick() - lastWake >= PERIOD){
        lastWake = HAL_GetTick();
        blinkState = !blinkState;

        for (uint8_t i = 0; i < 3; i++){
            const LedMode mode =
                leds_[i].mode.load(
                    std::memory_order_relaxed);

            switch (mode)
            {
                case LedMode::Off:
                    callbacks_[i](false);
                 //   printf("LED %d OFF\r\n",i);
                    break;

                case LedMode::On:
                    callbacks_[i](true);
                    break;

                case LedMode::Blink:
                    callbacks_[i](blinkState);
                    //printf("LED %d BLINK %d\r\n",i,blinkState);
                    break;

                case LedMode::OneShot: 
                    if(oneShotState==false){
                        callbacks_[i](true);
                        oneShotState = true;
                    } else {
                        callbacks_[i](false);
                        oneShotState = false;
                        leds_[i].mode.store(
                            LedMode::Off,
                            std::memory_order_relaxed);    
                    }                     
            }
        }
    }     
}


void LedDriverInit(){

	auto RedLed = [](bool on) { 
		on ? BSP_LED_On(LED_RED) : BSP_LED_Off(LED_RED);
	};
	auto GreenLed = [](bool on) { 
		 on ? BSP_LED_On(LED_GREEN) : BSP_LED_Off(LED_GREEN);
	};
	auto  BlueLed = [](bool on) { 
		 on ? BSP_LED_On(LED_BLUE) : BSP_LED_Off(LED_BLUE);
	};
    LedController::Init(RedLed,GreenLed,BlueLed);
}