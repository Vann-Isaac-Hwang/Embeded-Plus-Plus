#ifndef __SERIAL_H
#define __SERIAL_H

#include "main.h"
#include <vector>
#include <functional>

class Serial
{
private:
using RxCallback_t = std::function<void()>;
    UART_HandleTypeDef *uartHandle;
    HAL_StatusTypeDef Serial_Write(uint8_t *data);
    char Buffer[256]; // Buffer for receiving data, adjust size as needed
    int receiveSize = 0;
    static std::vector<Serial *> InstancePool;
public:
    Serial(UART_HandleTypeDef *uart);
    ~Serial();
    HAL_StatusTypeDef Init(void);
    HAL_StatusTypeDef Sprintf(const char *format, ...);
    RxCallback_t RxCallback = nullptr;
    UART_HandleTypeDef* getUartHandle();
    char* getBuffer();
    void Receive_IT(uint16_t size);
    int getReceiveSize();
    static std::vector<Serial*> getInstancePool();
};


#endif // __SERIAL_H
