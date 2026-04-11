/*
 * qn8027.hpp
 *
 *  Created on: Apr 10, 2026
 *      Author: huang
 */

#ifndef __QN8027_H
#define __QN8027_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#define QN8027_TIMEOUT_MS      100

// QN8027 I2C 8位读写地址 (7位地址 0x2C -> 0101100)
#define QN8027_I2C_ADDR_WRITE  0x58
#define QN8027_I2C_ADDR_READ   0x58

// --- 寄存器地址 (Register Addresses) ---
#define QN8027_R00_SYSTEM      0x00  // SWRST, RECAL, TXREQ, MONO, MUTE, RDSRDY, CH[9:8]
#define QN8027_R01_CH1         0x01  // CH[7:0]
#define QN8027_R02_GPLT        0x02  // TC, priv_en, t1m_sel[1:0], GAIN_TXPLT[3:0]
#define QN8027_R03_REG_XTL     0x03  // XINJ[1:0], XISEL[5:0]
#define QN8027_R04_REG_VGA     0x04  // XSEL, GVGA[2:0], GDB[1:0], RIN[1:0]
#define QN8027_R05_CID1        0x05  // Device ID 1 (RO)
#define QN8027_R06_CID2        0x06  // Device ID 2 (RO)
#define QN8027_R07_STATUS      0x07  // aud_pk[3:0], RDS_UPD, FSM[2:0] (RO)
#define QN8027_R10_PAC         0x10  // TXPD_CLR, PA_TRGT[6:0]
#define QN8027_R11_FDEV        0x11  // TX_FDEV[7:0]
#define QN8027_R12_RDS         0x12  // RDSEN, RDSFDEV[6:0]

// --- R00_SYSTEM 寄存器位掩码 ---
#define QN8027_R00_SWRST       (1 << 7) // 软件复位
#define QN8027_R00_RECAL       (1 << 6) // 重新校准
#define QN8027_R00_TXREQ       (1 << 5) // 发射模式请求 (1=TX, 0=IDLE)
#define QN8027_R00_MONO        (1 << 4) // 强制单声道
#define QN8027_R00_MUTE        (1 << 3) // 静音

// --- R04_REG_VGA 寄存器位掩码 ---
#define QN8027_R04_XSEL_24MHZ  (1 << 7) // 1=24MHz, 0=12MHz

class QN8027
{
private:
    I2C_HandleTypeDef* i2cHandle;

public:
    HAL_StatusTypeDef _QN8027_HW_WriteReg(uint8_t addr, uint8_t val);
    HAL_StatusTypeDef _QN8027_HW_ReadReg(uint8_t addr, uint8_t *pVal);

    void QN8027_WR_Reg(uint8_t addr, uint8_t val);
    uint8_t QN8027_RD_Reg(uint8_t addr);

    QN8027(I2C_HandleTypeDef* i2c);
    ~QN8027();

    HAL_StatusTypeDef QN8027_Init(void);

    void QN8027_TX_Enable(bool enable);
    void QN8027_Mute_Set(bool mute);
    void QN8027_Freq_Set(float freq_MHz);
    void QN8027_TxPwr_Set(uint8_t pa_trgt);
    void QN8027_PreEmphasis_Set(uint8_t val);

    uint8_t QN8027_GetState(void);
};

#ifdef __cplusplus
}
#endif

#endif /* __QN8027_H */
