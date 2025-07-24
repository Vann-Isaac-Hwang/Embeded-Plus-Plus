#include "PE4302.hpp"

PE4302::PE4302(GPIO_TypeDef* port, uint16_t lePin, uint16_t clkPin, uint16_t dataPin)
    : m_port(port), m_lePin(lePin), m_clkPin(clkPin), m_dataPin(dataPin) {
}

bool PE4302::init() {
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
    // Add more ports as needed
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    // Configure LE pin
    GPIO_InitStruct.Pin = m_lePin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(m_port, &GPIO_InitStruct);
    
    // Configure CLK pin
    GPIO_InitStruct.Pin = m_clkPin;
    HAL_GPIO_Init(m_port, &GPIO_InitStruct);
    
    // Configure DATA pin
    GPIO_InitStruct.Pin = m_dataPin;
    HAL_GPIO_Init(m_port, &GPIO_InitStruct);
    
    // Set initial states
    HAL_GPIO_WritePin(m_port, m_lePin, GPIO_PIN_SET);   // LE high
    HAL_GPIO_WritePin(m_port, m_clkPin, GPIO_PIN_RESET);  // CLK low
    HAL_GPIO_WritePin(m_port, m_dataPin, GPIO_PIN_RESET); // DATA low

    m_currentAttenuation = MIN_ATTENUATION;
    return true;
}

uint8_t PE4302::dBToRegValue(float dB) const {
    // 手动实现范围限制
    if (dB < MIN_ATTENUATION) {
        dB = MIN_ATTENUATION;
    } else if (dB > MAX_ATTENUATION) {
        dB = MAX_ATTENUATION;
    }

    // 四舍五入到最近的0.5dB步进
    return static_cast<uint8_t>(roundf(dB / ATTEN_STEP));
}

float PE4302::regValueTodB(uint8_t regValue) const {
    regValue = std::min(regValue, static_cast<uint8_t>(MAX_ATTENUATION / ATTEN_STEP));
    return regValue * ATTEN_STEP;
}

float PE4302::setAttenuation(float dB) {
    // 转换为寄存器值（0-63对应0-31.5dB）
    uint8_t regValue = dBToRegValue(dB);

    // 计算实际设置的衰减值
    m_currentAttenuation = regValueTodB(regValue);

    // PE4302需要6位数据（D5-D0），格式为：
    // D5 D4 D3 D2 D1 D0 = 衰减值（0-63对应0-31.5dB）
    // 数据需要左移2位（因为D7-D6是保留位）
    uint8_t data = regValue << 2;

    // 开始传输
    HAL_GPIO_WritePin(m_port, m_lePin, GPIO_PIN_RESET);
    delayUs(1);

    // 发送6位数据（MSB first）
    for (uint8_t i = 0; i < 6; i++) {
        writeBit(data & 0x80);
        data <<= 1;
        pulseClock();
    }

    // 结束传输
    HAL_GPIO_WritePin(m_port, m_dataPin, GPIO_PIN_RESET);
    delayUs(1);
    HAL_GPIO_WritePin(m_port, m_lePin, GPIO_PIN_SET);
    delayUs(1);

    return m_currentAttenuation;
}

void PE4302::delayUs(uint32_t microseconds) {
    // Implement a microsecond delay (can use HAL_Delay or custom timer)
    // This is a placeholder - adjust based on your system
    uint32_t ticks = microseconds * (SystemCoreClock / 1000000) / 5;
    while(ticks--);
}

void PE4302::writeBit(bool bit) {
    HAL_GPIO_WritePin(m_port, m_dataPin, bit ? GPIO_PIN_SET : GPIO_PIN_RESET);
    delayUs(1);
}

void PE4302::pulseClock() {
    HAL_GPIO_WritePin(m_port, m_clkPin, GPIO_PIN_RESET);
    delayUs(1);
    HAL_GPIO_WritePin(m_port, m_clkPin, GPIO_PIN_SET);
    delayUs(1);
    HAL_GPIO_WritePin(m_port, m_clkPin, GPIO_PIN_RESET);
}
