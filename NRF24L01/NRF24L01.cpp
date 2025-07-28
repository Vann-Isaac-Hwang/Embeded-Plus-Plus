#include "NRF24L01.hpp" // Include the updated header file
#include <string.h>     // For memcpy
#include <functional>
#include "Serial.hpp"


extern Serial serial; // DEBUG USED
// Constructor: Initializes the NRF24L01 driver with necessary SPI and GPIO handles.
NRF24L01::NRF24L01(SPI_HandleTypeDef *spiHandle, GPIO_TypeDef *cePort, uint16_t cePin, GPIO_TypeDef *csnPort, uint16_t csnPin)
    : spiHandle(spiHandle)
{ // Initialize spiHandle in the member initializer list
    // Initialize GPIO port and pin members for CE and CSN
    GPIO_PORT_CE = cePort;
    GPIO_PIN_CE = cePin;
    GPIO_PORT_CSN = csnPort;
    GPIO_PIN_CSN = csnPin;
    // Initialize the dummy buffer for TX payload
    Dummy = new uint8_t[RX_PLOAD_WIDTH]; // Allocate memory for the dummy buffer
    memset(Dummy, 0, RX_PLOAD_WIDTH); // Initialize the dummy buffer to zero
    Dummy_NOP = new uint8_t[TX_PLOAD_WIDTH]; // Allocate memory for the dummy buffer for NOP command
    memset(Dummy_NOP, static_cast<uint8_t>(NRF24L01_Command::NOP), 1); // Initialize the dummy buffer for NOP command
}

// Private Helper Function: Controls the state of the CE (Chip Enable) pin.
// state: true for high (set), false for low (reset).
void NRF24L01::CE_Pin(bool state)
{
    if (state)
        HAL_GPIO_WritePin(GPIO_PORT_CE, GPIO_PIN_CE, GPIO_PIN_SET); // Set CE pin high
    else
        HAL_GPIO_WritePin(GPIO_PORT_CE, GPIO_PIN_CE, GPIO_PIN_RESET); // Set CE pin low
}

// Private Helper Function: Controls the state of the CSN (Chip Select Not) pin.
// state: true for high (set), false for low (reset).
void NRF24L01::CSN_Pin(bool state)
{
    if (state)
        HAL_GPIO_WritePin(GPIO_PORT_CSN, GPIO_PIN_CSN, GPIO_PIN_SET); // Set CSN pin high
    else
        HAL_GPIO_WritePin(GPIO_PORT_CSN, GPIO_PIN_CSN, GPIO_PIN_RESET); // Set CSN pin low
}

// Private Helper Function: Performs a combined SPI transmit and receive operation.
// This function handles the CSN pin toggling for a single SPI transaction.
// txData: Pointer to transmit data buffer.
// rxData: Pointer to receive data buffer (can be NULL if only transmitting).
// size: Number of bytes to transmit/receive.
void NRF24L01::NRF24L01_TransmitReceive(uint8_t *txData, uint8_t *rxData, uint16_t size)
{                                                          // Set CSN low to start SPI transaction
    HAL_SPI_TransmitReceive(spiHandle, txData, rxData, size, HAL_MAX_DELAY); // Perform SPI transfer                                                           // Set CSN high to end SPI transaction
}

bool NRF24L01::checkConnection()
{
      uint8_t buf[5] = {0XA5, 0XA5, 0XA5, 0XA5, 0XA5};
    uint8_t i;                   
    NRF24L01_WriteBuffer(static_cast<uint8_t>(NRF24L01_Command::W_REGISTER) | static_cast<uint8_t>(NRF24L01_Register::TX_ADDR), buf, 5);
    NRF24L01_ReadBuffer(static_cast<uint8_t>(NRF24L01_Register::TX_ADDR), buf, 5); 

    for (i = 0; i < 5; i++)
    {
        if (buf[i] != 0XA5) break;
    }
    
    if (i != 5) return 1;   

    return 0; 
}

void NRF24L01::Init() {
    CE_Pin(0);
    CSN_Pin(1); // Ensure CSN is high before any operation
}

uint8_t NRF24L01::NRF24L01_WriteRegister(uint8_t reg_addr, uint8_t value){
    uint8_t status;
    CSN_Pin(0); // Start SPI transaction
    NRF24L01_TransmitReceive(&reg_addr, &status, 1); // Transmit command byte and receive status
    // Send the value to write
    NRF24L01_TransmitReceive(&value, Dummy, 1); // Transmit value, Dummy is used for rxData as we are only writing
    CSN_Pin(1);                                                         // End SPI transaction
    return status;                                                      // Return the STATUS register value
}

uint8_t NRF24L01::NRF24L01_ReadRegister(uint8_t reg_addr){
    uint8_t reg_value;
    CSN_Pin(0); // Start SPI transaction
    NRF24L01_TransmitReceive(&reg_addr, Dummy, 1); // Transmit command byte and receive register va
    NRF24L01_TransmitReceive(Dummy_NOP, &reg_value, 1); // Read the register value
    CSN_Pin(1); // End SPI transaction
    return reg_value; // Return the register value read
}

uint8_t NRF24L01::NRF24L01_ReadBuffer(uint8_t reg_addr, uint8_t *pBuf, uint8_t len)
{
    uint8_t status;
    // Send the command byte and receive the status byte
    CSN_Pin(0); // Start SPI transaction
    NRF24L01_TransmitReceive(&reg_addr, &status, 1); // Transmit command byte and receive status
    for (int i = 0; i < len; i++)
    {
        NRF24L01_TransmitReceive(Dummy_NOP, &pBuf[i], 1); // Read the actual data. For reading, we transmit dummy bytes (0x00).
    }
    
    CSN_Pin(1); // End SPI transaction
    return status; // Return the STATUS register value
}

uint8_t NRF24L01::NRF24L01_WriteBuffer(uint8_t reg_addr, const uint8_t *pBuf, uint8_t len)
{
    uint8_t status;
    CSN_Pin(0); // Start SPI transaction
    NRF24L01_TransmitReceive(&reg_addr, &status, 1); // Transmit command byte and receive status
    for (int i = 0; i < len; i++)
    {
        NRF24L01_TransmitReceive(const_cast<uint8_t *>(&pBuf[i]), Dummy, 1); // Transmit data buffer, Dummy is used for rxData as we are only writing
    }
    CSN_Pin(1); // End SPI transaction
    return status; // Return the STATUS register value
}

uint8_t NRF24L01::Transmit(uint8_t* data)
{
    uint8_t sta;
    uint8_t rval = 0XFF;
    
    CE_Pin(0);
    NRF24L01_WriteBuffer(static_cast<uint8_t>(NRF24L01_Command::WR_TX_PLOAD), data, TX_PLOAD_WIDTH); // Write data to TX buffer
    CE_Pin(1); // Send the data by setting CE high

    while (NRF24L01_IRQ != 0);          // block until the IRQ is triggered

    sta = NRF24L01_ReadRegister(static_cast<uint8_t>(NRF24L01_Register::STATUS));    
    NRF24L01_WriteRegister(static_cast<uint8_t>(NRF24L01_Register::STATUS), sta); // Clear status bits except for MAX_RT, TX_OK, and RX_DR

    if (sta & static_cast<uint8_t>(NRF24L01_StatusBits::MAX_RT))   
    {
        NRF24L01_WriteRegister(static_cast<uint8_t>(NRF24L01_Command::FLUSH_TX), Dummy_NOP[0]); /* Clear TX FIFO register */
        rval = 1;
    }

    if (sta & static_cast<uint8_t>(NRF24L01_StatusBits::TX_DS))
    {
        rval = 0;   
    }

    return rval;    
}
uint8_t NRF24L01::Receive(uint8_t* data)
{
    uint8_t sta;
    uint8_t rval = 1;
    sta = NRF24L01_ReadRegister(static_cast<uint8_t>(NRF24L01_Register::STATUS)); 
    NRF24L01_WriteRegister(static_cast<uint8_t>(NRF24L01_Register::STATUS) + static_cast<uint8_t>(NRF24L01_Command::W_REGISTER), sta); // Clear RX_DR, TX_DS, and MAX_RT bits
    if (sta & static_cast<uint8_t>(NRF24L01_StatusBits::RX_DR)) 
    {
        NRF24L01_ReadBuffer(static_cast<uint8_t>(NRF24L01_Command::RD_RX_PLOAD), data, RX_PLOAD_WIDTH); 
        NRF24L01_WriteRegister(static_cast<uint8_t>(NRF24L01_Command::FLUSH_RX), Dummy_NOP[0]);
        rval = 0;       
    }

    return rval;    
}

void NRF24L01::RxMode(void)
{
    CE_Pin(0);

    NRF24L01_WriteBuffer(
        static_cast<uint8_t>(NRF24L01_Command::W_REGISTER) + static_cast<uint8_t>(NRF24L01_Register::RX_ADDR_P0),
        (uint8_t *)RX_ADDRESS,
        RX_ADR_WIDTH);

    NRF24L01_WriteRegister(
        static_cast<uint8_t>(NRF24L01_Command::W_REGISTER) + static_cast<uint8_t>(NRF24L01_Register::EN_AA),
        static_cast<uint8_t>(NRF24L01_EnAABits::ENAA_P0)); // Enable auto acknowledgment for data pipe 0

    NRF24L01_WriteRegister(
        static_cast<uint8_t>(NRF24L01_Command::W_REGISTER) + static_cast<uint8_t>(NRF24L01_Register::EN_RXADDR),
        static_cast<uint8_t>(NRF24L01_EnRxAddrBits::ERX_P0)); // Enable data pipe 0

    NRF24L01_WriteRegister(
        static_cast<uint8_t>(NRF24L01_Command::W_REGISTER) + static_cast<uint8_t>(NRF24L01_Register::RF_CH),
        40); // Set RF channel to 40

    NRF24L01_WriteRegister(
        static_cast<uint8_t>(NRF24L01_Command::W_REGISTER) + static_cast<uint8_t>(NRF24L01_Register::RX_PW_P0),
        RX_PLOAD_WIDTH); 

    NRF24L01_WriteRegister(
        static_cast<uint8_t>(NRF24L01_Command::W_REGISTER) + static_cast<uint8_t>(NRF24L01_Register::RF_SETUP),
        static_cast<uint8_t>(
            NRF24L01_RfSetupBits::RF_PWR_0dBm |
            NRF24L01_RfSetupBits::RF_DR_2Mbps |
            NRF24L01_RfSetupBits::LNA_HCURR));

    NRF24L01_WriteRegister(
        static_cast<uint8_t>(NRF24L01_Command::W_REGISTER) + static_cast<uint8_t>(NRF24L01_Register::CONFIG),
        static_cast<uint8_t>(
            NRF24L01_ConfigRegBits::PWR_UP |
            NRF24L01_ConfigRegBits::EN_CRC |
            NRF24L01_ConfigRegBits::CRCO |
            NRF24L01_ConfigRegBits::PRIM_RX)); // Set to RX mode

    CE_Pin(1); 
}

void NRF24L01::TxMode(void)
{
    CE_Pin(0);

  
    NRF24L01_WriteBuffer(
        static_cast<uint8_t>(NRF24L01_Command::W_REGISTER) + static_cast<uint8_t>(NRF24L01_Register::TX_ADDR),
        (uint8_t *)TX_ADDRESS,
        TX_ADR_WIDTH);

    NRF24L01_WriteBuffer(
        static_cast<uint8_t>(NRF24L01_Command::W_REGISTER) + static_cast<uint8_t>(NRF24L01_Register::RX_ADDR_P0),
        (uint8_t *)RX_ADDRESS,
        RX_ADR_WIDTH);

   
    NRF24L01_WriteRegister(
        static_cast<uint8_t>(NRF24L01_Command::W_REGISTER) + static_cast<uint8_t>(NRF24L01_Register::EN_AA),
        static_cast<uint8_t>(NRF24L01_EnAABits::ENAA_P0));

    NRF24L01_WriteRegister(
        static_cast<uint8_t>(NRF24L01_Command::W_REGISTER) + static_cast<uint8_t>(NRF24L01_Register::EN_RXADDR),
        static_cast<uint8_t>(NRF24L01_EnRxAddrBits::ERX_P0));


    NRF24L01_WriteRegister(
        static_cast<uint8_t>(NRF24L01_Command::W_REGISTER) + static_cast<uint8_t>(NRF24L01_Register::SETUP_RETR),
        static_cast<uint8_t>(NRF24L01_SetupRetrBits::AutoReTxDelay_500uS) |
        static_cast<uint8_t>(NRF24L01_SetupRetrBits::AutoReTxTimes_10));

 
    NRF24L01_WriteRegister(
        static_cast<uint8_t>(NRF24L01_Command::W_REGISTER) + static_cast<uint8_t>(NRF24L01_Register::RF_CH),
        40);

    
    NRF24L01_WriteRegister(
        static_cast<uint8_t>(NRF24L01_Command::W_REGISTER) + static_cast<uint8_t>(NRF24L01_Register::RF_SETUP),
        static_cast<uint8_t>(
            NRF24L01_RfSetupBits::RF_PWR_0dBm |
            NRF24L01_RfSetupBits::RF_DR_2Mbps |
            NRF24L01_RfSetupBits::LNA_HCURR));


    NRF24L01_WriteRegister(
        static_cast<uint8_t>(NRF24L01_Command::W_REGISTER) + static_cast<uint8_t>(NRF24L01_Register::CONFIG),
        static_cast<uint8_t>(
            NRF24L01_ConfigRegBits::PWR_UP |
            NRF24L01_ConfigRegBits::EN_CRC |
            NRF24L01_ConfigRegBits::CRCO));

    CE_Pin(1); 
}

void NRF24L01::SetMode(NRF24L01Mode mode)
{
    if (mode == NRF24L01Mode::NRF_TX_MODE)
    {
        TxMode(); 
    }
    else
    { 
        RxMode(); 
    }
}

void NRF24L01::setRxCallback(RxCallback_t callback)
{
    RxCallback = callback; // Set the RX callback function
}
void NRF24L01::setTxCallback(TxCallback_t callback)
{
    TxCallback = callback; // Set the TX callback function
}
void NRF24L01::setMaxCallback(MaxCallback_t callback)
{
    MaxCallback = callback; // Set the MAX callback function
}

void NRF24L01::IRQ_Handler()
{
    uint8_t sta;
    sta = NRF24L01_ReadRegister(static_cast<uint8_t>(NRF24L01_Register::STATUS)); // Read the status register

    NRF24L01_WriteRegister(static_cast<uint8_t>(NRF24L01_Register::STATUS), sta); // Clear RX_DR, TX_DS, and MAX_RT bits

    if (sta & static_cast<uint8_t>(NRF24L01_StatusBits::RX_DR)) // If data received
    {
        if (RxCallback) RxCallback(); // Call the RX callback function
    }

    if (sta & static_cast<uint8_t>(NRF24L01_StatusBits::TX_DS)) // If data transmitted successfully
    {
        if (TxCallback) TxCallback(); // Call the TX callback function
    }

    if (sta & static_cast<uint8_t>(NRF24L01_StatusBits::MAX_RT)) // If maximum retransmissions reached
    {
        NRF24L01_WriteRegister(static_cast<uint8_t>(NRF24L01_Command::FLUSH_TX), Dummy_NOP[0]); // Clear TX FIFO
        if (MaxCallback) MaxCallback(); // Call the MAX callback function
    }
}