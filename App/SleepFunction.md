# App::Sleep() Function Documentation

## Overview

The `App::Sleep()` method provides ultra-low-power sleep functionality for the STM32WL55 microcontroller. It allows the processor to enter **STOP mode** with RTC-based wakeup, consuming only ~1.5 µA while maintaining the ability to resume execution at the exact point where `Sleep()` was called.

## Function Signature

```cpp
void App::Sleep(uint32_t durationMs);
```

### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `durationMs` | `uint32_t` | Sleep duration in milliseconds |

### Return Value

None. Execution resumes immediately after the RTC alarm fires.

## Power Consumption

- **STOP Mode**: ~1.5 µA (CPU halted, RTC active)
- **Normal Operation**: ~5-10 mA (depending on radio/sensor activity)
- **Energy Savings**: Approximately 1000× reduction in idle current

## How It Works

1. **Configuration**: Sets up RTC alarm for the specified duration
2. **Entry**: Processor enters STOP mode (CPU clock halted)
3. **Waiting**: Only RTC timer runs, consuming minimal power
4. **Wakeup**: RTC alarm fires → processor resumes execution
5. **Cleanup**: RTC alarm is disabled, execution continues normally

```
Timeline:
Before Sleep:  Normal execution
    ↓
Sleep(5000):   Configure RTC for 5000ms
    ↓
[STOP Mode]    CPU halted, RTC counting down...
    ↓
RTC Alarm:     After 5000ms, interrupt fires
    ↓
After Sleep:   Execution resumes at next line
```

## Usage Examples

### Example 1: Simple Sleep for 10 seconds

```cpp
#include "App.hpp"

int main() {
    App app;
    app.init();
    
    // Main operation
    printf("Device operating normally\n");
    
    // Sleep for 10 seconds (ultra-low power)
    app.Sleep(10000);  // 10000 ms = 10 seconds
    
    printf("Device woken up after sleep\n");
    
    return 0;
}
```

### Example 2: Periodic Measurement with Sleep

```cpp
void App::loop() {
    for (;;) {
        // Measure level
        GetLevel = static_cast<uint16_t>(EchoDriver.getCentimeter());
        printf("Current level: %u cm\n", GetLevel);
        
        // Transmit data
        DataTransmit::GetInstance().SendData(Packet::Level_response, payload);
        
        // Sleep for 30 seconds before next measurement
        printf("Entering sleep mode...\n");
        Sleep(30000);  // 30 seconds sleep
        printf("Resumed from sleep\n");
    }
}
```

### Example 3: Battery-Powered Application

```cpp
void App::loop() {
    constexpr uint32_t SLEEP_INTERVAL_MS = 60000;  // 1 minute
    
    for (;;) {
        // Quick measurement and transmission
        MeasureAndTransmit();
        
        // Minimize battery drain with extended sleep
        printf("Battery saving mode: sleeping for 1 minute\n");
        Sleep(SLEEP_INTERVAL_MS);
    }
}
```

### Example 4: Conditional Sleep

```cpp
void App::loop() {
    uint32_t inactivity_counter = 0;
    const uint32_t INACTIVITY_THRESHOLD = 5;  // 5 consecutive empty scans
    
    for (;;) {
        // Check for button press or data
        if (button_monitor.ScanButton() != ButtonAssignment::NO_BUTTON ||
            DataTransmit::DataAvailable) {
            inactivity_counter = 0;
            ProcessEvent();
        } else {
            inactivity_counter++;
        }
        
        // Sleep after inactivity
        if (inactivity_counter >= INACTIVITY_THRESHOLD) {
            printf("No activity detected. Sleeping for 5 minutes.\n");
            Sleep(300000);  // 5 minutes
            inactivity_counter = 0;
        }
    }
}
```

## Implementation Details

### STM32WL55 STOP Mode Characteristics

| Aspect | Detail |
|--------|--------|
| **CPU Clock** | Stopped |
| **Memory** | Retained (SRAM, Flash) |
| **RTC** | Active (continues timing) |
| **Peripherals** | Most inactive |
| **GPIO** | Maintains state |
| **Power Domains** | VDDCORE (1.2V) and VDD (3.3V) maintained |
| **Wake Sources** | RTC alarm, button interrupt, external wakeup |
| **Time to Resume** | <1 µs |

### RTC Timer Configuration

The `Sleep()` function utilizes the existing RTC infrastructure:

- **Source**: STM32WL55 internal RTC (32 kHz oscillator)
- **Resolution**: Millisecond precision via `TIMER_IF_Convert_ms2Tick()`
- **Maximum Duration**: Depends on RTC register width (~24 hours typical)
- **Accuracy**: ±2% (typical RTC accuracy)

### Interrupt Handling

When the RTC alarm fires:

1. `RTC_Alarm_IRQHandler()` is triggered
2. `HAL_RTC_AlarmIRQHandler()` processes the interrupt
3. Program resumes execution in `Sleep()` function
4. CPU re-initializes clock domains
5. Normal execution continues seamlessly

## Wakeup Sources

The processor can wake from STOP mode via:

1. **RTC Alarm** (primary for `Sleep()`)
2. **GPIO Interrupt** (button press)
3. **UART Reception** (if configured)
4. **External Interrupt** (EXTI)

## Energy Consumption Comparison

Assuming a 10-second cycle:

| Mode | Current | Duration | Energy per Cycle |
|------|---------|----------|------------------|
| **Active (5 mA)** | 5 mA | 10 s | 500 mJ |
| **Sleep (1.5 µA)** | 1.5 µA | 10 s | 150 µJ |
| **Savings** | - | - | **99.97%** |

For a battery-powered application:
- Battery: 2000 mAh at 3.3V = 26.4 kJ
- **Active mode only**: 52,800 seconds (~14.7 hours)
- **With Sleep mode**: ~18.5 years (with the above cycle)

## Important Notes

⚠️ **Critical Considerations:**

1. **Flash Storage Access**: Do not access flash during sleep. Perform all writes before calling `Sleep()`.

2. **RTC Configuration**: Ensure RTC is properly initialized by the HAL before using `Sleep()`.

3. **Current Spike**: Brief current spike (~10 mA) occurs during wakeup transition. Ensure power supply can handle this.

4. **Watchdog Timer**: If IWDG is enabled, you must consider the watchdog timeout. Either:
   - Sleep duration < watchdog timeout, or
   - Use RTC-based watchdog refresh

5. **Debug Interface**: When debugging, the processor may not properly exit sleep if debugger is active. Use hardware breakpoints carefully.

6. **Zero Duration**: Passing `durationMs = 0` enters OFF mode (not recommended). Always use positive values.

## Troubleshooting

### Issue: Processor doesn't wake up

**Cause**: RTC not initialized or RTC crystal not oscillating  
**Solution**: Verify `TIMER_IF_Init()` was called in the initialization sequence

### Issue: Wakeup takes longer than expected

**Cause**: Clock domain recovery time  
**Solution**: Normal behavior, expected <1 ms recovery time

### Issue: Program crashes after wakeup

**Cause**: IWDG timeout occurred during sleep  
**Solution**: Increase IWDG timeout or refresh watchdog before sleeping

### Issue: Current consumption during sleep still high

**Cause**: Peripheral still consuming power (UART, SPI, etc.)  
**Solution**: Disable peripherals before calling `Sleep()`

## C++20 Features Used

- **`static_cast<>`**: Type-safe casting
- **`const`/`constexpr`**: Compile-time constants
- **Templates**: Generic timer API handling
- **Printf**: C-style formatted output

## Related API Functions

| Function | Purpose |
|----------|---------|
| `TIMER_IF_StartTimer()` | Configure RTC alarm |
| `TIMER_IF_StopTimer()` | Cancel RTC alarm |
| `TIMER_IF_Convert_ms2Tick()` | Time conversion |
| `UTIL_LPM_EnterLowPower()` | Enter low-power mode |
| `UTIL_LPM_SetStopMode()` | Enable/disable STOP mode |

## References

- STM32WL55 Datasheet
- STM32CubeMX Low Power Mode Documentation
- ARM Cortex-M4 Power Management Documentation
