/*
 * CD4051.cpp
 *
 *  Created on: Jul 24, 2025
 *      Author: huang
 */

#include "CD4051.hpp"

CD4051::CD4051(GPIO_TypeDef* port, uint16_t PinA, uint16_t PinB, uint16_t PinC)
	: m_port(port), m_PinA(PinA), m_PinB(PinB), m_PinC(PinC) {
}

bool CD4051::init()
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

    // Configure pin C
    GPIO_InitStruct.Pin = m_PinC;
    HAL_GPIO_Init(m_port, &GPIO_InitStruct);

    // Set initial states
    HAL_GPIO_WritePin(m_port, m_PinA, GPIO_PIN_RESET);   // A low
    HAL_GPIO_WritePin(m_port, m_PinB, GPIO_PIN_RESET);  // B low
    HAL_GPIO_WritePin(m_port, m_PinC, GPIO_PIN_RESET);  // C low

    m_channel = 1;
    return true;
}

bool CD4051::setChannel(uint8_t channel)
{
    switch(channel) {
        case 0:  // CBA=000
            HAL_GPIO_WritePin(m_port, m_PinC, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(m_port, m_PinB, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(m_port, m_PinA, GPIO_PIN_RESET);
            break;
        case 1:  // CBA=001
            HAL_GPIO_WritePin(m_port, m_PinC, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(m_port, m_PinB, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(m_port, m_PinA, GPIO_PIN_SET);
            break;
        case 2:  // CBA=010
            HAL_GPIO_WritePin(m_port, m_PinC, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(m_port, m_PinB, GPIO_PIN_SET);
            HAL_GPIO_WritePin(m_port, m_PinA, GPIO_PIN_RESET);
            break;
        case 3:  // CBA=011
            HAL_GPIO_WritePin(m_port, m_PinC, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(m_port, m_PinB, GPIO_PIN_SET);
            HAL_GPIO_WritePin(m_port, m_PinA, GPIO_PIN_SET);
            break;
        case 4:  // CBA=100
            HAL_GPIO_WritePin(m_port, m_PinC, GPIO_PIN_SET);
            HAL_GPIO_WritePin(m_port, m_PinB, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(m_port, m_PinA, GPIO_PIN_RESET);
            break;
        case 5:  // CBA=101
            HAL_GPIO_WritePin(m_port, m_PinC, GPIO_PIN_SET);
            HAL_GPIO_WritePin(m_port, m_PinB, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(m_port, m_PinA, GPIO_PIN_SET);
            break;
        case 6:  // CBA=110
            HAL_GPIO_WritePin(m_port, m_PinC, GPIO_PIN_SET);
            HAL_GPIO_WritePin(m_port, m_PinB, GPIO_PIN_SET);
            HAL_GPIO_WritePin(m_port, m_PinA, GPIO_PIN_RESET);
            break;
        case 7:  // CBA=111
            HAL_GPIO_WritePin(m_port, m_PinC, GPIO_PIN_SET);
            HAL_GPIO_WritePin(m_port, m_PinB, GPIO_PIN_SET);
            HAL_GPIO_WritePin(m_port, m_PinA, GPIO_PIN_SET);
            break;
        default:
            return false;  // 无效通道号
    }
    return true;
}

uint8_t CD4051::getChannel()
{
	return m_channel;
}
