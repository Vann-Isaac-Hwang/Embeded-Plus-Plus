#include "HMC241.hpp"

HMC241::HMC241(GPIO_TypeDef* port, uint16_t PinA, uint16_t PinB)
	: m_port(port), m_PinA(PinA), m_PinB(PinB) {
}

bool HMC241::init()
{
    // Enable GPIO clock
    if (m_port == GPIOA) {
        __HAL_RCC_GPIOA_CLK_ENABLE();
    } else if (m_port == GPIOB) {
        __HAL_RCC_GPIOB_CLK_ENABLE();
    } else if (m_port == GPIOC) {
        __HAL_RCC_GPIOC_CLK_ENABLE();
    } else if (m_port == GPIOD) {
        __HAL_RCC_GPIOD_CLK_ENABLE();
    } else if (m_port == GPIOE) {
        __HAL_RCC_GPIOE_CLK_ENABLE();
    } else if (m_port == GPIOF) {
        __HAL_RCC_GPIOF_CLK_ENABLE();
    } else if (m_port == GPIOG) {
        __HAL_RCC_GPIOG_CLK_ENABLE();
    }

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // Configure pin A
    GPIO_InitStruct.Pin = m_PinA;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(m_port, &GPIO_InitStruct);

    // Configure pin B
    GPIO_InitStruct.Pin = m_PinB;
    HAL_GPIO_Init(m_port, &GPIO_InitStruct);

    // Set initial states
    HAL_GPIO_WritePin(m_port, m_PinA, GPIO_PIN_RESET);   // A low
    HAL_GPIO_WritePin(m_port, m_PinB, GPIO_PIN_RESET);  // B low

    m_channel = 1;
    return true;
}

bool HMC241::setChannel(uint8_t channel)
{
	switch(channel)
	{
		case 1:
			HAL_GPIO_WritePin(m_port, m_PinA, GPIO_PIN_RESET);   // A low
			HAL_GPIO_WritePin(m_port, m_PinB, GPIO_PIN_RESET);  // B low
			break;
		case 2:
			HAL_GPIO_WritePin(m_port, m_PinA, GPIO_PIN_SET);   // A high
			HAL_GPIO_WritePin(m_port, m_PinB, GPIO_PIN_RESET);  // B low
			break;
		case 3:
			HAL_GPIO_WritePin(m_port, m_PinA, GPIO_PIN_RESET);   // A low
			HAL_GPIO_WritePin(m_port, m_PinB, GPIO_PIN_SET);  // B high
			break;
		case 4:
			HAL_GPIO_WritePin(m_port, m_PinA, GPIO_PIN_SET);   // A high
			HAL_GPIO_WritePin(m_port, m_PinB, GPIO_PIN_SET);  // B high
			break;
		default:
			return false;
	}
	return true;
}

uint8_t HMC241::getChannel()
{
	return m_channel;
}
