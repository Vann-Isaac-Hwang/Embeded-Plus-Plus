/*
 * QN8025.cpp
 *
 *  Created on: Jan 21, 2026
 *      Author: huang
 */

#include <QN8025.hpp>
#include <cstdio>
#include <cstdarg>

QN8025::QN8025(I2C_HandleTypeDef* hi2c) : _hi2c(hi2c), _currentFreq(0.0f), _logger(nullptr) {}

void QN8025::setDebugLog(LogCallback cb) {
    _logger = cb;
}

// 内部日志辅助，如果有注册回调则调用，否则忽略
void QN8025::log(const char* fmt, ...) {
    if (_logger) {
        char buffer[128];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buffer, sizeof(buffer), fmt, args);
        va_end(args);
        _logger("%s", buffer); // 调用外部传入的回调
    }
}

void QN8025::writeReg(uint8_t reg, uint8_t val) {
    HAL_I2C_Mem_Write(_hi2c, _address, reg, I2C_MEMADD_SIZE_8BIT, &val, 1, 100);
}

uint8_t QN8025::readReg(uint8_t reg) {
    uint8_t val = 0;
    HAL_I2C_Mem_Read(_hi2c, _address, reg, I2C_MEMADD_SIZE_8BIT, &val, 1, 100);
    return val;
}

bool QN8025::begin() {
    log("Checking QN8025...\r\n");
    if (HAL_I2C_IsDeviceReady(_hi2c, _address, 3, 100) != HAL_OK) {
        log("[ERR] Device not found!\r\n");
        return false;
    }

    uint8_t cid1 = readReg(REG_CID1);
    uint8_t cid2 = readReg(REG_CID2);
    log("[OK] ID: 0x%02X 0x%02X\r\n", cid1, cid2);

    // 复位或初始化配置可放在这里
    return true;
}

void QN8025::setFrequency(float mhz) {
    if (mhz < 61.75f || mhz > 108.0f) return;

    _currentFreq = mhz;
    uint16_t idx = (uint16_t)((mhz - 61.75f) / 0.05f);

    uint8_t reg07 = idx & 0xFF;
    uint8_t reg0A = (idx >> 8) & 0x03;

    log("Set Freq: %d.%02d MHz\r\n", (int)mhz, (int)((mhz-(int)mhz)*100));

    writeReg(REG_CH, reg07);

    uint8_t temp0A = readReg(REG_CH_STEP);
    temp0A &= 0xFC;
    temp0A |= reg0A;
    writeReg(REG_CH_STEP, temp0A);

    // RXREQ=1, CHSC=0, CCA_DIS=1 (Manual)
    writeReg(REG_SYSTEM1, 0x11);
}

float QN8025::getFrequency() const {
    return _currentFreq;
}

uint8_t QN8025::getRSSI() {
    return readReg(REG_RSSISIG);
}

float QN8025::autoSeek(float startMHz, float stopMHz, int step_kHz) {
    uint16_t startIdx = (uint16_t)((startMHz - 61.75f) / 0.05f);
    uint16_t stopIdx  = (uint16_t)((stopMHz - 61.75f) / 0.05f);

    uint8_t stepVal = 0x00; // 50k
    if (step_kHz == 100) stepVal = 0x40;
    else if (step_kHz == 200) stepVal = 0x80;

    writeReg(REG_CH_START, startIdx & 0xFF);
    writeReg(REG_CH_STOP, stopIdx & 0xFF);

    uint8_t reg0A = stepVal | (((stopIdx >> 8) & 0x03) << 4) | (((startIdx >> 8) & 0x03) << 2);
    writeReg(REG_CH_STEP, reg0A);

    writeReg(REG_CCA, 0x10); // Threshold

    // Start Seek: RXREQ=1, CHSC=1, CCA_DIS=0
    writeReg(REG_SYSTEM1, 0x12);

    // Wait
    uint32_t timeout = 500;
    while ((readReg(REG_SYSTEM1) & 0x02) && timeout--) {
        HAL_Delay(10);
    }

    if (timeout == 0) return 0.0f;

    // Check Success
    if ((readReg(REG_STATUS1) & 0x08) == 0) {
        uint8_t chLow = readReg(REG_CH);
        uint8_t chHigh = readReg(REG_CH_STEP) & 0x03;
        uint16_t idx = chLow | (chHigh << 8);
        float found = 61.75f + (idx * 0.05f);
        _currentFreq = found;
        return found;
    }

    return 0.0f;
}

float QN8025::scanAndLockBest(float startMHz, float stopMHz) {
    float currentPtr = startMHz;
    float bestFreq = 0.0f;
    uint8_t maxRSSI = 0;

    log("Scanning %d.%02d - %d.%02d MHz...\r\n",
        (int)startMHz, (int)((startMHz-(int)startMHz)*100),
        (int)stopMHz, (int)((stopMHz-(int)stopMHz)*100));

    while (currentPtr < stopMHz) {
        float found = autoSeek(currentPtr, stopMHz, 100);
        if (found > 0.0f) {
            uint8_t rssi = getRSSI();
            log("  Found: %d.%02d MHz (RSSI: %d)\r\n",
                (int)found, (int)((found-(int)found)*100), rssi);

            if (rssi > maxRSSI) {
                maxRSSI = rssi;
                bestFreq = found;
            }
            currentPtr = found + 0.1f;
        } else {
            break;
        }
    }

    if (bestFreq > 0.0f) {
        log("Locked Best: %d.%02d MHz (RSSI: %d)\r\n",
             (int)bestFreq, (int)((bestFreq-(int)bestFreq)*100), maxRSSI);
        setFrequency(bestFreq);
        return bestFreq;
    }

    log("No station found.\r\n");
    return 0.0f;
}

bool QN8025::monitorSignal(uint8_t threshold, uint32_t intervalMs) {
    static uint32_t lastCheck = 0;

    // 非阻塞式时间检查
    if (HAL_GetTick() - lastCheck < intervalMs) {
        return true; // 还没到时间，认为正常
    }
    lastCheck = HAL_GetTick();

    if (_currentFreq == 0.0f) return false;

    uint8_t rssi = getRSSI();
    log("Monitor: %d.%02d MHz, RSSI: %d\r\n",
        (int)_currentFreq, (int)((_currentFreq-(int)_currentFreq)*100), rssi);

    if (rssi < threshold) {
        log("Signal weak! Rescanning...\r\n");
        scanAndLockBest(); // 重新全频扫描 (使用默认参数)
        return false; // 指示发生了重搜
    }

    return true;
}
