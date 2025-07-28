/*
 * ADF4351.cpp
 *
 *  Created on: Jul 28, 2025
 *      Author: huang
 */

#include "ADF4351.hpp"

ADF4351::ADF4351(GPIO_TypeDef* port,
		uint16_t clkPin, uint16_t dataPin, uint16_t lePin, uint16_t cePin)
	: m_port(port), m_clkPin(clkPin), m_dataPin(dataPin), m_lePin(lePin), m_cePin(cePin) {
}

void ADF4351::Init_GPIOs() {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // 确保GPIO时钟已使能
    if (m_port == GPIOA) {
        __HAL_RCC_GPIOA_CLK_ENABLE();
    } else if (m_port == GPIOB) {
        __HAL_RCC_GPIOB_CLK_ENABLE();
    } else if (m_port == GPIOC) {
        __HAL_RCC_GPIOC_CLK_ENABLE();
    } else if (m_port == GPIOD) {
        __HAL_RCC_GPIOC_CLK_ENABLE();
    } else if (m_port == GPIOE) {
        __HAL_RCC_GPIOC_CLK_ENABLE();
    } else if (m_port == GPIOF) {
        __HAL_RCC_GPIOC_CLK_ENABLE();
    } else if (m_port == GPIOG) {
        __HAL_RCC_GPIOC_CLK_ENABLE();
    }
    // 可根据需要添加更多端口

    // 配置CLK引脚
    GPIO_InitStruct.Pin = m_clkPin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(m_port, &GPIO_InitStruct);

    // 配置DATA引脚
    GPIO_InitStruct.Pin = m_dataPin;
    HAL_GPIO_Init(m_port, &GPIO_InitStruct);

    // 配置LE引脚
    GPIO_InitStruct.Pin = m_lePin;
    HAL_GPIO_Init(m_port, &GPIO_InitStruct);

    // 配置CE引脚
    GPIO_InitStruct.Pin = m_cePin;
    HAL_GPIO_Init(m_port, &GPIO_InitStruct);

}

void ADF4351::Delay_1us(unsigned char t)
{
	unsigned char i=0;
	do
	{
		i=8;											// i=7,t=0.986us,误差14ns
		do
		{
		} while (--i);
	}while(--t);
}

void ADF4351::Init(unsigned int data) {
    HAL_GPIO_WritePin(m_port, m_cePin, GPIO_PIN_SET);
	Delay_1us(5);
    HAL_GPIO_WritePin(m_port, m_clkPin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(m_port, m_lePin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(m_port, m_dataPin, GPIO_PIN_RESET);
	Wdata(0x00580005);	 // 设置寄存器5 ：LD设置为数字锁定监测模式
	Wdata(0x0060a43c);	 // 设置寄存器4 : 差分输出功率设置为5dbm，使能静音至检测到锁定，频段选择时钟设为10K
	Wdata(0x006004b3);	 // 设置寄存器3	：charge cancellation，ABP设为1
	Wdata(0x0D003Fc2| R<<14);	  // 设置寄存器2：电荷泵电流设为5mA，LDP,LDF设为1。
	Wdata(0x08008011);			     //	设置寄存器1 ：预分配器设为8/9
	Wdata(0x00000000|data<<15);	 // N分配器数据写入寄存器R0
}

void ADF4351::Wdata(unsigned int dat) {
	unsigned char i;
    HAL_GPIO_WritePin(m_port, m_clkPin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(m_port, m_lePin, GPIO_PIN_RESET);
	for(i=0; i<32; i++)
	{
		if( dat & 0x80000000 )
		    HAL_GPIO_WritePin(m_port, m_dataPin, GPIO_PIN_SET);
		else
		    HAL_GPIO_WritePin(m_port, m_dataPin, GPIO_PIN_RESET);
		dat <<= 1;
	    HAL_GPIO_WritePin(m_port, m_clkPin, GPIO_PIN_SET);
	    HAL_GPIO_WritePin(m_port, m_clkPin, GPIO_PIN_RESET);
	}
    HAL_GPIO_WritePin(m_port, m_lePin, GPIO_PIN_SET);
}
