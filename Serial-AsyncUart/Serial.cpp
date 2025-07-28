#include "Serial.hpp"
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <algorithm>

Serial::Serial(UART_HandleTypeDef *uart) : uartHandle(uart)
{
    // Initialize the UART handle
    if (uartHandle == nullptr)
    {
        // Error handling: UART handle is null
    }
    else
    {
        InstancePool.push_back(this);
    }
}

Serial::~Serial()
{
    InstancePool.erase(
        std::remove(InstancePool.begin(), InstancePool.end(), this),
        InstancePool.end());
}

HAL_StatusTypeDef Serial::Init(void)
{
    return HAL_OK;
}

HAL_StatusTypeDef Serial::Serial_Write(uint8_t *data)
{
    return HAL_UART_Transmit(uartHandle, data, strlen((char *)data), HAL_MAX_DELAY);
}

HAL_StatusTypeDef Serial::Sprintf(const char *format, ...)
{
    va_list args;
    va_start(args, format);

    char buffer[256];
    unsigned int len = vsnprintf(buffer, sizeof(buffer), format, args);

    va_end(args);

    if (len >= sizeof(buffer))
    {
        return HAL_ERROR;
    }

    return Serial_Write(reinterpret_cast<uint8_t *>(buffer));
}

UART_HandleTypeDef *Serial::getUartHandle()
{
    return uartHandle;
}

char *Serial::getBuffer()
{
    return Buffer;
}

void Serial::Receive_IT(uint16_t size)
{
    receiveSize = size;
    Buffer[size] = '\0'; // Ensure the buffer is null-terminated
    HAL_UART_Receive_IT(uartHandle, reinterpret_cast<uint8_t *>(Buffer), size);
}

int Serial::getReceiveSize()
{
    return receiveSize;
}

std::vector<Serial *> Serial::InstancePool;

std::vector<Serial *> Serial::getInstancePool()
{
    return Serial::InstancePool;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    for (auto &instance : Serial::getInstancePool())
    {
        if (huart->Instance == instance->getUartHandle()->Instance)
        {
            if (instance->RxCallback)   instance->RxCallback();
            instance->Receive_IT(instance->getReceiveSize());
        }
    }
}
