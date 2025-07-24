/*
 * HMC241.hpp
 *
 *  Created on: Jul 24, 2025
 *      Author: huang
 */

#ifndef INC_HMC241_HPP_
#define INC_HMC241_HPP_

#include "main.h"
#include <cstdint>

class HMC241 {
public:
	HMC241(GPIO_TypeDef* port, uint16_t PinA, uint16_t PinB);
	bool init();
	bool setChannel(uint8_t channel);
	uint8_t getChannel();
private:
	GPIO_TypeDef* m_port;
	uint16_t m_PinA;
	uint16_t m_PinB;
	uint8_t m_channel;
};

#endif /* INC_HMC241_HPP_ */
