#pragma once

#include "main.h" // IWYU pragma: keep

#include <array>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <span>

/**
 * @class AdcReader
 * @brief Asynchronous multi-channel ADC reader based on HAL ADC interrupt.
 *
 * Class sequentially starts conversion for configured channels using
 * HAL ADC interrupt mode. Conversion result is stored as raw ADC data and as
 * final value in mV after resistor divider ratio compensation.
 */
class AdcReader {
public:
	/**
	 * @brief Result codes returned by getter functions.
	 */
	enum class Result : uint8_t {
		RESULT_OK = 0,         /**< Requested value returned successfully */
		CHANNEL_NOT_AVAILABLE, /**< Required channel is not configured */
		DATA_NOT_AVAILABLE     /**< Conversion for required channel not ready yet */
	};

	/**
	 * @brief Per-channel static configuration.
	 */
	struct ChannelConfig {
		uint32_t channel = 0U;         /**< HAL ADC channel number (ADC_CHANNEL_x) */
		float divider_ratio = 1.0F; /**< Divider ratio to convert ADC input mV to real
																	 channel voltage */
	};

	static constexpr std::size_t MAX_CHANNELS = 16U;

	/**
	 * @brief Constructor.
	 * @param adc_handle Handle to HAL ADC driver (mandatory).
	 * @param channels List of measured channels and divider ratios (mandatory).
	 * @param reference_voltage_mv ADC reference voltage in mV (default 3000 mV).
	 */
	AdcReader(ADC_HandleTypeDef *adc_handle, std::span<const ChannelConfig> channels,
						uint16_t reference_voltage_mv = 3000U);

	/**
	 * @brief Constructor overload for initializer list.
	 * @param adc_handle Handle to HAL ADC driver (mandatory).
	 * @param channels List of measured channels and divider ratios (mandatory).
	 * @param reference_voltage_mv ADC reference voltage in mV (default 3000 mV).
	 */
	AdcReader(ADC_HandleTypeDef *adc_handle,
						std::initializer_list<ChannelConfig> channels,
						uint16_t reference_voltage_mv = 3000U);

	/**
	 * @brief Start ADC conversion sequence in background.
	 * @return true if conversion started, false if already running or on error.
	 */
	bool Start();

	/**
	 * @brief Get final converted value for one channel.
	 * @param channel Requested HAL ADC channel number (ADC_CHANNEL_x).
	 * @param value_out Reference where converted value is stored.
	 * @return RESULT_OK, CHANNEL_NOT_AVAILABLE or DATA_NOT_AVAILABLE.
	 * @note On RESULT_OK the channel data_available flag is cleared.
	 */
	Result GetFinalValue(uint32_t channel, uint16_t &value_out);

	/**
	 * @brief Get raw ADC value for one channel.
	 * @param channel Requested HAL ADC channel number (ADC_CHANNEL_x).
	 * @param value_out Reference where raw ADC value is stored.
	 * @return RESULT_OK, CHANNEL_NOT_AVAILABLE or DATA_NOT_AVAILABLE.
	 * @note This function does not clear data_available flag.
	 */
	Result GetRawValue(uint32_t channel, uint16_t &value_out) const;

	/**
	 * @brief ADC conversion complete callback handler.
	 * @param adc_handle ADC handle passed from HAL callback routine.
	 * @note Call from HAL_ADC_ConvCpltCallback.
	 */
	void AdcCallback(ADC_HandleTypeDef *adc_handle);

	/**
	 * @brief Attach instance used by global HAL ADC callback bridge.
	 * @param instance Pointer to AdcReader instance (or nullptr to detach).
	 */
	static void AttachIrqHandler(AdcReader *instance);

	/**
	 * @brief Dispatch HAL ADC callback to attached AdcReader instance.
	 * @param adc_handle ADC handle passed from HAL callback routine.
	 * @note Intended for use in HAL_ADC_ConvCpltCallback bridge.
	 */
	static void HandleInterrupt(ADC_HandleTypeDef *adc_handle);

private:
	struct ChannelData {
		uint32_t channel = 0U;
		float divider_ratio = 1.0F;
		volatile uint16_t raw_value = 0U;
		volatile uint16_t final_value = 0U;
		volatile bool data_available = false;
	};

	ADC_HandleTypeDef *adc_handle_ = nullptr;
	std::array<ChannelData, MAX_CHANNELS> channels_{};
	std::size_t channel_count_ = 0U;
	uint16_t reference_voltage_mv_ = 3000U;

	volatile bool conversion_running_ = false;
	volatile std::size_t active_channel_index_ = 0U;

	bool ConfigureAndStartChannel(std::size_t index);
	std::size_t FindChannelIndex(uint32_t channel) const;
	uint16_t ConvertRawToFinalMillivolts(uint16_t raw_value,
																			 float divider_ratio) const;

	static AdcReader *irq_instance_;
};
