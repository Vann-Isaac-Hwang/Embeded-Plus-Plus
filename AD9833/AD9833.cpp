/*
 * AD9833.cpp
 *
 *  Created on: Jul 23, 2025
 *      Author: huang
 */

#include "AD9833.hpp"

AD9833::AD9833(GPIO_TypeDef* port, uint16_t dataPin, uint16_t sckPin, uint16_t ssPin,
               uint32_t masterClock)
    : m_port(port), m_dataPin(dataPin), m_sckPin(sckPin), m_ssPin(ssPin),
      m_masterClock(masterClock), m_currentWave(SIN) {
    // 构造函数中不进行硬件初始化，留到init()函数中
}

void AD9833::initGPIO() {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // 确保GPIO时钟已使能
    if (m_port == GPIOA) {
        __HAL_RCC_GPIOA_CLK_ENABLE();
    } else if (m_port == GPIOB) {
        __HAL_RCC_GPIOB_CLK_ENABLE();
    } else if (m_port == GPIOC) {
        __HAL_RCC_GPIOC_CLK_ENABLE();
    }
    // 可根据需要添加更多端口

    // 配置DATA引脚
    GPIO_InitStruct.Pin = m_dataPin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(m_port, &GPIO_InitStruct);

    // 配置SCK引脚
    GPIO_InitStruct.Pin = m_sckPin;
    HAL_GPIO_Init(m_port, &GPIO_InitStruct);

    // 配置SS引脚
    GPIO_InitStruct.Pin = m_ssPin;
    HAL_GPIO_Init(m_port, &GPIO_InitStruct);

    // 初始状态设置
    HAL_GPIO_WritePin(m_port, m_dataPin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(m_port, m_sckPin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(m_port, m_ssPin, GPIO_PIN_SET);
}

bool AD9833::init(WaveType waveType, float frequency, float phase) {
    // 初始化GPIO
    initGPIO();

    // 设置初始波形
    setWave(waveType);

    // 设置初始频率和相位
    setWaveData(frequency, phase);

    return true; // 可以扩展为返回初始化状态
}

void AD9833::writeSPI(uint16_t word) {
    for (uint8_t i = 0; i < 16; i++) {
        if (word & 0x8000) {
            HAL_GPIO_WritePin(m_port, m_dataPin, GPIO_PIN_SET);   // bit=1, Set High
        } else {
            HAL_GPIO_WritePin(m_port, m_dataPin, GPIO_PIN_RESET); // bit=0, Set Low
        }
        __NOP();

        HAL_GPIO_WritePin(m_port, m_sckPin, GPIO_PIN_RESET);     // Data is valid on falling edge
        __NOP();
        HAL_GPIO_WritePin(m_port, m_sckPin, GPIO_PIN_SET);
        word = word << 1; // Shift left by 1 bit
    }
    HAL_GPIO_WritePin(m_port, m_dataPin, GPIO_PIN_RESET);        // Idle low
    __NOP();
}

void AD9833::setWave(WaveType waveType) {
    m_currentWave = waveType;

    HAL_GPIO_WritePin(m_port, m_ssPin, GPIO_PIN_RESET);
    switch (waveType) {
    case SIN:
        writeSPI(0x2000); // Value for Sinusoidal Wave
        break;
    case SQR:
        writeSPI(0x2028); // Value for Square Wave
        break;
    case TRI:
        writeSPI(0x2002); // Value for Triangle Wave
        break;
    }
    HAL_GPIO_WritePin(m_port, m_ssPin, GPIO_PIN_SET);
}

void AD9833::setWaveData(float frequency, float phase) {
    __NOP();

    // ---------- Tuning Word for Phase (0 - 360 Degree)
    if (phase < 0) phase = 0;
    if (phase > 360) phase = 360;
    uint32_t phaseVal = ((int)(phase * (4096.0f / 360.0f))) | 0xC000;

    // ---------- Tuning word for Frequency
    long freq = 0;
    freq = (int)(((frequency * pow(2, 28)) / m_masterClock) + 1);
    uint16_t FRQHW = (int)((freq & 0xFFFC000) >> 14); // FREQ MSB
    uint16_t FRQLW = (int)(freq & 0x3FFF);           // FREQ LSB
    FRQLW |= 0x4000;
    FRQHW |= 0x4000;

    // ------------------------------------------------ Writing DATA
    HAL_GPIO_WritePin(m_port, m_dataPin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(m_port, m_sckPin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(m_port, m_ssPin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(m_port, m_ssPin, GPIO_PIN_RESET); // low = selected
    __NOP();

    writeSPI(0x2100); // enable 16bit words and set reset bit
    writeSPI(FRQLW);
    writeSPI(FRQHW);
    writeSPI(phaseVal);
    writeSPI(0x2000); // clear reset bit
    __NOP();
    HAL_GPIO_WritePin(m_port, m_ssPin, GPIO_PIN_SET); // high = deselected

    setWave(m_currentWave);
    __NOP();
}


