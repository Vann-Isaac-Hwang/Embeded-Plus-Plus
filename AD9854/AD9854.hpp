#ifndef _AD9854_HPP_
#define _AD9854_HPP_

#include "main.h" // Assuming main.h contains HAL_GPIO_WritePin and other necessary types like GPIO_TypeDef, uint16_t, u8, u32, u16, etc.

// AD9854 Register Addresses
#define PHASE1    0x00    // phase adjust register #1
#define PHASE2    0x01    // phase adjust register #2
#define FREQ1     0x02    // frequency tuning word 1
#define FREQ2     0x03    // frequency tuning word 2
#define DELFQ     0x04    // delta frequency word
#define UPDCK     0x05    // update clock
#define RAMPF     0x06    // ramp rate clock
#define CONTR     0x07    // control register
#define SHAPEI    0x08    // output shape key I mult
#define SHAPEQ    0x09    // output shape key Q mult
#define RAMPO     0x0A    // output shape key ramp rate
#define CDAC      0x0B    // QDAC

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t

// Structure to hold GPIO pin configurations
struct AD9854PinConfig {
    GPIO_TypeDef* RD_Port;
    uint16_t RD_Pin;
    GPIO_TypeDef* WR_Port;
    uint16_t WR_Pin;
    GPIO_TypeDef* UDCLK_Port;
    uint16_t UDCLK_Pin;
    GPIO_TypeDef* RST_Port;
    uint16_t RST_Pin;
    GPIO_TypeDef* SP_Port;
    uint16_t SP_Pin;
    GPIO_TypeDef* OSK_Port;
    uint16_t OSK_Pin;
    GPIO_TypeDef* FDATA_Port;
    uint16_t FDATA_Pin;
    GPIO_TypeDef* IO_RST_Port;
    uint16_t IO_RST_Pin;
    GPIO_TypeDef* SDO_Port;
    uint16_t SDO_Pin;
    GPIO_TypeDef* SDI_Port;
    uint16_t SDI_Pin;
};

class AD9854 {
public:
    // Constructor: Initializes the AD9854 with pin configurations
    AD9854(const AD9854PinConfig& config);

    // Initialization function for AD9854
    void Init();

    void Init_GPIOs(void);

    // Set sine wave output
    void SetSine(u32 freq, u16 shape);
    void SetSine_double(double freq, u16 shape);

    // FSK functions
    void InitFSK();
    void SetFSK(u32 freq1, u32 freq2);

    // BPSK functions
    void InitBPSK();
    void SetBPSK(u16 phase1, u16 phase2);

    // OSK functions
    void InitOSK();
    void SetOSK(u8 rateShape);

    // AM functions
    void InitAM();
    void SetAM(u16 shape);

    // RFSK functions
    void InitRFSK();
    void SetRFSK(u32 freq_Low, u32 freq_High, u32 freq_Up_Down, u32 freRate);

private:
    // Private helper function to write a byte to AD9854
    void WriteByte(u8 data);

    // Private helper function to convert frequency (u32)
    void Freq_convert(u32 freq);

    // Private helper function to convert frequency (double)
    void Freq_double_convert(double freq);

    // Member variables to store GPIO port and pin information
    GPIO_TypeDef* _RD_Port;
    uint16_t _RD_Pin;
    GPIO_TypeDef* _WR_Port;
    uint16_t _WR_Pin;
    GPIO_TypeDef* _UDCLK_Port;
    uint16_t _UDCLK_Pin;
    GPIO_TypeDef* _RST_Port;
    uint16_t _RST_Pin;
    GPIO_TypeDef* _SP_Port;
    uint16_t _SP_Pin;
    GPIO_TypeDef* _OSK_Port;
    uint16_t _OSK_Pin;
    GPIO_TypeDef* _FDATA_Port;
    uint16_t _FDATA_Pin;
    GPIO_TypeDef* _IO_RST_Port;
    uint16_t _IO_RST_Pin;
    GPIO_TypeDef* _SDO_Port;
    uint16_t _SDO_Pin;
    GPIO_TypeDef* _SDI_Port;
    uint16_t _SDI_Pin;

    u8 _freqWord[6]; // Internal buffer for frequency word

    // Constants for frequency conversion (replace with actual values from your main.h or a configuration)
    // These values were 'CLK_Set', 'Freq_mult_ulong', 'Freq_mult_doulle' in the C code.
    // They are made private static const members to be accessible within the class.
    static const u8 CLK_SET = 9;
    static const u32 FREQ_MULT_ULONG = 1042500;
    static constexpr double FREQ_MULT_DOUBLE = 1042499.9137431;

    // Helper macro to simplify HAL_GPIO_WritePin calls within the class
    #define HAL_GPIO_SET(port, pin)     HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET)
    #define HAL_GPIO_CLR(port, pin)     HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET)
};

#endif // _AD9854_HPP_
