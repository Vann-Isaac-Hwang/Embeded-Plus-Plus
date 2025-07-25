#include "rda5820.hpp"
#include <stdio.h>



static uint16_t RDA5820_Regs[0x43];


RDA5820::RDA5820(I2C_HandleTypeDef* i2c) : i2cHandle(i2c)
{
    for (size_t i = 0; i < sizeof(RDA5820_Regs) / sizeof(RDA5820_Regs[0]); i++)
    {
        RDA5820_Regs[i] = 0;
    }
}
RDA5820::~RDA5820(){

}

void RDA5820::RDA5820_WR_Reg(uint8_t addr, uint16_t val)
{
    if (_RDA5820_HW_WriteReg(addr, val) == HAL_OK)
    {
        if (addr < (sizeof(RDA5820_Regs) / sizeof(uint16_t)))
        {
            RDA5820_Regs[addr] = val;
        }
    }
}
uint16_t RDA5820::RDA5820_RD_Reg(uint8_t addr)
{
    uint16_t val = 0;
    _RDA5820_HW_ReadReg(addr, &val);
    return val;
}

HAL_StatusTypeDef RDA5820::_RDA5820_HW_WriteReg(uint8_t addr, uint16_t val)
{
    uint8_t data_to_send[2];
    data_to_send[0] = (uint8_t)(val >> 8);   
    data_to_send[1] = (uint8_t)(val & 0xFF);

    return HAL_I2C_Mem_Write(i2cHandle, RDA5820_I2C_ADDR_WRITE, addr, I2C_MEMADD_SIZE_8BIT, data_to_send, 2, RDA5820_TIMEOUT_MS);
}

HAL_StatusTypeDef RDA5820::_RDA5820_SyncCache(void)
{
    HAL_StatusTypeDef status = HAL_OK;
    for (uint8_t addr = 0x02; addr <= 0x07; addr++) 
    {
        status = _RDA5820_HW_ReadReg(addr, &RDA5820_Regs[addr]);
        if (status != HAL_OK) return status;
    }
    status = _RDA5820_HW_ReadReg(RDA5820_R40, &RDA5820_Regs[RDA5820_R40]);
    if (status != HAL_OK) return status;

    status = _RDA5820_HW_ReadReg(RDA5820_R42, &RDA5820_Regs[RDA5820_R42]);
    return status;
}



HAL_StatusTypeDef RDA5820::_RDA5820_HW_ReadReg(uint8_t addr, uint16_t *pVal)
{
    uint8_t received_data[2];
    HAL_StatusTypeDef status;
    
    status = HAL_I2C_Mem_Read(i2cHandle, RDA5820_I2C_ADDR_READ, addr, I2C_MEMADD_SIZE_8BIT, received_data, 2, RDA5820_TIMEOUT_MS);

    if (status == HAL_OK)
    {
        *pVal = ((uint16_t)received_data[0] << 8) | received_data[1];
    }
    return status;
}

HAL_StatusTypeDef RDA5820::RDA5820_Init(void)
{
    uint16_t chip_id;

    if (_RDA5820_HW_ReadReg(RDA5820_R00, &chip_id) != HAL_OK)
    {
        return HAL_ERROR; 
    }
    
    printf("RDA5820 Chip ID: 0x%04X\r\n", chip_id);

    if (chip_id == 0x5820 || chip_id == 0x5805)
    {
        RDA5820_WR_Reg(RDA5820_R02, RDA5820_R02_SOFTRESET);
        HAL_Delay(50);
        uint16_t r02_val = RDA5820_R02_ENABLE; 
        RDA5820_WR_Reg(RDA5820_R02, r02_val);
        HAL_Delay(600);

        if (_RDA5820_SyncCache() != HAL_OK)
        {
            return HAL_ERROR;
        }

        return HAL_OK;
    }

    return HAL_ERROR; 
}

void RDA5820::RDA5820_RX_Mode(void)
{
    uint16_t temp = RDA5820_Regs[RDA5820_R40];
    temp &= 0xFFF0;
    RDA5820_WR_Reg(RDA5820_R40, temp);
}

void RDA5820::RDA5820_TX_Mode(void)
{
    uint16_t temp = RDA5820_Regs[RDA5820_R40];
    temp &= 0xFFF0;
    temp |= 0x0001; 
    RDA5820_WR_Reg(RDA5820_R40, temp);
}

void RDA5820::RDA5820_Vol_Set(uint8_t vol)
{
    uint16_t temp = RDA5820_Regs[RDA5820_R05];
    temp &= 0xFFF0;
    temp |= (vol & 0x0F);
    RDA5820_WR_Reg(RDA5820_R05, temp);
}

void RDA5820::RDA5820_Mute_Set(uint8_t mute)
{
    uint16_t temp = RDA5820_Regs[RDA5820_R02];
    if (mute)
    {
        temp |= RDA5820_R02_DMUTE;
    }
    else
    {
        temp &= ~RDA5820_R02_DMUTE;
    }
    RDA5820_WR_Reg(RDA5820_R02, temp);
}

void RDA5820::RDA5820_Freq_Set(uint16_t freq)
{
    uint16_t temp;
    uint8_t spc_val = 0, band = 0;
    uint16_t f_bottom, chan;
    
    temp = RDA5820_Regs[RDA5820_R03];
    band = (temp >> 2) & 0x03;
    spc_val = temp & 0x03;

    uint8_t spacing_khz;
    if(spc_val == 0) spacing_khz = 10;     
    else if(spc_val == 1) spacing_khz = 20;
    else spacing_khz = 5;                

    if (band == 0) f_bottom = 8700; 
    else if (band == 1 || band == 2) f_bottom = 7600;
    else
    {
        f_bottom = RDA5820_RD_Reg(RDA5820_R53) * 10;
    }

    if (freq < f_bottom) return;

    chan = (freq - f_bottom) / spacing_khz;
    chan &= 0x03FF; 

    temp = RDA5820_Regs[RDA5820_R03]; 
    temp &= 0x003F; 
    temp |= (chan << 6);
    temp |= RDA5820_R03_TUNE; 
    
    RDA5820_WR_Reg(RDA5820_R03, temp);

    while ((RDA5820_RD_Reg(RDA5820_R0B) & RDA5820_R0B_FM_READY) == 0)
    {
        HAL_Delay(5);
    }
}

uint8_t RDA5820::RDA5820_Rssi_Get(void)
{
	uint16_t temp;
	temp=RDA5820_RD_Reg(0X0B);	
	return temp>>9;              
}
