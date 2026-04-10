/*
 * kt0803l.hpp
 *
 *  Created on: Apr 10, 2026
 *      Author: huang
 */

#ifndef __KT0803L_H
#define __KT0803L_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#define KT0803L_TIMEOUT_MS      100
// KT0803L I2C 8位读写地址 (7位地址为 0x3E)
#define KT0803L_I2C_ADDR_WRITE  0x7C
#define KT0803L_I2C_ADDR_READ   0x7D

// --- 寄存器地址 (Register Addresses) ---
#define KT0803L_R00		0X00  // CHSEL[8:1]
#define KT0803L_R01		0X01  // RFGAIN[1:0], PGA[2:0], CHSEL[11:9]
#define KT0803L_R02		0X02  // CHSEL[0], RFGAIN[3], PLTADJ, MUTE, PHTCNST
#define KT0803L_R04		0X04  // ALC_EN, MONO, PGA_LSB[1:0], BASS[1:0]
#define KT0803L_R0B		0X0B  // Standby, PDPA, AUTO_PADN
#define KT0803L_R0C		0X0C  // ALC_DECAY_TIME[3:0], ALC_ATTACK_TIME[3:0]
#define KT0803L_R0E		0X0E  // PA_BIAS
#define KT0803L_R0F		0X0F  // PW_OK, SLNCID (Read Only)
#define KT0803L_R10		0X10  // PGAMOD
#define KT0803L_R12		0X12  // SLNCDIS, SLNCTHL[2:0], SLNCTHH[2:0], SW_MOD
#define KT0803L_R13		0X13  // RFGAIN[2], PA_CTRL
#define KT0803L_R17		0X17  // FDEV, AU_ENHANCE, XTAL_SEL
#define KT0803L_R1E     0X1E  // DCLK, XTALD, REF_CLK[3:0]

// --- 寄存器位掩码 (Bit Masks) ---
// Register 0x02
#define KT0803L_R02_MUTE        (1 << 3)  // Software Mute
#define KT0803L_R02_PHTCNST     (1 << 0)  // Pre-emphasis: 0=75us, 1=50us

// Register 0x0B
#define KT0803L_R0B_STANDBY     (1 << 7)  // Standby enable
#define KT0803L_R0B_PDPA        (1 << 5)  // Power Amplifier Power Down

// Register 0x0F (Read Only)
#define KT0803L_R0F_PW_OK       (1 << 4)  // Power OK Indicator
#define KT0803L_R0F_SLNCID      (1 << 2)  // Silence detected flag

class KT0803L
{
private:
    I2C_HandleTypeDef* i2cHandle;

public:
    HAL_StatusTypeDef _KT0803L_HW_WriteReg(uint8_t addr, uint8_t val);
    HAL_StatusTypeDef _KT0803L_HW_ReadReg(uint8_t addr, uint8_t *pVal);
    HAL_StatusTypeDef _KT0803L_SyncCache(void);

    void KT0803L_WR_Reg(uint8_t addr, uint8_t val);
    uint8_t KT0803L_RD_Reg(uint8_t addr);

    KT0803L(I2C_HandleTypeDef* i2c);
    ~KT0803L();

    HAL_StatusTypeDef KT0803L_Init(void);

    void KT0803L_Standby_Set(bool standby);
    void KT0803L_Mute_Set(bool mute);
    void KT0803L_Freq_Set(float freq_MHz);
    void KT0803L_TxPwr_Set(uint8_t pwr_level);
    void KT0803L_PGA_Set(uint8_t pga_gain);
    void KT0803L_PreEmphasis_Set(uint8_t val);
    bool KT0803L_IsPowerOK(void);
};

#ifdef __cplusplus
}
#endif

#endif /* __KT0803L_H */
