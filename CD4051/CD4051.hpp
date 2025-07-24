/*
 * CD4051.hpp
 *
 *  Created on: Jul 24, 2025
 *      Author: huang
 */

#ifndef INC_CD4051_HPP_
#define INC_CD4051_HPP_

#include "main.h"
#include <cstdint>

class CD4051 {
public:
	CD4051(GPIO_TypeDef* port, uint16_t PinA, uint16_t PinB, uint16_t PinC);
	bool init();
	bool setChannel(uint8_t channel);
	uint8_t getChannel();
private:
	GPIO_TypeDef* m_port;
	uint16_t m_PinA;
	uint16_t m_PinB;
	uint16_t m_PinC;
	uint8_t m_channel;
};

#endif /* INC_CD4051_HPP_ */
