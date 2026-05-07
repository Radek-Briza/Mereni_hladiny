/*
 * DataTransmit.hpp
 *
 *  Created on: 21. 4. 2026
 *      Author: radek
 */

#ifndef DATATRANSMIT_HPP_
#define DATATRANSMIT_HPP_

#include "../Core/Inc/main.h" 
#include "app_subghz_phy.h"
#include "subghz_phy_app.h"
#include "radio.h"

class DataTransmit {
	DataTransmit(const struct Radio_s *Radio_) : Radio(Radio_){};
	void Init();

private:
	const struct Radio_s *Radio=NULL;
	RadioEvents_t RadioEvents;



};

extern "C" {
void OnTxDone(void);
void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t LoraSnr_FskCfo);
void OnTxTimeout(void);
void OnRxTimeout(void);
void OnRxError(void);
}

#endif /* DATATRANSMIT_HPP_ */
