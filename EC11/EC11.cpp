#include "EC11.h"


EC11::EC11(TIM_HandleTypeDef *htim, GPIO_TypeDef *A_Port, uint16_t A_Pin, GPIO_TypeDef *B_Port, uint16_t B_Pin,
        GPIO_TypeDef *IRQ_Port, uint16_t IRQ_Pin): timHandler(htim)
{
    this->A_Port = A_Port;
    this->A_Pin = A_Pin;
    this->B_Port = B_Port;
    this->B_Pin = B_Pin;
    this->IRQ_Port = IRQ_Port;
    this->IRQ_Pin = IRQ_Pin;
};

EC11::~EC11()
{
    // Destructor can be used to clean up resources if needed
    // Currently, no dynamic memory allocation is done, so nothing to free
}


uint8_t EC11::ENCODER_READ(void){
    Direction=__HAL_TIM_IS_TIM_COUNTING_DOWN(&htim3); //读取编码器旋转方向
    CaptureNumber=__HAL_TIM_GET_COUNTER(&htim3);   //读取脉冲计数值
    return Direction;
}