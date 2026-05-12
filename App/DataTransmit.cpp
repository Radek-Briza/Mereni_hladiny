/*
 * DataTransmit.cpp
 *
 *  Created on: 21. 4. 2026
 *      Author: radek
 */

#include  "DataTransmit.hpp"
#include  "RadioParams.hpp"
#include "timer.h"
#include <cassert>
#include <cstring>
#include <cstdio>

bool DataTransmit::DataAvailable = false;
bool DataTransmit::DataOverload = false;
const struct Radio_s* DataTransmit::RadioDriver = nullptr;
RadioEvents_t DataTransmit::RadioEvents = {};
TimerEvent_t DataTransmit::CadTimer = {};
Packet DataTransmit::packet = {};
std::array<uint8_t, Packet::max_packet_size> DataTransmit::Data = {};
Packet::PacketType DataTransmit::DataType = Packet::Data_level;

extern "C" void TimerExpiryCallback(void *context){
	// This function will be called when the timer expires
	// You can add any code here that you want to execute when the timer expires
	printf("Timer expired, executing callback\n");
}

extern "C"  [[maybe_unused]] void RadioCadTimeoutIrq( void *context ){
	DataTransmit::RadioDriver->Sleep( );
	DataTransmit::RadioDriver->StartCad( );
	printf("Start CAD\n");
}

   
extern "C" void OnCadDone( bool channelActivityDetected ){
	if(channelActivityDetected){
		DataTransmit::RadioDriver->Rx(100);
		printf("Start RX\n");
	}else{
		// Channel is clear, proceed with transmission
		TimerStart(&DataTransmit::CadTimer );
		printf("Channel is clear\n");	
	}
}


void OnTxDone(void){
	DataTransmit::RadioDriver->Standby( );
	TimerStart(&DataTransmit::CadTimer );	
	printf("Transmission done, restarting CAD\n");
};


extern "C" void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t LoraSnr_FskCfo){
	printf("Received packet: size=%u, rssi=%d, snr=%d\n", size, rssi, LoraSnr_FskCfo);
	// kopírujeme payload do packetu pro další zpracování
	std::array<uint8_t, Packet::max_packet_size> buffer{};
	std::memcpy(buffer.data(), payload, size);
	if(DataTransmit::packet.ParsePacket(buffer, DataTransmit::DataType, DataTransmit::Data)){
		if(DataTransmit::DataAvailable){
			DataTransmit::DataOverload = true;
		}
		else DataTransmit::DataOverload = false;
		DataTransmit::DataAvailable = true;
	}
	DataTransmit::RadioDriver->Standby( );
}

extern "C" void OnTxTimeout(void){
	DataTransmit::RadioDriver->Standby( );
	TimerStart(&DataTransmit::CadTimer );	
	printf("TX Timeout, restarting CAD\n");
}

extern "C" void OnRxTimeout(void){
	DataTransmit::RadioDriver->Standby( );
	TimerStart(&DataTransmit::CadTimer );	
	printf("RX Timeout, restarting CAD\n");
}

extern "C" void OnRxError(void){
	DataTransmit::RadioDriver->Standby( );
	TimerStart(&DataTransmit::CadTimer );	
	printf("RX Error, restarting CAD\n");
}


void DataTransmit::Init(const struct Radio_s *Radio_){
	assert(Radio_ != nullptr); 
	RadioDriver = Radio_;
	
	/* events  setup */
	RadioEvents.TxDone = OnTxDone;
	RadioEvents.RxDone = OnRxDone;
	RadioEvents.TxTimeout = OnTxTimeout;
	RadioEvents.RxTimeout = OnRxTimeout;
	RadioEvents.RxError = OnRxError;
	RadioEvents.CadDone = OnCadDone;
	RadioDriver->Init(&RadioEvents);

	/* radio setup */
	RadioDriver->SetTxConfig(MODEM_LORA,TX_POWER,0,BANDWIDTH,SPREED_FACTOR,CODE_RATE,PREAMBLE_LEN,FIX_LEN,CRC_ON,
	  									FREQ_HOP_ON,HOP_PERIODE,SYMBOL_INVERTED,10000);
	RadioDriver->SetChannel(CHANNEL);
	RadioDriver->SetRxConfig(MODEM_LORA, BANDWIDTH,SPREED_FACTOR, CODE_RATE,0,PREAMBLE_LEN*2,SYMB_TIMEOUT,FIX_LEN,PAYLOAD_LEN,
	  										CRC_ON, FREQ_HOP_ON, HOP_PERIODE,SYMBOL_INVERTED,false);
	RadioDriver->Standby( );
 	
	/* CAD sampler timer setup */
	TimerInit( &CadTimer,RadioCadTimeoutIrq );
	TimerSetValue( &CadTimer,CAD_sample );
    TimerStart( &CadTimer );	
	DataAvailable = false;		
	DataOverload = false;		
	printf("DataTransmit initialized\n");
}







