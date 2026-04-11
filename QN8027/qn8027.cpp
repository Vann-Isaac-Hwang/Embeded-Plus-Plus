#include "qn8027.hpp"

// 🔴 引入您在 main.cpp 中定义的打印函数
extern int USART_printf(const char *format, ...);

// 缓存QN8027常用的寄存器以减少I2C读取
static uint8_t QN8027_Regs[0x13];

QN8027::QN8027(I2C_HandleTypeDef* i2c) : i2cHandle(i2c)
{
    for (size_t i = 0; i < sizeof(QN8027_Regs) / sizeof(QN8027_Regs[0]); i++)
    {
        QN8027_Regs[i] = 0;
    }
}

QN8027::~QN8027() {}

HAL_StatusTypeDef QN8027::_QN8027_HW_WriteReg(uint8_t addr, uint8_t val)
{
    return HAL_I2C_Mem_Write(i2cHandle, QN8027_I2C_ADDR_WRITE, addr, I2C_MEMADD_SIZE_8BIT, &val, 1, QN8027_TIMEOUT_MS);
}

HAL_StatusTypeDef QN8027::_QN8027_HW_ReadReg(uint8_t addr, uint8_t *pVal)
{
    return HAL_I2C_Mem_Read(i2cHandle, QN8027_I2C_ADDR_READ, addr, I2C_MEMADD_SIZE_8BIT, pVal, 1, QN8027_TIMEOUT_MS);
}

void QN8027::QN8027_WR_Reg(uint8_t addr, uint8_t val)
{
    if (_QN8027_HW_WriteReg(addr, val) == HAL_OK)
    {
        if (addr < sizeof(QN8027_Regs)) QN8027_Regs[addr] = val;
    }
}

uint8_t QN8027::QN8027_RD_Reg(uint8_t addr)
{
    uint8_t val = 0;
    if (_QN8027_HW_ReadReg(addr, &val) == HAL_OK)
    {
        if (addr < sizeof(QN8027_Regs)) QN8027_Regs[addr] = val;
    }
    return val;
}

HAL_StatusTypeDef QN8027::QN8027_Init(void)
{
    uint8_t cid2 = 0;

    // 1. 读取 CID2 (Reg 0x06)，检查是否在线
    if (_QN8027_HW_ReadReg(QN8027_R06_CID2, &cid2) != HAL_OK) {
        USART_printf("QN8027 I2C Comm Failed!\r\n");
        return HAL_ERROR;
    }

    // 2. 软件复位 (写入 0x81 彻底复位)
    QN8027_WR_Reg(0x00, 0x81);
    HAL_Delay(20); // 给芯片内部系统充足的重启时间

    // 3. 晶振配置：12MHz (0x33 是标准 12MHz 配置值)
    QN8027_WR_Reg(0x04, 0x33);

    // =========================================================
    // 🔴 核心魔法步骤：触发内部状态机(FSM)进行频率校准
    // 这是解决 68MHz 问题的绝对关键！
    // =========================================================
    QN8027_WR_Reg(0x00, 0x41); // 置位 RECAL (启动校准)
    QN8027_WR_Reg(0x00, 0x01); // 清除 RECAL
    HAL_Delay(20);             // 必须等待至少 20ms 让芯片完成射频校准！

    // =========================================================
    // 🔴 厂商未公开的“隐藏”寄存器 (参考自 Arduino 库)
    // =========================================================
    QN8027_WR_Reg(0x18, 0xE4); // 改善信噪比 (SNR) 优化底层模拟电路
    QN8027_WR_Reg(0x1B, 0xF0); // 解锁发射功率限制

    // 4. 禁用无音频信号时的自动 PA 关闭功能 (写入 0xB9)
    // 这同时也是告诉芯片：开启发射，并使用 50us 的预加重
    QN8027_WR_Reg(0x02, 0xB9);

    // 5. 同步一下缓存
    QN8027_RD_Reg(0x00);
    QN8027_RD_Reg(0x01);
    QN8027_RD_Reg(0x02);
    QN8027_RD_Reg(0x10);

    USART_printf("QN8027 Init Success (FSM Calibrated!).\r\n");
    return HAL_OK;
}

// ...(下方其他的 Freq_Set / TxPwr_Set 等函数保持不变)
void QN8027::QN8027_Freq_Set(float freq_MHz)
{
    if (freq_MHz < 76.0f) freq_MHz = 76.0f;
    if (freq_MHz > 108.0f) freq_MHz = 108.0f;

    uint16_t chsel = (uint16_t)((freq_MHz - 76.0f) * 20.0f);
    QN8027_WR_Reg(QN8027_R01_CH1, (uint8_t)(chsel & 0xFF));

    uint8_t r00 = QN8027_RD_Reg(QN8027_R00_SYSTEM);
    r00 &= 0xFC;
    r00 |= (uint8_t)((chsel >> 8) & 0x03);
    QN8027_WR_Reg(QN8027_R00_SYSTEM, r00);
}

void QN8027::QN8027_TxPwr_Set(uint8_t pa_trgt)
{
    if (pa_trgt < 20) pa_trgt = 20;
    if (pa_trgt > 75) pa_trgt = 75;

    uint8_t r10 = QN8027_RD_Reg(QN8027_R10_PAC);
    r10 &= 0x80;
    r10 |= (pa_trgt & 0x7F);
    QN8027_WR_Reg(QN8027_R10_PAC, r10);

    uint8_t r00 = QN8027_RD_Reg(QN8027_R00_SYSTEM);
    if (r00 & QN8027_R00_TXREQ) {
        QN8027_TX_Enable(false);
        HAL_Delay(5);
        QN8027_TX_Enable(true);
    }
}

void QN8027::QN8027_TX_Enable(bool enable)
{
    uint8_t r00 = QN8027_RD_Reg(QN8027_R00_SYSTEM);
    if (enable) r00 |= QN8027_R00_TXREQ;
    else        r00 &= ~QN8027_R00_TXREQ;
    QN8027_WR_Reg(QN8027_R00_SYSTEM, r00);
}

void QN8027::QN8027_Mute_Set(bool mute)
{
    uint8_t r00 = QN8027_RD_Reg(QN8027_R00_SYSTEM);
    if (mute) r00 |= QN8027_R00_MUTE;
    else      r00 &= ~QN8027_R00_MUTE;
    QN8027_WR_Reg(QN8027_R00_SYSTEM, r00);
}

void QN8027::QN8027_PreEmphasis_Set(uint8_t val)
{
    uint8_t r02 = QN8027_RD_Reg(QN8027_R02_GPLT);
    if (val) r02 |= (1 << 7);
    else     r02 &= ~(1 << 7);
    QN8027_WR_Reg(QN8027_R02_GPLT, r02);
}

uint8_t QN8027::QN8027_GetState(void)
{
    uint8_t status = QN8027_RD_Reg(QN8027_R07_STATUS);
    return (status & 0x07);
}
