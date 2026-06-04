/*
 * App.hpp
 *
 *  Created on: 19. 4. 2026
 *      Author: radek
 */

#ifndef APP_HPP_
#define APP_HPP_
#include "main.h"
#include  "DataTransmit.hpp"
#include "ButtonControl.hpp"
#include "LedController.hpp"
#include "FlashStorage.hpp"


class App {
public:

    App() : button_monitor(
        []()->bool { return (static_cast<bool>(HAL_GPIO_ReadPin(BT_1_GPIO_Port, BT_1_Pin) == GPIO_PIN_RESET)); },
        []()->bool { return (static_cast<bool>(HAL_GPIO_ReadPin(BT_2_GPIO_Port, BT_2_Pin) == GPIO_PIN_RESET)); },
        []()->bool { return (static_cast<bool>(HAL_GPIO_ReadPin(BT_3_GPIO_Port, BT_3_Pin) == GPIO_PIN_RESET)); }
    ) { 
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

    void init();
    void loop();

    private:
    bool CalibrationError = true;
    ButtonMonitor button_monitor;
    LedController led_controller;
    FlashParameterStorage param_storage{0x0803F800}; 
    std::map<std::string,int32_t> Param = {
        {"L_level", 100},
        {"M_level", 50},
        {"H_level", 10}
    };
};



#endif /* APP_HPP_ */
