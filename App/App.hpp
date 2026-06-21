#pragma once
/**
 * @file App.hpp
 * @brief Application entry point and level calibration management.
 *
 * The application class coordinates button scanning, calibration logic,
 * LED feedback, flash parameter storage, and the main execution loop.
 */


#include <map>
#include <string>

#include "main.h"
#include "ButtonControl.hpp"
#include "LedController.hpp"
#include "FlashStorage.hpp"



/**
 * @brief Defines the current calibration state.
 */
enum class CalibrationMode : uint8_t {
    MODE_NONE,              /**< Calibration is disabled or inactive. */
    MODE_CALIBRATION_L,     /**< Low level calibration is active. */
    MODE_CALIBRATION_M,     /**< Medium level calibration is active. */
    MODE_CALIBRATION_H      /**< High level calibration is active. */
};

/**
 * @brief Mapped button actions used by the application.
 */
enum class ButtonAssignment : uint8_t {
    NO_BUTTON,               /**< No button is currently pressed. */
    BUTTON_SETUP_ENABLE = 1, /**< Enter calibration mode. */
    BUTTON_MODE_SELECT = 2,  /**< Cycle through calibration steps. */
    BUTTON_STORE = 3,        /**< Store the current measured value during calibration. */
    BUTTON_ESC = 3,          /**< Exit calibration or clear stored values. */
    BUTTON_VALUE_STORE = 1   /**< Save captured calibration values to non-volatile storage. */
};

/**
 * @brief Main application controller.
 *
 * This class encapsulates the top-level device application, including
 * initialization, processing loop, and calibration workflow.
 */
class App {
public:
    /**
     * @brief Construct a new application instance.
     *
     * The constructor configures button monitoring callbacks and initializes
     * LED control handlers for the Red, Green, and Blue indicators.
     */
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
        auto BlueLed = [](bool on) {
            on ? BSP_LED_On(LED_BLUE) : BSP_LED_Off(LED_BLUE);
        };
        LedController::Init(RedLed, GreenLed, BlueLed);
    }

    /**
     * @brief Initialize the device and load calibration parameters.
     *
     * This method initializes the ultrasonic distance sensor and radio transport,
     * then reads stored calibration parameters from flash memory.
     */
    void init();

    /**
     * @brief Enter the main application loop.
     *
     * The loop monitors button events, executes calibration logic, handles
     * incoming radio requests, and updates LED states indefinitely.
     */
    void loop();

    /**
     * @brief Sleep the processor with minimal power consumption.
     *
     * Configures an RTC alarm for the specified duration and enters STOP mode.
     * The processor resumes execution at the instruction following this call
     * when the RTC alarm fires or another wakeup source is triggered.
     *
     * @param durationMs Sleep duration in milliseconds.
     *                   If set to 0, enters OFF mode (deepest sleep).
     *
     * @note Power consumption in STOP mode: ~1.5 µA
     * @note RTC remains active during sleep for wakeup timing.
     * @note Execution resumes after the RTC interrupt handler completes.
     */
    void Sleep(uint32_t durationMs);

private:
    /**
     * @brief Calibration polling interval in milliseconds.
     */
    static constexpr uint32_t CALIBRATION_RUN_PERIODE = 100U; // ms

    /**
     * @brief Last measured level in centimeters.
     */
    uint16_t Distance;

    /**
     * @brief Indicates whether calibration data is invalid or missing.
     */
    bool CalibrationError = true;

    /**
     * @brief Monitors button state changes.
     */
    ButtonMonitor button_monitor;

    /**
     * @brief Controls the RGB LEDs used for status feedback.
     */
    LedController led_controller;

    /**
     * @brief Persistent storage for calibration parameters.
     * @param Address in flash memory where calibration parameters are stored.
     */
    FlashParameterStorage param_storage{0x0803F800};

    /**
     * @brief Current calibration thresholds.
     *
     * Initial default values are used until flash data is loaded.
     */
    std::map<std::string, int32_t> Param = {
        {"L_level", 100},
        {"M_level", 50},
        {"H_level", 10}
    };

    /**
     * @brief Run calibration state machine for the given button event.
     *
     * @param button Button action that triggered calibration logic.
     * @return true if calibration measurement mode remains active.
     * @return false if calibration mode is inactive or was ended.
     */
    bool CalibrationRoutime(ButtonAssignment button);

};
