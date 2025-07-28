#ifndef NRF24L01_ENUM_HPP
#define NRF24L01_ENUM_HPP
#include <cstdint> // For uint8_t
// --- NRF24L01 Constant Definitions ---
// These enums categorize NRF24L01 commands, register addresses, and bit flags.
// This approach enhances type safety, readability, and prevents naming conflicts
// with existing HAL library macros.

// 1. NRF24L01 SPI Commands
// Renamed to avoid conflicts with STM32Cube HAL's READ_REG/WRITE_REG macros.
enum class NRF24L01_Command : uint8_t
{
    R_REGISTER = 0x00,  // Read command for register. Actual register address is added to this.
    W_REGISTER = 0x20,  // Write command for register. Actual register address is added to this.
    RD_RX_PLOAD = 0x61, // Read RX payload
    WR_TX_PLOAD = 0xA0, // Write TX payload
    FLUSH_TX = 0xE1,    // Flush TX FIFO
    FLUSH_RX = 0xE2,    // Flush RX FIFO
    REUSE_TX_PL = 0xE3, // Reuse last transmitted payload
    NOP = 0xFF          // No Operation (can be used to read STATUS register)
};

// 2. NRF24L01 Register Addresses
enum class NRF24L01_Register : uint8_t
{
    CONFIG = 0x00,
    EN_AA = 0x01,       // Enable Auto Acknowledgment
    EN_RXADDR = 0x02,   // Enabled RX Addresses
    SETUP_AW = 0x03,    // Setup Address Widths
    SETUP_RETR = 0x04,  // Setup Auto Retransmission
    RF_CH = 0x05,       // RF Channel
    RF_SETUP = 0x06,    // RF Setup Register
    STATUS = 0x07,      // Status Register
    OBSERVE_TX = 0x08,  // Transmit observe register
    CD = 0x09,          // Carrier Detect (nRF24L01+ only)
    RX_ADDR_P0 = 0x0A,  // Receive address data pipe 0
    RX_ADDR_P1 = 0x0B,  // Receive address data pipe 1
    RX_ADDR_P2 = 0x0C,  // Receive address data pipe 2
    RX_ADDR_P3 = 0x0D,  // Receive address data pipe 3
    RX_ADDR_P4 = 0x0E,  // Receive address data pipe 4
    RX_ADDR_P5 = 0x0F,  // Receive address data pipe 5
    TX_ADDR = 0x10,     // Transmit address
    RX_PW_P0 = 0x11,    // Number of bytes in RX payload in data pipe 0
    RX_PW_P1 = 0x12,    // Number of bytes in RX payload in data pipe 1
    RX_PW_P2 = 0x13,    // Number of bytes in RX payload in data pipe 2
    RX_PW_P3 = 0x14,    // Number of bytes in RX payload in data pipe 3
    RX_PW_P4 = 0x15,    // Number of bytes in RX payload in data pipe 4
    RX_PW_P5 = 0x16,    // Number of bytes in RX payload in data pipe 5
    FIFO_STATUS = 0x17, // FIFO Status Register
    DYNPD = 0x1C,       // Enable dynamic payload length
    FEATURE = 0x1D      // Feature Register
};

// 3. CONFIG (Configuration) Register Bit Flags
enum class NRF24L01_ConfigRegBits : uint8_t
{
    // Operation Mode Control
    PRIM_RX = 0x01, // RX/TX control: 0 for TX (Primary Transmitter), 1 for RX (Primary Receiver)
    PWR_UP = 0x02,  // Power up/down: 0 for POWER_DOWN, 1 for PWR_UP

    // CRC Configuration
    CRCO = 0x04,   // CRC encoding scheme: 0 for 8-bit, 1 for 16-bit
    EN_CRC = 0x08, // Enable CRC. Forced high if any of EN_AA is high.

    // Interrupt Masking (setting bit to 1 masks the interrupt)
    MASK_MAX_RT = 0x10, // Mask interrupt caused by MAX_RT (Max number of TX retransmits reached)
    MASK_TX_DS = 0x20,  // Mask interrupt caused by TX_DS (Data Sent)
    MASK_RX_DR = 0x40   // Mask interrupt caused by RX_DR (Data Ready in RX FIFO)
};

// Bitwise operator overloads for NRF24L01_ConfigRegBits
constexpr NRF24L01_ConfigRegBits operator|(NRF24L01_ConfigRegBits a, NRF24L01_ConfigRegBits b)
{
    return static_cast<NRF24L01_ConfigRegBits>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}
constexpr NRF24L01_ConfigRegBits operator&(NRF24L01_ConfigRegBits a, NRF24L01_ConfigRegBits b)
{
    return static_cast<NRF24L01_ConfigRegBits>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}
constexpr NRF24L01_ConfigRegBits operator~(NRF24L01_ConfigRegBits a)
{
    return static_cast<NRF24L01_ConfigRegBits>(~static_cast<uint8_t>(a));
}
constexpr NRF24L01_ConfigRegBits &operator|=(NRF24L01_ConfigRegBits &a, NRF24L01_ConfigRegBits b)
{
    a = a | b;
    return a;
}
constexpr NRF24L01_ConfigRegBits &operator&=(NRF24L01_ConfigRegBits &a, NRF24L01_ConfigRegBits b)
{
    a = a & b;
    return a;
}

// 4. EN_AA (Enable Auto Acknowledgment) Register Bit Flags
enum class NRF24L01_EnAABits : uint8_t
{
    ENAA_P0 = 0x01,        // Enable auto acknowledgment for data pipe 0
    ENAA_P1 = 0x02,        // Enable auto acknowledgment for data pipe 1
    ENAA_P2 = 0x04,        // Enable auto acknowledgment for data pipe 2
    ENAA_P3 = 0x08,        // Enable auto acknowledgment for data pipe 3
    ENAA_P4 = 0x10,        // Enable auto acknowledgment for data pipe 4
    ENAA_P5 = 0x20,        // Enable auto acknowledgment for data pipe 5
    ENAA_DisableALL = 0x00 // Disable auto acknowledgment for all data pipes
};

// Bitwise operator overloads for NRF24L01_EnAABits
constexpr NRF24L01_EnAABits operator|(NRF24L01_EnAABits a, NRF24L01_EnAABits b)
{
    return static_cast<NRF24L01_EnAABits>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}
constexpr NRF24L01_EnAABits operator&(NRF24L01_EnAABits a, NRF24L01_EnAABits b)
{
    return static_cast<NRF24L01_EnAABits>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}
constexpr NRF24L01_EnAABits operator~(NRF24L01_EnAABits a)
{
    return static_cast<NRF24L01_EnAABits>(~static_cast<uint8_t>(a));
}
constexpr NRF24L01_EnAABits &operator|=(NRF24L01_EnAABits &a, NRF24L01_EnAABits b)
{
    a = a | b;
    return a;
}
constexpr NRF24L01_EnAABits &operator&=(NRF24L01_EnAABits &a, NRF24L01_EnAABits b)
{
    a = a & b;
    return a;
}

// 5. EN_RXADDR (Enable RX Addresses) Register Bit Flags
enum class NRF24L01_EnRxAddrBits : uint8_t
{
    ERX_P0 = 0x01,  // Enable data pipe 0
    ERX_P1 = 0x02,  // Enable data pipe 1
    ERX_P2 = 0x04,  // Enable data pipe 2
    ERX_P3 = 0x08,  // Enable data pipe 3
    ERX_P4 = 0x10,  // Enable data pipe 4
    ERX_P5 = 0x20,  // Enable data pipe 5
    ERX_None = 0x00 // Disable all data pipes
};

// Bitwise operator overloads for NRF24L01_EnRxAddrBits
constexpr NRF24L01_EnRxAddrBits operator|(NRF24L01_EnRxAddrBits a, NRF24L01_EnRxAddrBits b)
{
    return static_cast<NRF24L01_EnRxAddrBits>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}
constexpr NRF24L01_EnRxAddrBits operator&(NRF24L01_EnRxAddrBits a, NRF24L01_EnRxAddrBits b)
{
    return static_cast<NRF24L01_EnRxAddrBits>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}
constexpr NRF24L01_EnRxAddrBits operator~(NRF24L01_EnRxAddrBits a)
{
    return static_cast<NRF24L01_EnRxAddrBits>(~static_cast<uint8_t>(a));
}
constexpr NRF24L01_EnRxAddrBits &operator|=(NRF24L01_EnRxAddrBits &a, NRF24L01_EnRxAddrBits b)
{
    a = a | b;
    return a;
}
constexpr NRF24L01_EnRxAddrBits &operator&=(NRF24L01_EnRxAddrBits &a, NRF24L01_EnRxAddrBits b)
{
    a = a & b;
    return a;
}

// 6. SETUP_AW (Address Width Setup) Register Bit Flags
enum class NRF24L01_SetupAWBits : uint8_t
{
    AW_3Bytes = 0x01, // 3 bytes address width (LSB 2 bits: 01)
    AW_4Bytes = 0x02, // 4 bytes address width (LSB 2 bits: 10)
    AW_5Bytes = 0x03  // 5 bytes address width (LSB 2 bits: 11)
};

// Bitwise OR operator overload for NRF24L01_SetupAWBits (only OR makes sense for this enum)
constexpr NRF24L01_SetupAWBits operator|(NRF24L01_SetupAWBits a, NRF24L01_SetupAWBits b)
{
    return static_cast<NRF24L01_SetupAWBits>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

// 7. SETUP_RETR (Setup of Automatic Retransmission) Register Bit Flags
enum class NRF24L01_SetupRetrBits : uint8_t
{
    // Auto Re-transmit Delay (ARD) - Bits 7:4
    // Delay defined from end of transmission to start of next transmission
    AutoReTxDelay_250uS = 0x00, // Default value for ARD (0000)
    AutoReTxDelay_500uS = 0x10,
    AutoReTxDelay_750uS = 0x20,
    AutoReTxDelay_1000uS = 0x30,
    AutoReTxDelay_1250uS = 0x40,
    AutoReTxDelay_1500uS = 0x50,
    AutoReTxDelay_1750uS = 0x60,
    AutoReTxDelay_2000uS = 0x70,
    AutoReTxDelay_2250uS = 0x80,
    AutoReTxDelay_2500uS = 0x90,
    AutoReTxDelay_2750uS = 0xA0,
    AutoReTxDelay_3000uS = 0xB0,
    AutoReTxDelay_3250uS = 0xC0,
    AutoReTxDelay_3500uS = 0xD0,
    AutoReTxDelay_3750uS = 0xE0,
    AutoReTxDelay_4000uS = 0xF0,

    // Auto Re-transmit Count (ARC) - Bits 3:0
    AutoReTx_Disable = 0x00, // Re-Transmit disabled (ARC = 0000)
    AutoReTxTimes_1 = 0x01,
    AutoReTxTimes_2 = 0x02,
    AutoReTxTimes_3 = 0x03, // Default for ARC (0011)
    AutoReTxTimes_4 = 0x04,
    AutoReTxTimes_5 = 0x05,
    AutoReTxTimes_6 = 0x06,
    AutoReTxTimes_7 = 0x07,
    AutoReTxTimes_8 = 0x08,
    AutoReTxTimes_9 = 0x09,
    AutoReTxTimes_10 = 0x0A,
    AutoReTxTimes_11 = 0x0B,
    AutoReTxTimes_12 = 0x0C,
    AutoReTxTimes_13 = 0x0D,
    AutoReTxTimes_14 = 0x0E,
    AutoReTxTimes_15 = 0x0F
};

// Bitwise operator overloads for NRF24L01_SetupRetrBits
constexpr NRF24L01_SetupRetrBits operator|(NRF24L01_SetupRetrBits a, NRF24L01_SetupRetrBits b)
{
    return static_cast<NRF24L01_SetupRetrBits>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}
constexpr NRF24L01_SetupRetrBits operator&(NRF24L01_SetupRetrBits a, NRF24L01_SetupRetrBits b)
{
    return static_cast<NRF24L01_SetupRetrBits>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}
constexpr NRF24L01_SetupRetrBits operator~(NRF24L01_SetupRetrBits a)
{
    return static_cast<NRF24L01_SetupRetrBits>(~static_cast<uint8_t>(a));
}
constexpr NRF24L01_SetupRetrBits &operator|=(NRF24L01_SetupRetrBits &a, NRF24L01_SetupRetrBits b)
{
    a = a | b;
    return a;
}
constexpr NRF24L01_SetupRetrBits &operator&=(NRF24L01_SetupRetrBits &a, NRF24L01_SetupRetrBits b)
{
    a = a & b;
    return a;
}

// 8. RF_SETUP (RF Setup) Register Bit Flags
enum class NRF24L01_RfSetupBits : uint8_t
{
    // RF Power (RF_PWR) - Bits 2:1
    RF_PWR_M18dBm = 0x00, // -18dBm output power (00)
    RF_PWR_M12dBm = 0x02, // -12dBm output power (01)
    RF_PWR_M6dBm = 0x04,  // -6dBm output power (10)
    RF_PWR_0dBm = 0x06,   // 0dBm output power (11)

    // Air Data Rate (RF_DR_LOW, RF_DR_HIGH) - Bits 5, 3
    // Note: RF_DR_LOW (bit 5) and RF_DR_HIGH (bit 3) combine to set data rate.

    RF_DR_1Mbps = 0x00,   // 1Mbps data rate (default)
    RF_DR_2Mbps = 0x08,   // 2Mbps data rate (RF_DR_HIGH bit set)
    RF_DR_250Kbps = 0x20, // 250kbps data rate (RF_DR_LOW bit set)

    LNA_HCURR = 0x01 // Setup LNA gain. Set to '1' for high gain LNA.
    // PLL_LOCK (bit 4) is for test purposes, not typically used in application code.
    // CONT_WAVE (bit 7) enables continuous carrier transmit, also for testing.
};

// Bitwise operator overloads for NRF24L01_RfSetupBits
constexpr NRF24L01_RfSetupBits operator|(NRF24L01_RfSetupBits a, NRF24L01_RfSetupBits b)
{
    return static_cast<NRF24L01_RfSetupBits>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}
constexpr NRF24L01_RfSetupBits operator&(NRF24L01_RfSetupBits a, NRF24L01_RfSetupBits b)
{
    return static_cast<NRF24L01_RfSetupBits>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}
constexpr NRF24L01_RfSetupBits operator~(NRF24L01_RfSetupBits a)
{
    return static_cast<NRF24L01_RfSetupBits>(~static_cast<uint8_t>(a));
}
constexpr NRF24L01_RfSetupBits &operator|=(NRF24L01_RfSetupBits &a, NRF24L01_RfSetupBits b)
{
    a = a | b;
    return a;
}
constexpr NRF24L01_RfSetupBits &operator&=(NRF24L01_RfSetupBits &a, NRF24L01_RfSetupBits b)
{
    a = a & b;
    return a;
}

// 9. STATUS Register Bit Flags
// Note: This register's bits are often cleared by writing '1' to them.
enum class NRF24L01_StatusBits : uint8_t
{
    // Interrupt Flags (Cleared by writing 1)
    RX_DR = 0x40,  // Data Ready RX FIFO interrupt. Asserted when new data arrives in RX FIFO.
    TX_DS = 0x20,  // Data Sent TX FIFO interrupt. Asserted when packet transmitted.
    MAX_RT = 0x10, // Max number of TX retransmits interrupt. Asserted when MAX_RT is reached.
    TX_FULL = 0x01, // TX FIFO Full flag. Asserted when TX FIFO is full.

    // RX Pipe Number for RX_DR (P_NO[2:0] are bits 3:1)
    // These values represent the masked bits for the pipe number.
    // To get the actual pipe number, you'd typically read the register,
    // mask with 0x0E, and then right-shift by 1.
    RX_P_NO_0 = 0x00,     // Data pipe 0 (when P_NO bits are 000)
    RX_P_NO_1 = 0x02,     // Data pipe 1 (when P_NO bits are 001)
    RX_P_NO_2 = 0x04,     // Data pipe 2 (when P_NO bits are 010)
    RX_P_NO_3 = 0x06,     // Data pipe 3 (when P_NO bits are 011)
    RX_P_NO_4 = 0x08,     // Data pipe 4 (when P_NO bits are 100)
    RX_P_NO_5 = 0x0A,     // Data pipe 5 (when P_NO bits are 101)
    RX_P_NO_EMPTY = 0x0E, // RX FIFO Empty (when P_NO bits are 111)
};

// Bitwise operator overloads for NRF24L01_StatusBits
constexpr NRF24L01_StatusBits operator|(NRF24L01_StatusBits a, NRF24L01_StatusBits b)
{
    return static_cast<NRF24L01_StatusBits>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}
constexpr NRF24L01_StatusBits operator&(NRF24L01_StatusBits a, NRF24L01_StatusBits b)
{
    return static_cast<NRF24L01_StatusBits>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}
constexpr NRF24L01_StatusBits operator~(NRF24L01_StatusBits a)
{
    return static_cast<NRF24L01_StatusBits>(~static_cast<uint8_t>(a));
}
constexpr NRF24L01_StatusBits &operator|=(NRF24L01_StatusBits &a, NRF24L01_StatusBits b)
{
    a = a | b;
    return a;
}
constexpr NRF24L01_StatusBits &operator&=(NRF24L01_StatusBits &a, NRF24L01_StatusBits b)
{
    a = a & b;
    return a;
}


enum class NRF24L01_FIFOStatusBits : uint8_t
{
    // FIFO Status
    TX_FULL = 0x20, // TX FIFO full flag (BIT0). When set, TX FIFO is full.
    TX_EMPTY = 0x10, // RX FIFO empty flag (BIT0). When set, RX FIFO is empty.
    RX_FULL = 0x02, // RX FIFO full flag (BIT0). When set, RX FIFO is full.
    RX_EMPTY = 0x01, // RX FIFO empty flag (BIT0). When set, RX FIFO is empty.
    // TX_FIFO_EMPTY is not a direct bit, but implied when TX_FULL is 0 and TX_FIFO_LEVEL is 0
    // The original MASK_TX_FULL was likely meant as a flag for the TX_FULL bit.
};

// Bitwise operator overloads for NRF24L01_FIFOStatusBits
constexpr NRF24L01_FIFOStatusBits operator|(NRF24L01_FIFOStatusBits a, NRF24L01_FIFOStatusBits b)
{
    return static_cast<NRF24L01_FIFOStatusBits>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}
constexpr NRF24L01_FIFOStatusBits operator&(NRF24L01_FIFOStatusBits a, NRF24L01_FIFOStatusBits b)
{
    return static_cast<NRF24L01_FIFOStatusBits>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}
constexpr NRF24L01_FIFOStatusBits operator~(NRF24L01_FIFOStatusBits a)
{
    return static_cast<NRF24L01_FIFOStatusBits>(~static_cast<uint8_t>(a));
}
constexpr NRF24L01_FIFOStatusBits &operator|=(NRF24L01_FIFOStatusBits &a, NRF24L01_FIFOStatusBits b)
{
    a = a | b;
    return a;
}
constexpr NRF24L01_FIFOStatusBits &operator&=(NRF24L01_FIFOStatusBits &a, NRF24L01_FIFOStatusBits b)
{
    a = a & b;
    return a;
}




#endif // NRF24L01_ENUM_HPP
