#ifndef _ENCODER_H_
#define _ENCODER_H_

#include "main.h"
#include "tim.h"
#include <cstdint> // For uint8_t

class EC11
{
private:
    TIM_HandleTypeDef *timHandler;
    GPIO_TypeDef *A_Port;
    uint16_t A_Pin;
    GPIO_TypeDef *B_Port;
    uint16_t B_Pin;
    GPIO_TypeDef *IRQ_Port;
    uint16_t IRQ_Pin;
    uint8_t Direction;
    uint16_t CaptureNumber;

public:
    EC11(TIM_HandleTypeDef *htim, GPIO_TypeDef *A_Port, uint16_t A_Pin, GPIO_TypeDef *B_Port, uint16_t B_Pin,
            GPIO_TypeDef *IRQ_Port, uint16_t IRQ_Pin);
    ~EC11();
    uint8_t ENCODER_READ(void);
};



#endif
