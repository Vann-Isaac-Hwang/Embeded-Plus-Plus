/*
 * AD9833.h
 *
 *  Created on: Jul 23, 2025
 *      Author: huang
 */

#ifndef AD9833_AD9833_HPP_
#define AD9833_AD9833_HPP_

#include <cmath>
#include <cstdint>
#include "stm32f4xx_hal.h"

class AD9833 {
public:
    enum WaveType { SIN, SQR, TRI };

    // 构造函数现在只需要端口和引脚定义
    AD9833(GPIO_TypeDef* port, uint16_t dataPin, uint16_t sckPin, uint16_t ssPin,
           uint32_t masterClock = 25000000);

    // 初始化函数现在包含硬件初始化
    bool init(WaveType waveType, float frequency, float phase);

    void setWave(WaveType waveType);
    void setWaveData(float frequency, float phase);

private:
    void writeSPI(uint16_t word);
    void initGPIO();  // 新增GPIO初始化私有方法

    GPIO_TypeDef* m_port;
    uint16_t m_dataPin;
    uint16_t m_sckPin;
    uint16_t m_ssPin;
    uint32_t m_masterClock;
    WaveType m_currentWave;
};

#endif /* AD9833_AD9833_H_ */
