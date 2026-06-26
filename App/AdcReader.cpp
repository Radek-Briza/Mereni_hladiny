#include "AdcReader.hpp"

#include <cmath>
#include <limits>

namespace {
constexpr uint16_t ADC_MAX_12BIT = 4095U;
}

AdcReader *AdcReader::irq_instance_ = nullptr;

AdcReader::AdcReader(ADC_HandleTypeDef *adc_handle,
                     std::span<const ChannelConfig> channels,
                     uint16_t reference_voltage_mv)
    : adc_handle_(adc_handle), reference_voltage_mv_(reference_voltage_mv) {
  const std::size_t accepted_channels =
      (channels.size() < MAX_CHANNELS) ? channels.size() : MAX_CHANNELS;

  for (std::size_t i = 0; i < accepted_channels; ++i) {
    channels_[i].channel = channels[i].channel;
    channels_[i].divider_ratio =
        (channels[i].divider_ratio > 0.0F) ? channels[i].divider_ratio : 1.0F;
    channels_[i].raw_value = 0U;
    channels_[i].final_value = 0U;
    channels_[i].data_available = false;
  }

  channel_count_ = accepted_channels;
}

AdcReader::AdcReader(ADC_HandleTypeDef *adc_handle,
                     std::initializer_list<ChannelConfig> channels,
                     uint16_t reference_voltage_mv)
    : AdcReader(adc_handle,
                std::span<const ChannelConfig>(channels.begin(), channels.size()),
                reference_voltage_mv) {}

bool AdcReader::Start() {
  if ((adc_handle_ == nullptr) || (channel_count_ == 0U) || conversion_running_) {
    return false;
  }

  for (std::size_t i = 0; i < channel_count_; ++i) {
    channels_[i].data_available = false;
  }

  active_channel_index_ = 0U;
  conversion_running_ = true;

  if (!ConfigureAndStartChannel(active_channel_index_)) {
    conversion_running_ = false;
    return false;
  }

  return true;
}

AdcReader::Result AdcReader::GetFinalValue(uint32_t channel, uint16_t &value_out) {
  const std::size_t index = FindChannelIndex(channel);
  if (index >= channel_count_) {
    return Result::CHANNEL_NOT_AVAILABLE;
  }

  if (!channels_[index].data_available) {
    return Result::DATA_NOT_AVAILABLE;
  }

  value_out = channels_[index].final_value;
  channels_[index].data_available = false;
  return Result::RESULT_OK;
}

AdcReader::Result AdcReader::GetRawValue(uint32_t channel,
                                         uint16_t &value_out) const {
  const std::size_t index = FindChannelIndex(channel);
  if (index >= channel_count_) {
    return Result::CHANNEL_NOT_AVAILABLE;
  }

  if (!channels_[index].data_available) {
    return Result::DATA_NOT_AVAILABLE;
  }

  value_out = channels_[index].raw_value;
  return Result::RESULT_OK;
}

void AdcReader::AdcCallback(ADC_HandleTypeDef *adc_handle) {
  if ((adc_handle == nullptr) || (adc_handle != adc_handle_) ||
      !conversion_running_ || (active_channel_index_ >= channel_count_)) {
    return;
  }

  const uint16_t raw_value = static_cast<uint16_t>(HAL_ADC_GetValue(adc_handle));
  ChannelData &channel = channels_[active_channel_index_];
  channel.raw_value = raw_value;
  channel.final_value = ConvertRawToFinalMillivolts(raw_value, channel.divider_ratio);
  channel.data_available = true;

  active_channel_index_ = active_channel_index_ + 1U;
  if (active_channel_index_ >= channel_count_) {
    conversion_running_ = false;
    return;
  }

  if (!ConfigureAndStartChannel(active_channel_index_)) {
    conversion_running_ = false;
  }
}

bool AdcReader::ConfigureAndStartChannel(std::size_t index) {
  if ((adc_handle_ == nullptr) || (index >= channel_count_)) {
    return false;
  }

  ADC_ChannelConfTypeDef channel_config{};
  channel_config.Channel = channels_[index].channel;
  channel_config.Rank = ADC_REGULAR_RANK_1;
  channel_config.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;

  if (HAL_ADC_ConfigChannel(adc_handle_, &channel_config) != HAL_OK) {
    return false;
  }

  if (HAL_ADC_Start_IT(adc_handle_) != HAL_OK) {
    return false;
  }

  return true;
}

std::size_t AdcReader::FindChannelIndex(uint32_t channel) const {
  for (std::size_t i = 0; i < channel_count_; ++i) {
    if (channels_[i].channel == channel) {
      return i;
    }
  }

  return channel_count_;
}

uint16_t AdcReader::ConvertRawToFinalMillivolts(uint16_t raw_value,
                                                float divider_ratio) const {
  const float adc_mv =
      static_cast<float>(raw_value) * static_cast<float>(reference_voltage_mv_) /
      static_cast<float>(ADC_MAX_12BIT);
  const float final_mv = adc_mv * divider_ratio;

  if (final_mv <= 0.0F) {
    return 0U;
  }

  const float max_u16 = static_cast<float>(std::numeric_limits<uint16_t>::max());
  if (final_mv >= max_u16) {
    return std::numeric_limits<uint16_t>::max();
  }

  return static_cast<uint16_t>(std::lround(final_mv));
}

void AdcReader::AttachIrqHandler(AdcReader *instance) { irq_instance_ = instance; }

void AdcReader::HandleInterrupt(ADC_HandleTypeDef *adc_handle) {
  if (irq_instance_ != nullptr) {
    irq_instance_->AdcCallback(adc_handle);
  }
}