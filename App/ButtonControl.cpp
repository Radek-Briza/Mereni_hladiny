#include "ButtonControl.hpp"

ButtonMonitor::ButtonMonitor(ButtonReadFunc read_btn1, ButtonReadFunc read_btn2,
                             ButtonReadFunc read_btn3)
    : read_funcs{read_btn1, read_btn2, read_btn3} {
  // Initialize all button states
  for (auto &state : button_states) {
    state.last_event = ButtonEvent::NONE;
    state.press_start_time = 0;
    state.is_pressed = false;
    state.hold_event_sent = false;
    state.prev_pressed = false;
  }
}

uint8_t ButtonMonitor::ScanButton() {
  // Scan all 3 buttons
  for (uint8_t i = 0; i < NUM_BUTTONS; ++i) {
    if (UpdateButtonState(i)) {
      // Return button number (1-3) if state change detected
      return i + 1;
    }
  }
  // No state change detected
  return 0;
}

bool ButtonMonitor::ReadButtonStatus(uint8_t button) {
  // Validate button number (1-3)
  if (button < 1 || button > NUM_BUTTONS) {
    return false;
  }

  // Button is 1-indexed externally, 0-indexed internally
  return ReadButtonRaw(button - 1);
}

ButtonEvent ButtonMonitor::GetLastEvent(uint8_t button) const {
  // Validate button number (1-3)
  if (button < 1 || button > NUM_BUTTONS) {
    return ButtonEvent::NONE;
  }

  return button_states[button - 1].last_event;
}

bool ButtonMonitor::ReadButtonRaw(uint8_t button) {
  // Validate button index (0-2)
  if (button >= NUM_BUTTONS) {
    return false;
  }

  return read_funcs[button]();
}

bool ButtonMonitor::UpdateButtonState(uint8_t button) {
  // Get current physical state
  bool current_pressed = ReadButtonRaw(button);
  ButtonState &state = button_states[button];

  uint32_t current_time = HAL_GetTick();
  bool state_changed = false;

  // Detect state change (pressed -> released or vice versa)
  if (current_pressed != state.prev_pressed) {
    state.prev_pressed = current_pressed;
    state_changed = true;

    if (current_pressed) {
      // Button just pressed
      state.press_start_time = current_time;
      state.hold_event_sent = false;
      state.last_event = ButtonEvent::NONE;
    } else {
      // Button just released
      uint32_t press_duration = current_time - state.press_start_time;

      if (state.hold_event_sent) {
        // Hold event was already sent during the press
        // Now send release event
        state.last_event = ButtonEvent::RELEASED;
      } else if (press_duration >= PRESS_TIMEOUT_MS) {
        // Button was held for at least 100ms
        state.last_event = ButtonEvent::PRESSED;
      } else {
        // Button was pressed too briefly
        state.last_event = ButtonEvent::NONE;
        state_changed = false; // Don't report as change if press was too short
      }
    }
  } else if (current_pressed && !state.hold_event_sent) {
    // Button is still pressed and hold event not yet sent
    uint32_t press_duration = current_time - state.press_start_time;

    if (press_duration >= HOLD_TIMEOUT_MS) {
      // Hold timeout reached
      state.hold_event_sent = true;
      state.last_event = ButtonEvent::HELD;
      state_changed = true;
    }
  }

  state.is_pressed = current_pressed;
  return state_changed;
}
