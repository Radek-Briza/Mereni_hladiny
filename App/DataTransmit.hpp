/*
 * DataTransmit.hpp
 *
 *  Created on: 21. 4. 2026
 *      Author: radek
 */

#ifndef DATATRANSMIT_HPP_
#define DATATRANSMIT_HPP_

#include "radio.h"
#include "timer.h"
#include "Packet.hpp"

#ifdef __cplusplus
extern "C" {
#endif

void RadioCadTimeoutIrq(void *context);
void OnCadDone(bool channelActivityDetected);
void OnTxDone(void);
void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t LoraSnr_FskCfo);
void OnRxTimeout(void);
void OnRxError(void);
void OnTxTimeout(void);
#ifdef __cplusplus
}
#endif

class DataTransmit {	
public:
	void Init(const struct Radio_s *Radio_);
	static bool DataAvailable;
	static bool DataOverload; 

private:
	const uint32_t MaxPayloadSize = Packet::max_packet_size;
	static const struct Radio_s *RadioDriver;
	static const uint32_t CAD_sample = 200U; 
	static RadioEvents_t RadioEvents;
	static TimerEvent_t CadTimer;
	static Packet packet;
	static std::array<uint8_t, Packet::max_packet_size> Data; // Buffer pro příjem dat z rádia
    static Packet::PacketType DataType; // Proměnná pro uložení typu dat z příchozího packetu

	friend void RadioCadTimeoutIrq(void *context);
	friend void OnCadDone(bool channelActivityDetected);
	friend void OnTxTimeout(void);
	friend void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t LoraSnr_FskCfo);
	friend void OnRxTimeout(void);
	friend void OnRxError(void);
	friend void OnTxDone(void);
};

#endif /* DATATRANSMIT_HPP_ */
