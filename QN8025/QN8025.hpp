/*
 * QN8025.hpp
 *
 *  Created on: Jan 21, 2026
 *      Author: huang
 */

#ifndef INC_QN8025_HPP_
#define INC_QN8025_HPP_

#include "main.h"
#include <functional>      // 用于回调函数 (可选，如果不用标准库可用函数指针)

// 如果不支持 std::function，可以使用传统的函数指针:
// typedef void (*DebugCallback)(const char* msg);

class QN8025 {
public:
    // 构造函数，传入 I2C 句柄
    QN8025(I2C_HandleTypeDef* hi2c);

    // 初始化检查
    bool begin();

    // 核心功能
    void setFrequency(float mhz);
    float getFrequency() const;
    uint8_t getRSSI();

    // 自动搜台功能
    // step_kHz: 50, 100, 200
    float autoSeek(float startMHz, float stopMHz, int step_kHz = 100);

    // 高级功能：全频段扫描最强台并锁定
    // 返回找到的频率，失败返回 0.0
    float scanAndLockBest(float startMHz = 87.0f, float stopMHz = 108.0f);

    // 信号监控（用于主循环）
    // threshold: 掉线阈值，intervalMs: 检查间隔
    // 返回 true 表示信号良好，false 表示触发了重搜
    bool monitorSignal(uint8_t threshold = 40, uint32_t intervalMs = 1000);

    // 调试回调注册 (可选)
    // 允许外部注入打印函数，例如: radio.setDebugLog(USART_printf_wrapper);
    typedef void (*LogCallback)(const char* fmt, ...);
    void setDebugLog(LogCallback cb);

private:
    I2C_HandleTypeDef* _hi2c;
    const uint8_t _address = (0x10 << 1); // 0x20
    float _currentFreq;
    LogCallback _logger;

    // 寄存器定义
    static const uint8_t REG_SYSTEM1  = 0x00;
    static const uint8_t REG_CCA      = 0x01;
    static const uint8_t REG_RSSISIG  = 0x03;
    static const uint8_t REG_STATUS1  = 0x04;
    static const uint8_t REG_CID1     = 0x05;
    static const uint8_t REG_CID2     = 0x06;
    static const uint8_t REG_CH       = 0x07;
    static const uint8_t REG_CH_START = 0x08;
    static const uint8_t REG_CH_STOP  = 0x09;
    static const uint8_t REG_CH_STEP  = 0x0A;
    static const uint8_t REG_VOL_CTL  = 0x14;

    // 内部辅助函数
    void writeReg(uint8_t reg, uint8_t val);
    uint8_t readReg(uint8_t reg);
    void log(const char* fmt, ...); // 内部使用的日志包装器
};

#endif /* INC_QN8025_HPP_ */
