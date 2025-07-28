#ifndef NRF24L01_H
#define NRF24L01_H

#include "NRF24L01_enum.hpp" // Include the enum definitions for NRF24L01 registers and bit flags
#include "main.h"    // Include your main project header, which typically includes STM32 HAL headers
#include <cstdint>   // For uint8_t
#include <functional>


// 10. NRF24L01 Module Operating Modes
enum class NRF24L01Mode : uint8_t {
    NRF_TX_MODE = 0x00, // Transmit mode
    NRF_RX_MODE = 0x01  // Receive mode
};
#define NRF24L01_IRQ      HAL_GPIO_ReadPin(NRF24L01_IRQ_GPIO_Port, NRF24L01_IRQ_Pin)
// Payload and Address Width Definitions (as constant class members)
// These define the standard widths for addresses and payloads.
const uint8_t TX_ADR_WIDTH   = 5;   // Default TX address width in bytes
const uint8_t RX_ADR_WIDTH   = 5;   // Default RX address width in bytes
const uint8_t TX_PLOAD_WIDTH = 32; // Default TX payload width in bytes
const uint8_t RX_PLOAD_WIDTH = 32; // Default RX payload width in bytes

// --- NRF24L01 Class Definition ---
class NRF24L01 {
public:
    using RxCallback_t = std::function<void()>;
    using TxCallback_t = std::function<void()>;
    using MaxCallback_t = std::function<void()>;
    // Constructor: Initializes the NRF24L01 driver with necessary SPI and GPIO handles.
    NRF24L01(SPI_HandleTypeDef* spiHandle, GPIO_TypeDef* cePort, uint16_t cePin, GPIO_TypeDef* csnPort, uint16_t csnPin);
    // Destructor: Cleans up any allocated resources.
    ~NRF24L01() {
        delete[] Dummy; // Free the dummy buffer memory
        delete[] Dummy_NOP; // Free the dummy NOP buffer memory
    }

    // Sets the operating mode of the NRF24L01 module (Transmit or Receive).
    void SetMode(NRF24L01Mode mode);

    bool checkConnection();

    void IRQ_Handler();

    void Init();
    
    uint8_t Transmit(uint8_t* data);
    
    uint8_t Receive(uint8_t* data);

    void setRxCallback(RxCallback_t callback);
    void setTxCallback(TxCallback_t callback);
    void setMaxCallback(MaxCallback_t callback);


    // void FlushTx();

    // void FlushRx();


private:
    RxCallback_t RxCallback = nullptr;
    TxCallback_t TxCallback = nullptr;
    MaxCallback_t MaxCallback = nullptr;
    SPI_HandleTypeDef* spiHandle; // Pointer to the SPI handle for communication


    // GPIO port and pin configurations for CE and CSN pins
    GPIO_TypeDef* GPIO_PORT_CE;
    uint16_t GPIO_PIN_CE;
    GPIO_TypeDef* GPIO_PORT_CSN;
    uint16_t GPIO_PIN_CSN;

    // Dummy for SPI
    uint8_t* Dummy; // Buffer for RX payload

    uint8_t* Dummy_NOP;

    // Private Helper Functions for Pin Control
    // Sets the state (high/low) of the CE pin.
    void CE_Pin(bool state);
    // Sets the state (high/low) of the CSN pin.
    void CSN_Pin(bool state);

    // Private Helper Functions for SPI Communication
    // Performs a combined SPI transmit and receive operation.
    void NRF24L01_TransmitReceive(uint8_t* txData, uint8_t* rxData, uint16_t size);

    // Writes a single byte value to a specified NRF24L01 register.
    uint8_t NRF24L01_WriteRegister(uint8_t reg_addr, uint8_t value);

    uint8_t NRF24L01_ReadRegister(uint8_t reg_addr);

    // Reads multiple bytes from a specified NRF24L01 register.
    uint8_t NRF24L01_ReadBuffer(uint8_t reg_addr, uint8_t* pBuf, uint8_t len);

    // Writes multiple bytes to a specified NRF24L01 register (e.g., address registers or TX payload).
    uint8_t NRF24L01_WriteBuffer(uint8_t reg_addr, const uint8_t* pBuf, uint8_t len);

    // Default static TX address. Can be changed via setter if needed.
    const uint8_t TX_ADDRESS[TX_ADR_WIDTH] = {0x34, 0x43, 0x10, 0x10, 0x01};    /* 发送地址 */
    const uint8_t RX_ADDRESS[RX_ADR_WIDTH] = {0x34, 0x43, 0x10, 0x10, 0x01};

    // Configures the NRF24L01 module for Receive (RX) mode.
    void RxMode();
    // Configures the NRF24L01 module for Transmit (TX) mode.
    void TxMode();
};

#endif // NRF24L01_H
