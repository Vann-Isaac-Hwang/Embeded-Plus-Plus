#ifndef AD9959_CLASS_HPP
#define AD9959_CLASS_HPP

#include "main.h" // 包含STM32 HAL库的主头文件

// 如果main.h中没有定义，则在这里定义
// #define uchar unsigned char
// #define uint unsigned int

class AD9959 {
public:

    // 存储引脚信息
    struct GpioPin {
        GPIO_TypeDef* port;
        uint16_t pin;
    };

    // 构造函数：初始化引脚的GPIO端口和引脚号
    AD9959(
        GPIO_TypeDef* sclkPort, uint16_t sclkPin,
        GPIO_TypeDef* csPort, uint16_t csPin,
        GPIO_TypeDef* ioUpPort, uint16_t ioUpPin,
        GPIO_TypeDef* sd0Port, uint16_t sd0Pin,
        GPIO_TypeDef* p0Port, uint16_t p0Pin,
        GPIO_TypeDef* p1Port, uint16_t p1Pin,
        GPIO_TypeDef* p2Port, uint16_t p2Pin,
        GPIO_TypeDef* p3Port, uint16_t p3Pin,
        GPIO_TypeDef* sd1Port, uint16_t sd1Pin,
        GPIO_TypeDef* sd2Port, uint16_t sd2Pin,
        GPIO_TypeDef* sd3Port, uint16_t sd3Pin,
        GPIO_TypeDef* pwrPort, uint16_t pwrPin,
        GPIO_TypeDef* resetPort, uint16_t resetPin
    );

    void Init_GPIOs();

    // AD9959初始化函数
    void init();

    // 写入频率调谐字
    void writeFrequencyTuningWord(double frequency, uint8_t* channelFrequencyTuningWordData);

    // IO更新
    void ioUpdate();

    // 通过SPI写入AD9959寄存器
    void writeToAD9959ViaSpi(uint8_t registerAddress, uint8_t numberOfRegisters, uint8_t* registerData, bool doIoUpdate);

    // 写入相位偏移调谐字
    void writePhaseOffsetTuningWord(double phase, uint8_t* channelPhaseOffsetTuningWordData);

    // 写入幅度调谐字 (用于幅度扫描时)
    void writeAmplitudeTuningWord(double amplitude, uint8_t* channelAmplitudeTuningWordData);

    // 写入幅度调谐字 (用于固定幅度)
    void writeAmplitudeTuningWord1(double amplitude, uint8_t* channelAmplitudeTuningWordData, uint8_t* asrAmplitudeWordData);

private:

    GpioPin sclkPin_;
    GpioPin csPin_;
    GpioPin ioUpPin_;
    GpioPin sd0Pin_;
    GpioPin p0Pin_;
    GpioPin p1Pin_;
    GpioPin p2Pin_;
    GpioPin p3Pin_;
    GpioPin sd1Pin_;
    GpioPin sd2Pin_;
    GpioPin sd3Pin_;
    GpioPin pwrPin_;
    GpioPin resetPin_;

    // 私有函数，用于控制GPIO引脚
    void setPin(GpioPin gpio, GPIO_PinState state);
    
    // 常量
    static constexpr double SYSTEM_CLK = 500000000.0; // AD9959系统时钟频率，500MHz

public:
    // Outset
    // void AllChannl_init();
    void AD9959_Setwavefrequency(double f);
    void AD9959_enablechanne00(void);
    void AD9959_enablechannel0(void);
    void AD9959_enablechannel1(void);
    void AD9959_enablechannel2(void);
    void AD9959_enablechannel3(void);
    void AD9959_Setwavephase(double f,int p);
    void AD9959_Setwaveamplitute(double f,int a);
    void AD9959_SetFremodulation2(double f1,double f2);
    void AD9959_SetPhamodulation2(double f,int p1,int p2);
    void AD9959_SetAM2(double f);
    void AD9959_SetFremodulation4(double f1,double f2,double f3,double f4);
    void AD9959_SetPhamodulation4(double f,int p1,int p2,int p3,int p4);
    void AD9959_SetAM4(double f);
    void AD9959_SetFremodulation42(double f1,double f2,double f3,double f4);
    void AD9959_SetPhamodulation42(double f,int p1,int p2,int p3,int p4);
    void AD9959_SetAM42(double f);
    void AD9959_SetFremodulation80(double f1,double f2,double f3,double f4,double f5,double f6,double f7,double f8);
    void AD9959_SetPhamodulation80(double f,int p1,int p2,int p3,int p4,int p5,int p6,int p7,int p8);
    void AD9959_SetAM80(double f);
    void AD9959_SetFremodulation81(double f1,double f2,double f3,double f4,double f5,double f6,double f7,double f8);
    void AD9959_SetPhamodulation81(double f,int p1,int p2,int p3,int p4,int p5,int p6,int p7,int p8);
    void AD9959_SetAM81(double f);
    void AD9959_SetFremodulation82(double f1,double f2,double f3,double f4,double f5,double f6,double f7,double f8);
    void AD9959_SetPhamodulation82(double f,int p1,int p2,int p3,int p4,int p5,int p6,int p7,int p8);
    void AD9959_SetAM82(double f);
    void AD9959_SetFremodulation83(double f1,double f2,double f3,double f4,double f5,double f6,double f7,double f8);
    void AD9959_SetPhamodulation83(double f,int p1,int p2,int p3,int p4,int p5,int p6,int p7,int p8);
    void AD9959_SetAM83(double f);
    void AD9959_SetFremodulation160(double f1,double f2,double f3,double f4,double f5,double f6,double f7,double f8,double f9,double f10,double f11,double f12,double f13,double f14,double f15,double f16);
    void AD9959_SetPhamodulation160(double f,int p1,int p2,int p3,int p4,int p5,int p6,int p7,int p8,int p9,int p10,int p11,int p12,int p13,int p14,int p15,int p16);
    void AD9959_SetAM160(double f);
    void AD9959_SetFremodulation161(double f1,double f2,double f3,double f4,double f5,double f6,double f7,double f8,double f9,double f10,double f11,double f12,double f13,double f14,double f15,double f16);
    void AD9959_SetPhamodulation161(double f,int p1,int p2,int p3,int p4,int p5,int p6,int p7,int p8,int p9,int p10,int p11,int p12,int p13,int p14,int p15,int p16);
    void AD9959_SetAM161(double f);
    void AD9959_SetFremodulation162(double f1,double f2,double f3,double f4,double f5,double f6,double f7,double f8,double f9,double f10,double f11,double f12,double f13,double f14,double f15,double f16);
    void AD9959_SetPhamodulation162(double f,int p1,int p2,int p3,int p4,int p5,int p6,int p7,int p8,int p9,int p10,int p11,int p12,int p13,int p14,int p15,int p16);
    void AD9959_SetAM162(double f);
    void AD9959_SetFremodulation163(double f1,double f2,double f3,double f4,double f5,double f6,double f7,double f8,double f9,double f10,double f11,double f12,double f13,double f14,double f15,double f16);
    void AD9959_SetPhamodulation163(double f,int p1,int p2,int p3,int p4,int p5,int p6,int p7,int p8,int p9,int p10,int p11,int p12,int p13,int p14,int p15,int p16);
    void AD9959_SetAM163(double f);
    void AD9959_Frequency_Sweep(double f1,double f2,double a1,double a2);
    void AD9959_Phase_Sweep(int p1,int p2,int a1,int a2,double f);
    void AD9959_Amplitute_Sweep(int a,int a1,int a2,double f);
};

#endif // AD9959_CLASS_H
