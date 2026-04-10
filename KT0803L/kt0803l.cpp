/*
 * kt0803l.cpp
 *
 *  Created on: Apr 10, 2026
 *      Author: huang
 */

#include "kt0803l.hpp"
#include <stdio.h>

// KT0803L的最大寄存器地址为0x27
static uint8_t KT0803L_Regs[0x28];

KT0803L::KT0803L(I2C_HandleTypeDef* i2c) : i2cHandle(i2c)
{
    for (size_t i = 0; i < sizeof(KT0803L_Regs) / sizeof(KT0803L_Regs[0]); i++)
    {
        KT0803L_Regs[i] = 0;
    }
}

KT0803L::~KT0803L()
{
}

// 注意：KT0803L的寄存器是8位的，与RDA5820的16位不同
HAL_StatusTypeDef KT0803L::_KT0803L_HW_WriteReg(uint8_t addr, uint8_t val)
{
    return HAL_I2C_Mem_Write(i2cHandle, KT0803L_I2C_ADDR_WRITE, addr, I2C_MEMADD_SIZE_8BIT, &val, 1, KT0803L_TIMEOUT_MS);
}

HAL_StatusTypeDef KT0803L::_KT0803L_HW_ReadReg(uint8_t addr, uint8_t *pVal)
{
    return HAL_I2C_Mem_Read(i2cHandle, KT0803L_I2C_ADDR_READ, addr, I2C_MEMADD_SIZE_8BIT, pVal, 1, KT0803L_TIMEOUT_MS);
}

void KT0803L::KT0803L_WR_Reg(uint8_t addr, uint8_t val)
{
    if (_KT0803L_HW_WriteReg(addr, val) == HAL_OK)
    {
        if (addr < (sizeof(KT0803L_Regs) / sizeof(uint8_t)))
        {
            KT0803L_Regs[addr] = val;
        }
    }
}

uint8_t KT0803L::KT0803L_RD_Reg(uint8_t addr)
{
    uint8_t val = 0;
    if (_KT0803L_HW_ReadReg(addr, &val) == HAL_OK)
    {
        if (addr < (sizeof(KT0803L_Regs) / sizeof(uint8_t)))
        {
            KT0803L_Regs[addr] = val;
        }
    }
    return val;
}

HAL_StatusTypeDef KT0803L::_KT0803L_SyncCache(void)
{
    HAL_StatusTypeDef status = HAL_OK;
    // 同步几个常用的控制寄存器
    uint8_t sync_addrs[] = {KT0803L_R00, KT0803L_R01, KT0803L_R02, KT0803L_R04, KT0803L_R0B, KT0803L_R13};
    for (uint8_t i = 0; i < sizeof(sync_addrs); i++)
    {
        status = _KT0803L_HW_ReadReg(sync_addrs[i], &KT0803L_Regs[sync_addrs[i]]);
        if (status != HAL_OK) return status;
    }
    return status;
}

HAL_StatusTypeDef KT0803L::KT0803L_Init(void)
{
    // KT0803L没有明确的Chip ID寄存器，我们可以尝试读取状态寄存器0x0F来确认I2C通信
    uint8_t status_reg = 0;
    if (_KT0803L_HW_ReadReg(KT0803L_R0F, &status_reg) != HAL_OK)
    {
        printf("KT0803L I2C Connection Failed!\r\n");
        return HAL_ERROR;
    }

    // 唤醒芯片，清除Standby和PA Power Down
    KT0803L_Standby_Set(false);

    // 初始化缓存
    if (_KT0803L_SyncCache() != HAL_OK)
    {
        return HAL_ERROR;
    }

    printf("KT0803L Init Success.\r\n");
    return HAL_OK;
}

void KT0803L::KT0803L_Standby_Set(bool standby)
{
    uint8_t temp = KT0803L_RD_Reg(KT0803L_R0B);
    if (standby)
    {
        temp |= KT0803L_R0B_STANDBY; // Standby enable
        temp |= KT0803L_R0B_PDPA;    // Power Amplifier power down
    }
    else
    {
        temp &= ~KT0803L_R0B_STANDBY;
        temp &= ~KT0803L_R0B_PDPA;
    }
    KT0803L_WR_Reg(KT0803L_R0B, temp);
}

void KT0803L::KT0803L_Mute_Set(bool mute)
{
    uint8_t temp = KT0803L_RD_Reg(KT0803L_R02);
    if (mute) temp |= KT0803L_R02_MUTE;
    else      temp &= ~KT0803L_R02_MUTE;
    KT0803L_WR_Reg(KT0803L_R02, temp);
}

// 设置发射频率，单位MHz (例如 89.7)
void KT0803L::KT0803L_Freq_Set(float freq_MHz)
{
    // CHSEL[11:0] = 目标频率(MHz) * 20
    uint16_t chsel = (uint16_t)(freq_MHz * 20.0f);

    // CHSEL拆分到了3个不同的寄存器中:
    // Reg 0x00[7:0] = CHSEL[8:1]
    uint8_t r00 = (chsel >> 1) & 0xFF;
    KT0803L_WR_Reg(KT0803L_R00, r00);

    // Reg 0x01[2:0] = CHSEL[11:9]
    uint8_t r01 = KT0803L_RD_Reg(KT0803L_R01);
    r01 &= 0xF8; // 清除低3位
    r01 |= (chsel >> 9) & 0x07;
    KT0803L_WR_Reg(KT0803L_R01, r01);

    // Reg 0x02[7] = CHSEL[0]
    uint8_t r02 = KT0803L_RD_Reg(KT0803L_R02);
    r02 &= 0x7F; // 清除最高位
    r02 |= (chsel & 0x01) << 7;
    KT0803L_WR_Reg(KT0803L_R02, r02);
}

// 设置射频发射功率，参数范围 0 ~ 15 (对应 RFGAIN[3:0])
void KT0803L::KT0803L_TxPwr_Set(uint8_t pwr_level)
{
    if (pwr_level > 15) pwr_level = 15; // 限制在4位范围内

    // 发射功率位分布在3个寄存器:
    // Reg0x01[7:6] = pwr_level[1:0]
    uint8_t r01 = KT0803L_RD_Reg(KT0803L_R01) & 0x3F; // 清除高2位
    r01 |= (pwr_level & 0x03) << 6;
    KT0803L_WR_Reg(KT0803L_R01, r01);

    // Reg0x02[6] = pwr_level[3]
    uint8_t r02 = KT0803L_RD_Reg(KT0803L_R02) & 0xBF; // 清除第6位
    r02 |= ((pwr_level >> 3) & 0x01) << 6;
    KT0803L_WR_Reg(KT0803L_R02, r02);

    // Reg0x13[7] = pwr_level[2]
    uint8_t r13 = KT0803L_RD_Reg(KT0803L_R13) & 0x7F; // 清除最高位
    r13 |= ((pwr_level >> 2) & 0x01) << 7;
    KT0803L_WR_Reg(KT0803L_R13, r13);
}

// 预加重时间常数设置: 0 = 75us, 1 = 50us
void KT0803L::KT0803L_PreEmphasis_Set(uint8_t val)
{
    uint8_t temp = KT0803L_RD_Reg(KT0803L_R02);
    if(val) temp |= KT0803L_R02_PHTCNST;
    else    temp &= ~KT0803L_R02_PHTCNST;
    KT0803L_WR_Reg(KT0803L_R02, temp);
}

bool KT0803L::KT0803L_IsPowerOK(void)
{
    uint8_t temp = KT0803L_RD_Reg(KT0803L_R0F);
    return (temp & KT0803L_R0F_PW_OK) != 0;
}
