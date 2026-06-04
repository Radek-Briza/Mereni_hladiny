#pragma once

#include "main.h" // IWYU pragma: keep
#include <array>
#include <cstdint>
#include <functional>

/**
 * @enum ButtonEvent
 * @brief Events that can occur on a button
 */
enum class ButtonEvent : uint8_t {
  NONE = 0,    /**< No event detected */
  PRESSED = 1, /**< Button pressed for > 100ms and released */
  HELD = 2,    /**< Button held for > 1500ms */
  RELEASED = 3 /**< Held button was released */
};

/**
 * @struct ButtonState
 * @brief Internal state structure for a single button
 */
struct ButtonState {
  ButtonEvent last_event = ButtonEvent::NONE; /**< Last detected event */
  uint32_t press_start_time = 0; /**< Tick when button was pressed */
  bool is_pressed = false;       /**< Current physical state */
  bool hold_event_sent =
      false; /**< Flag: hold event already sent for this press */
  bool prev_pressed = false; /**< Previous physical state */
};

/**
 * @class ButtonMonitor
 * @brief Monitors 3 buttons with state detection (press, hold, release)
 *
 * - Press event: detected when button is held > 100ms and then released
 * - Hold event: detected when button is held > 1500ms (immediately when time
 * expires)
 * - Release event: detected when a held button is released
 * - Scan interval: 10ms
 * - Time source: HAL_GetTick()
 */
class ButtonMonitor {
public:
  /** Button read function: returns true if button is pressed */
  using ButtonReadFunc = std::function<bool()>;

  /**
   * @brief Constructor
   * @param read_btn1 Lambda to read button 1 state (true = pressed)
   * @param read_btn2 Lambda to read button 2 state (true = pressed)
   * @param read_btn3 Lambda to read button 3 state (true = pressed)
   */
  ButtonMonitor(ButtonReadFunc read_btn1, ButtonReadFunc read_btn2,
                ButtonReadFunc read_btn3);

  /**
   * @brief Scan buttons and detect state changes
   * @return 0 if no state change detected, or button number (1-3) if change
   * detected
   * @note Call every 10ms (non-blocking)
   */
  uint8_t ScanButton();

  /**
   * @brief Get current physical state of a button
   * @param button Button number (1-3)
   * @return true if button is currently pressed, false otherwise
   */
  bool ReadButtonStatus(uint8_t button);

  /**
   * @brief Get the last detected event for a button
   * @param button Button number (1-3)
   * @return Last ButtonEvent detected on this button
   */
  ButtonEvent GetLastEvent(uint8_t button) const;

private:
  static constexpr uint8_t NUM_BUTTONS = 3;
  static constexpr uint32_t PRESS_TIMEOUT_MS =
      100; /**< Time to detect press event */
  static constexpr uint32_t HOLD_TIMEOUT_MS =
      1500; /**< Time to detect hold event */

  /** Array of button read functions */
  std::array<ButtonReadFunc, NUM_BUTTONS> read_funcs;

  /** Array of button states */
  std::array<ButtonState, NUM_BUTTONS> button_states;

  /**
   * @brief Read raw state of a button
   * @param button Button number (0-2 internally, 1-3 externally)
   * @return true if button is pressed
   */
  bool ReadButtonRaw(uint8_t button);

  /**
   * @brief Update internal state for a single button
   * @param button Button number (0-2)
   * @return true if state change was detected
   */
  bool UpdateButtonState(uint8_t button);
};
