/*
 * DataTransmit.cpp
 *
 *  Created on: 21. 4. 2026
 *      Author: radek
 */

#include  "DataTransmit.hpp"
#include  "RadioParams.hpp"

void DataTransmit::Init(){
	//assert(Radio); 

	/* events  setup */
	RadioEvents.TxDone = OnTxDone;
	RadioEvents.RxDone = OnRxDone;
	RadioEvents.TxTimeout = OnTxTimeout;
	RadioEvents.RxTimeout = OnRxTimeout;
	RadioEvents.RxError = OnRxError;
	Radio->Init(&RadioEvents);

	/* radio setup */
	Radio->SetTxConfig(MODEM_LORA,TX_POWER,0,BANDWIDTH,SPREED_FACTOR,CODE_RATE,PREAMBLE_LEN,FIX_LEN,CRC_ON,
	  									FREQ_HOP_ON,HOP_PERIODE,SYMBOL_INVERTED,10000);
	Radio->SetChannel(CHANNEL);
	Radio->SetRxConfig(MODEM_LORA, BANDWIDTH,SPREED_FACTOR, CODE_RATE,0,PREAMBLE_LEN*2,SYMB_TIMEOUT,FIX_LEN,PAYLOAD_LEN,
	  										CRC_ON, FREQ_HOP_ON, HOP_PERIODE,SYMBOL_INVERTED,false);



}


void OnTxDone(void){

};

void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t LoraSnr_FskCfo){

}

void OnTxTimeout(void){

}
void OnRxTimeout(void){

}

void OnRxError(void){

}



