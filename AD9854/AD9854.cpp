#include "AD9854.hpp" // Include the class header file
#include "main.h"     // For HAL_Delay and other HAL specific definitions

// Constructor implementation
AD9854::AD9854(const AD9854PinConfig& config)
    : _RD_Port(config.RD_Port), _RD_Pin(config.RD_Pin),
      _WR_Port(config.WR_Port), _WR_Pin(config.WR_Pin),
      _UDCLK_Port(config.UDCLK_Port), _UDCLK_Pin(config.UDCLK_Pin),
      _RST_Port(config.RST_Port), _RST_Pin(config.RST_Pin),
      _SP_Port(config.SP_Port), _SP_Pin(config.SP_Pin),
      _OSK_Port(config.OSK_Port), _OSK_Pin(config.OSK_Pin),
      _FDATA_Port(config.FDATA_Port), _FDATA_Pin(config.FDATA_Pin),
      _IO_RST_Port(config.IO_RST_Port), _IO_RST_Pin(config.IO_RST_Pin),
      _SDO_Port(config.SDO_Port), _SDO_Pin(config.SDO_Pin),
      _SDI_Port(config.SDI_Port), _SDI_Pin(config.SDI_Pin) {
    // No specific initialization needed here, as HAL_GPIO_Init is usually done once globally
    // for all GPIOs in the main application setup.
    // The AD9854_Init() method will handle the chip-specific reset and configuration.
}

// Private helper function to write a byte to AD9854
void AD9854::WriteByte(u8 data) {
    for (int i = 8; i > 0; i--) {
        if (data & 0x80) {
            HAL_GPIO_SET(_SDI_Port, _SDI_Pin);
        } else {
            HAL_GPIO_CLR(_SDI_Port, _SDI_Pin);
        }
        data <<= 1;
        HAL_GPIO_CLR(_WR_Port, _WR_Pin);
        HAL_GPIO_SET(_WR_Port, _WR_Pin);
    }
}

// Private helper function to convert frequency (u32)
void AD9854::Freq_convert(u32 freq) {
    u32 freqBuf;
    u32 temp = FREQ_MULT_ULONG;

    u8 array_Freq[4]; // Array to store frequency bytes
    array_Freq[0] = (u8)freq;
    array_Freq[1] = (u8)(freq >> 8);
    array_Freq[2] = (u8)(freq >> 16);
    array_Freq[3] = (u8)(freq >> 24);

    freqBuf = temp * array_Freq[0];
    _freqWord[0] = freqBuf;
    freqBuf >>= 8;

    freqBuf += (temp * array_Freq[1]);
    _freqWord[1] = freqBuf;
    freqBuf >>= 8;

    freqBuf += (temp * array_Freq[2]);
    _freqWord[2] = freqBuf;
    freqBuf >>= 8;

    freqBuf += (temp * array_Freq[3]);
    _freqWord[3] = freqBuf;
    _freqWord[4] = freqBuf >> 8;
    _freqWord[5] = freqBuf >> 16; // Fix: The original C code only shifted 8 bits, but a 48-bit word requires 6 bytes.
                                  // This assumes freqBuf could potentially carry into the 6th byte.
                                  // Given Freq_mult_ulong is u32, this might need re-evaluation if actual frequency
                                  // calculation requires a full 48-bit intermediate result.
                                  // For simplicity based on original code, extending to 6 bytes.
}

// Private helper function to convert frequency (double)
void AD9854::Freq_double_convert(double freq) {
    u32 low32;
    u16 high16;
    double temp = FREQ_MULT_DOUBLE;

    freq *= temp;

    // Calculate High 16 bits and Low 32 bits for a 48-bit frequency word
    // Note: 2^32 is 4294967296, not 4294967295. Using (double)(1ULL << 32) for accuracy.
    const double power_of_2_32 = 4294967296.0;

    high16 = static_cast<u16>(freq / power_of_2_32);
    freq -= static_cast<double>(high16) * power_of_2_32;
    low32 = static_cast<u32>(freq);

    _freqWord[0] = low32;
    _freqWord[1] = low32 >> 8;
    _freqWord[2] = low32 >> 16;
    _freqWord[3] = low32 >> 24;
    _freqWord[4] = high16;
    _freqWord[5] = high16 >> 8;
}

// Initialization function for AD9854
void AD9854::Init() {
    HAL_GPIO_CLR(_SP_Port, _SP_Pin);
    HAL_GPIO_CLR(_WR_Port, _WR_Pin);
    HAL_GPIO_CLR(_UDCLK_Port, _UDCLK_Pin);
    HAL_GPIO_SET(_RST_Port, _RST_Pin); // Reset AD9854
    HAL_Delay(1);
    HAL_GPIO_CLR(_RST_Port, _RST_Pin);
    HAL_GPIO_CLR(_IO_RST_Port, _IO_RST_Pin); // Assuming IO_RST is specific to SPI
    HAL_GPIO_CLR(_RD_Port, _RD_Pin);

    WriteByte(CONTR);
    // WriteByte(0x10); // Original: close comparator
    WriteByte(0x00); // Current: open comparator

    WriteByte(CLK_SET); // Set system clock frequency division
    WriteByte(0x00);    // Set system to mode 0, external control
    WriteByte(0x60);    // Original comment: 锟斤拷锟斤拷为锟缴碉拷锟节凤拷锟饺ｏ拷取锟斤拷锟斤拷值锟斤拷锟斤拷 (likely related to Sinc filter or other control bits)

    HAL_GPIO_SET(_UDCLK_Port, _UDCLK_Pin); // Update AD9854 registers
    HAL_GPIO_CLR(_UDCLK_Port, _UDCLK_Pin);
}

void AD9854::Init_GPIOs(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // 使能所有涉及的GPIO端口时钟（PORTC, PORTE, PORTF）
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();

    /*------------ 初始化 PORTE 引脚（PE0, PE1, PE2, PE4, PE5, PE6）------------*/
    GPIO_InitStruct.Pin = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 |
                          GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;   // 推挽输出模式
    GPIO_InitStruct.Pull = GPIO_NOPULL;           // 无上下拉
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH; // 高速度（根据需求调整）
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /*------------ 初始化 PORTC 引脚（PC13）------------*/
    GPIO_InitStruct.Pin = GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;   // 推挽输出
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH; // 中速（常用LED控制）
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

    // 可选：设置初始电平（例如默认全低电平）
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 |
            GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOG, GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15, GPIO_PIN_RESET);
}

// Set sine wave output (u32 frequency)
void AD9854::SetSine(u32 freq, u16 shape) {
    Freq_convert(freq); // Convert frequency

    // Write 6-byte frequency word
    // Note: Original C code iterates count from 6 down to 1, writing _freqWord[--count]
    // This effectively writes bytes from _freqWord[5] down to _freqWord[0]
    // with FREQ1 as the starting address.
    WriteByte(FREQ1); // Send address first
    for (int count = 5; count >= 0; count--) {
        WriteByte(_freqWord[count]);
    }

    WriteByte(SHAPEI); // Set I channel shape
    WriteByte(shape >> 8);
    WriteByte(static_cast<u8>(shape & 0xff));

    WriteByte(SHAPEQ); // Set Q channel shape
    WriteByte(shape >> 8);
    WriteByte(static_cast<u8>(shape & 0xff));

    HAL_GPIO_SET(_UDCLK_Port, _UDCLK_Pin); // Update AD9854 registers
    HAL_GPIO_CLR(_UDCLK_Port, _UDCLK_Pin);
}

// Set sine wave output (double frequency)
void AD9854::SetSine_double(double freq, u16 shape) {
    Freq_double_convert(freq); // Convert frequency

    WriteByte(FREQ1); // Send address first
    for (int count = 5; count >= 0; count--) {
        WriteByte(_freqWord[count]);
    }

    WriteByte(SHAPEI); // Set I channel shape
    WriteByte(shape >> 8);
    WriteByte(static_cast<u8>(shape & 0xff));

    WriteByte(SHAPEQ); // Set Q channel shape
    WriteByte(shape >> 8);
    WriteByte(static_cast<u8>(shape & 0xff));

    HAL_GPIO_SET(_UDCLK_Port, _UDCLK_Pin); // Update AD9854 registers
    HAL_GPIO_CLR(_UDCLK_Port, _UDCLK_Pin);
}

// FSK initialization
void AD9854::InitFSK() {
    HAL_GPIO_CLR(_SP_Port, _SP_Pin);
    HAL_GPIO_CLR(_WR_Port, _WR_Pin);
    HAL_GPIO_CLR(_UDCLK_Port, _UDCLK_Pin);
    HAL_GPIO_SET(_RST_Port, _RST_Pin); // Reset AD9854
    HAL_Delay(1);
    HAL_GPIO_CLR(_RST_Port, _RST_Pin);
    HAL_GPIO_CLR(_IO_RST_Port, _IO_RST_Pin);
    HAL_GPIO_CLR(_RD_Port, _RD_Pin);

    WriteByte(CONTR);
    WriteByte(0x10); // Close comparator
    WriteByte(CLK_SET); // Set system clock frequency division
    WriteByte(0x02); // Set system for FSK mode (0x02 for FSK)
    WriteByte(0x60);

    HAL_GPIO_SET(_UDCLK_Port, _UDCLK_Pin);
    HAL_GPIO_CLR(_UDCLK_Port, _UDCLK_Pin);
}

// Set FSK frequencies
void AD9854::SetFSK(u32 freq1, u32 freq2) {
    const u16 shape = 4000;

    Freq_convert(freq1);
    WriteByte(FREQ1);
    for (int count = 5; count >= 0; count--) {
        WriteByte(_freqWord[count]);
    }

    Freq_convert(freq2);
    WriteByte(FREQ2);
    for (int count = 5; count >= 0; count--) {
        WriteByte(_freqWord[count]);
    }

    WriteByte(SHAPEI);
    WriteByte(shape >> 8);
    WriteByte(static_cast<u8>(shape & 0xff));

    WriteByte(SHAPEQ);
    WriteByte(shape >> 8);
    WriteByte(static_cast<u8>(shape & 0xff));

    HAL_GPIO_SET(_UDCLK_Port, _UDCLK_Pin);
    HAL_GPIO_CLR(_UDCLK_Port, _UDCLK_Pin);
}

// BPSK initialization
void AD9854::InitBPSK() {
    HAL_GPIO_CLR(_SP_Port, _SP_Pin);
    HAL_GPIO_CLR(_WR_Port, _WR_Pin);
    HAL_GPIO_CLR(_UDCLK_Port, _UDCLK_Pin);
    HAL_GPIO_SET(_RST_Port, _RST_Pin); // Reset AD9854
    HAL_Delay(1);
    HAL_GPIO_CLR(_RST_Port, _RST_Pin);
    HAL_GPIO_CLR(_IO_RST_Port, _IO_RST_Pin);
    HAL_GPIO_CLR(_RD_Port, _RD_Pin);

    WriteByte(CONTR);
    WriteByte(0x10); // Close comparator
    WriteByte(CLK_SET); // Set system clock frequency division
    WriteByte(0x08); // Set system for BPSK mode (0x08 for BPSK)
    WriteByte(0x60);

    HAL_GPIO_SET(_UDCLK_Port, _UDCLK_Pin);
    HAL_GPIO_CLR(_UDCLK_Port, _UDCLK_Pin);
}

// Set BPSK phases
void AD9854::SetBPSK(u16 phase1, u16 phase2) {
    const u32 freq = 60000;
    const u16 shape = 4000;

    WriteByte(PHASE1);
    WriteByte(phase1 >> 8);
    WriteByte(static_cast<u8>(phase1 & 0xff));

    WriteByte(PHASE2);
    WriteByte(phase2 >> 8);
    WriteByte(static_cast<u8>(phase2 & 0xff));

    Freq_convert(freq);
    WriteByte(FREQ1);
    for (int count = 5; count >= 0; count--) {
        WriteByte(_freqWord[count]);
    }

    WriteByte(SHAPEI);
    WriteByte(shape >> 8);
    WriteByte(static_cast<u8>(shape & 0xff));

    WriteByte(SHAPEQ);
    WriteByte(shape >> 8);
    WriteByte(static_cast<u8>(shape & 0xff));

    HAL_GPIO_SET(_UDCLK_Port, _UDCLK_Pin);
    HAL_GPIO_CLR(_UDCLK_Port, _UDCLK_Pin);
}

// OSK initialization
void AD9854::InitOSK() {
    HAL_GPIO_CLR(_SP_Port, _SP_Pin);
    HAL_GPIO_CLR(_WR_Port, _WR_Pin);
    HAL_GPIO_CLR(_UDCLK_Port, _UDCLK_Pin);
    HAL_GPIO_SET(_RST_Port, _RST_Pin); // Reset AD9854
    HAL_Delay(1);
    HAL_GPIO_CLR(_RST_Port, _RST_Pin);
    HAL_GPIO_CLR(_IO_RST_Port, _IO_RST_Pin);
    HAL_GPIO_CLR(_RD_Port, _RD_Pin);

    WriteByte(CONTR);
    WriteByte(0x10); // Close comparator
    WriteByte(CLK_SET); // Set system clock frequency division
    WriteByte(0x00); // Set system to mode 0, external control
    WriteByte(0x70); // Original comment: 锟斤拷锟斤拷为锟缴碉拷锟节凤拷锟饺ｏ拷取锟斤拷锟斤拷值锟斤拷锟斤拷,通锟斤拷锟斤拷锟斤拷锟节诧拷锟斤拷锟斤拷 (OSK enable bit set here)

    HAL_GPIO_SET(_UDCLK_Port, _UDCLK_Pin);
    HAL_GPIO_CLR(_UDCLK_Port, _UDCLK_Pin);
}

// Set OSK ramp rate and shape
void AD9854::SetOSK(u8 rateShape) {
    const u32 freq = 60000;
    const u16 shape = 4000;

    Freq_convert(freq);
    WriteByte(FREQ1);
    for (int count = 5; count >= 0; count--) {
        WriteByte(_freqWord[count]);
    }

    WriteByte(SHAPEI);
    WriteByte(shape >> 8);
    WriteByte(static_cast<u8>(shape & 0xff));

    WriteByte(SHAPEQ);
    WriteByte(shape >> 8);
    WriteByte(static_cast<u8>(shape & 0xff));

    WriteByte(RAMPO); // Set OSK ramp rate
    WriteByte(rateShape);

    HAL_GPIO_SET(_UDCLK_Port, _UDCLK_Pin);
    HAL_GPIO_CLR(_UDCLK_Port, _UDCLK_Pin);
}

// AM initialization
void AD9854::InitAM() {
    const u32 freq = 60000;

    HAL_GPIO_CLR(_SP_Port, _SP_Pin);
    HAL_GPIO_CLR(_WR_Port, _WR_Pin);
    HAL_GPIO_CLR(_UDCLK_Port, _UDCLK_Pin);
    HAL_GPIO_SET(_RST_Port, _RST_Pin); // Reset AD9854
    HAL_Delay(1);
    HAL_GPIO_CLR(_RST_Port, _RST_Pin);
    HAL_GPIO_CLR(_IO_RST_Port, _IO_RST_Pin);
    HAL_GPIO_CLR(_RD_Port, _RD_Pin);

    WriteByte(CONTR);
    WriteByte(0x10); // Close comparator
    WriteByte(CLK_SET); // Set system clock frequency division
    WriteByte(0x00); // Set system to mode 0, external control
    WriteByte(0x60);

    Freq_convert(freq);
    WriteByte(FREQ1);
    for (int count = 5; count >= 0; count--) {
        WriteByte(_freqWord[count]);
    }

    HAL_GPIO_SET(_UDCLK_Port, _UDCLK_Pin);
    HAL_GPIO_CLR(_UDCLK_Port, _UDCLK_Pin);
}

// Set AM shape
void AD9854::SetAM(u16 shape) {
    WriteByte(SHAPEI);
    WriteByte(shape >> 8);
    WriteByte(static_cast<u8>(shape & 0xff));

    WriteByte(SHAPEQ);
    WriteByte(shape >> 8);
    WriteByte(static_cast<u8>(shape & 0xff));

    HAL_GPIO_SET(_UDCLK_Port, _UDCLK_Pin);
    HAL_GPIO_CLR(_UDCLK_Port, _UDCLK_Pin);
}

// RFSK initialization
void AD9854::InitRFSK() {
    HAL_GPIO_CLR(_SP_Port, _SP_Pin);
    HAL_GPIO_CLR(_WR_Port, _WR_Pin);
    HAL_GPIO_CLR(_UDCLK_Port, _UDCLK_Pin);
    HAL_GPIO_SET(_RST_Port, _RST_Pin); // Reset AD9854
    HAL_Delay(1);
    HAL_GPIO_CLR(_RST_Port, _RST_Pin);
    HAL_GPIO_CLR(_IO_RST_Port, _IO_RST_Pin);
    HAL_GPIO_CLR(_RD_Port, _RD_Pin);
    HAL_GPIO_CLR(_FDATA_Port, _FDATA_Pin); // FSK/BPSK/HOLD

    WriteByte(CONTR);
    WriteByte(0x10); // Close comparator
    WriteByte(CLK_SET); // Set system clock frequency division
    WriteByte(0x24); // Set system to RFSK mode (0x24)
    WriteByte(0x20); // Disable Sinc filter

    HAL_GPIO_SET(_UDCLK_Port, _UDCLK_Pin);
    HAL_GPIO_CLR(_UDCLK_Port, _UDCLK_Pin);
}

// Set RFSK parameters
void AD9854::SetRFSK(u32 freq_Low, u32 freq_High, u32 freq_Up_Down, u32 freRate) {
    const u16 shape = 3600;

    Freq_convert(freq_Low);
    WriteByte(FREQ1);
    for (int count = 5; count >= 0; count--) {
        WriteByte(_freqWord[count]);
    }

    Freq_convert(freq_High);
    WriteByte(FREQ2);
    for (int count = 5; count >= 0; count--) {
        WriteByte(_freqWord[count]);
    }

    Freq_convert(freq_Up_Down);
    WriteByte(DELFQ);
    for (int count = 5; count >= 0; count--) {
        WriteByte(_freqWord[count]);
    }

    WriteByte(RAMPF); // Set ramp rate
    WriteByte(static_cast<u8>((freRate >> 16) & 0x0f));
    WriteByte(static_cast<u8>(freRate >> 8));
    WriteByte(static_cast<u8>(freRate));

    WriteByte(SHAPEI);
    WriteByte(shape >> 8);
    WriteByte(static_cast<u8>(shape & 0xff));

    WriteByte(SHAPEQ);
    WriteByte(shape >> 8);
    WriteByte(static_cast<u8>(shape & 0xff));

    HAL_GPIO_SET(_UDCLK_Port, _UDCLK_Pin);
    HAL_GPIO_CLR(_UDCLK_Port, _UDCLK_Pin);
}
