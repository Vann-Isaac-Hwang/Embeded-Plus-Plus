/*
 * ADF4351.hpp
 *
 *  Created on: Jul 28, 2025
 *      Author: huang
 */

#ifndef INC_ADF4351_HPP_
#define INC_ADF4351_HPP_

#include "main.h"

class ADF4351 {
public:
	ADF4351(GPIO_TypeDef* port,
			uint16_t clkPin, uint16_t dataPin, uint16_t lePin, uint16_t cePin);
	void Init_GPIOs();
	void Init(unsigned int data);
	void Wdata(unsigned int dat);

	void Delay_1us(unsigned char t);
private:
	const unsigned int R = 125;
	GPIO_TypeDef* m_port;
	uint16_t m_clkPin;
	uint16_t m_dataPin;
	uint16_t m_lePin;
	uint16_t m_cePin;
};


#endif /* INC_ADF4351_HPP_ */
