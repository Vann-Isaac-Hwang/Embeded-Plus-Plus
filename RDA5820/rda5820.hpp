#ifndef __RDA5820_H
#define __RDA5820_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h" 


#define RDA5820_TIMEOUT_MS      100
#define RDA5820_I2C_ADDR_WRITE  0x22
#define RDA5820_I2C_ADDR_READ   0x23

// --- �Ĵ�����ַ (Register Addresses) ---
#define RDA5820_R00		0X00  // Chip ID Register (RO), Ӧ�ö��� 0x5820
#define RDA5820_R02		0X02  // Control and Status Register 1
#define RDA5820_R03		0X03  // Channel and Tuning Register
#define RDA5820_R04		0X04  // Control and Status Register 2
#define RDA5820_R05		0X05  // RSSI, LNA and Volume Register
#define RDA5820_R0A		0X0A  // Status Register 1 (RO)
#define RDA5820_R0B		0X0B  // Status Register 2 (RO)
#define RDA5820_R40		0X40  // Mode Control Register
#define RDA5820_R41		0X41  // Memory Control Register
#define RDA5820_R42		0X42  // TX Power Control Register
#define RDA5820_R4A		0X4A  // Memory Interrupt Mode Register
#define RDA5820_R4B		0X4B  // Channel Count Register (RO, auto seek)
#define RDA5820_R4C		0X4C  // TX Audio Deviation Register
#define RDA5820_R4D		0X4D  // TX Pilot Deviation Register
#define RDA5820_R4E		0X4E  // TX RDS Deviation Register
#define RDA5820_R53		0X53  // User-defined Band Bottom Register
#define RDA5820_R54		0X54  // User-defined Band Top Register
#define RDA5820_R64		0X64  // RDS RX Mode Register


// Register 0x02
#define RDA5820_R02_DHIZ        (1 << 15) // High-Z anable
#define RDA5820_R02_DMUTE       (1 << 14) // Mute anable
#define RDA5820_R02_MONO        (1 << 13) // Force mono
#define RDA5820_R02_BASS        (1 << 12) // Bass boost
#define RDA5820_R02_SEEKUP      (1 << 9)  // Seek direction: 1=up, 0=down
#define RDA5820_R02_SEEK        (1 << 8)  // Start seek
#define RDA5820_R02_SOFTRESET   (1 << 1)  // Soft reset
#define RDA5820_R02_ENABLE      (1 << 0)  // Power up enable

// Register 0x03
#define RDA5820_R03_TUNE        (1 << 4)  // Tune enable

// Register 0x0A (Read Only)
#define RDA5820_R0A_STC         (1 << 14) // Seek/Tune complete flag
#define RDA5820_R0A_SF          (1 << 13) // Seek fail flag
#define RDA5820_R0A_ST          (1 << 10) // Stereo indicator: 1=Stereo

// Register 0x0B (Read Only)
#define RDA5820_R0B_FM_TRUE     (1 << 8)  // FM Station indicator
#define RDA5820_R0B_FM_READY    (1 << 7)  // FM Ready flag



class RDA5820
{
private:
    I2C_HandleTypeDef* i2cHandle; 

    HAL_StatusTypeDef _RDA5820_HW_WriteReg(uint8_t addr, uint16_t val);
    HAL_StatusTypeDef _RDA5820_HW_ReadReg(uint8_t addr, uint16_t *pVal);
    HAL_StatusTypeDef _RDA5820_SyncCache(void);
    void RDA5820_WR_Reg(uint8_t addr, uint16_t val);
    uint16_t RDA5820_RD_Reg(uint8_t addr);
public:
    RDA5820(I2C_HandleTypeDef* i2c);
    ~RDA5820();
    HAL_StatusTypeDef RDA5820_Init(void);
    void RDA5820_RX_Mode(void);
    void RDA5820_TX_Mode(void);
    uint8_t RDA5820_Rssi_Get(void);
    void RDA5820_Mute_Set(uint8_t mute);
    void RDA5820_Rssi_Set(uint8_t rssi);
    void RDA5820_Vol_Set(uint8_t vol);
    void RDA5820_TxPAG_Set(uint8_t gain);
    void RDA5820_TxPGA_Set(uint8_t gain);
    void RDA5820_Band_Set(uint8_t band);
    void RDA5820_Space_Set(uint8_t spc);
    void RDA5820_Freq_Set(uint16_t freq);
};




#ifdef __cplusplus
}
#endif

#endif /* __RDA5820_H */
