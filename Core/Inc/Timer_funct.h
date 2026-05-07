/*
 * Timer_funct.h
 *
 *  Created on: 19. 4. 2026
 *      Author: radek
 */

#ifndef INC_TIMER_FUNCT_H_
#define INC_TIMER_FUNCT_H_

extern "C" {
	void delayMicroseconds(uint16_t us);
	uint32_t GetMillis(void);
	uint16_t GetCapture(void);
	void StartCapture(void);
	void StopCapture(void);
}

#endif /* INC_TIMER_FUNCT_H_ */
