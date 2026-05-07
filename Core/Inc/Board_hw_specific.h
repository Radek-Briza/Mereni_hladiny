/*
 * Board_hw_specific.h
 *
 *  Created on: 19. 4. 2026
 *      Author: radek
 */

#ifndef INC_BOARD_HW_SPECIFIC_H_
#define INC_BOARD_HW_SPECIFIC_H_


#define  TRIGER_PORT GPIOB
#define  ECHO_PORT  GPIOA
#define  TRIGER_PIN GPIO_PIN_5  //5
#define  ECHO_PIN   GPIO_PIN_8 // 8 PA8


#define TRIGER	0
#define ECHO	1

static GPIO_TypeDef* SRF05_PORT[2] = {TRIGER_PORT, ECHO_PORT};
static const uint16_t SRF05_PIN[2] = {TRIGER_PIN,ECHO_PIN};

#endif /* INC_BOARD_HW_SPECIFIC_H_ */
