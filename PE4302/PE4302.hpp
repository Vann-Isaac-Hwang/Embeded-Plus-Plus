#ifndef PE4302_HPP
#define PE4302_HPP

#include "main.h"
#include <cstdint>
#include <algorithm>
#include <cmath>

class PE4302 {
public:
    // 最大/最小衰减常量
    static constexpr float MIN_ATTENUATION = 0.0f;    // 0dB (0b000000)
    static constexpr float MAX_ATTENUATION = 31.5f;   // 31.5dB (0b111111)
    static constexpr float ATTEN_STEP = 0.5f;         // 0.5dB步进

    PE4302(GPIO_TypeDef* port, uint16_t lePin, uint16_t clkPin, uint16_t dataPin);
    
    bool init();
    
    // 设置衰减值（0.0-31.5dB，步进0.5dB）
    // 返回实际设置的衰减值（四舍五入到最近的0.5dB）
    float setAttenuation(float dB);
    
    float getAttenuation() const { return m_currentAttenuation; }

private:
    void writeBit(bool bit);
    void pulseClock();
    uint8_t dBToRegValue(float dB) const;
    float regValueTodB(uint8_t regValue) const;
    
    GPIO_TypeDef* m_port;
    uint16_t m_lePin;
    uint16_t m_clkPin;
    uint16_t m_dataPin;
    float m_currentAttenuation = MIN_ATTENUATION;
    
    void delayUs(uint32_t microseconds);
};


#endif // PE4302_HPP
