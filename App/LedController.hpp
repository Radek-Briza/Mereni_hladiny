

#pragma once

#include <array>
#include <atomic>
#include <functional>
#include <cstdint>

class LedController{
public:

    enum class Leds : uint8_t{
        Red = 0,
        Green,
        Blue,
        Count
    };

    enum class LedMode : uint8_t{
        Off,
        On,
        Blink,
        OneShot
    };

    struct LedState{
    std::atomic<LedMode> mode;
    };

    using LedCallback = std::function<void(bool)>;

    static void Init(
        LedCallback red,
        LedCallback green,
        LedCallback blue);

    static void SetMode(
        Leds led,
        LedMode mode);

    static LedMode GetMode(
        Leds led);

    static void Run();

private:
        static size_t ToIndex(Leds led){
            const auto index = static_cast<size_t>(led);
            return index;
        }

    static constexpr uint32_t PERIOD = 100; // 100 ms

    static constexpr size_t LED_COUNT = static_cast<size_t>(Leds::Count);

    static std::array<LedState, LED_COUNT> leds_;

    static std::array<LedCallback,LED_COUNT> callbacks_;
};


void LedDriverInit();