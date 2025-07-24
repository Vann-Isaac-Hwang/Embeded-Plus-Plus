#include <AD9959.hpp>
#include <cmath> // For round() if needed, though bit shifts handle this here

void AD9959::Init_GPIOs(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // 使能所有涉及的GPIO端口时钟（PORTC, PORTE, PORTF）
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();

    /*------------ 初始化 PORTE 引脚（PE0, PE1, PE2, PE4, PE5, PE6）------------*/
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 |
                          GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;   // 推挽输出模式
    GPIO_InitStruct.Pull = GPIO_NOPULL;           // 无上下拉
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH; // 高速度（根据需求调整）
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    /*------------ 初始化 PORTC 引脚（PC13）------------*/
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;   // 推挽输出
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM; // 中速（常用LED控制）
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /*------------ 初始化 PORTF 引脚（PF0-PF5）------------*/
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 |
                          GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

    // 可选：设置初始电平（例如默认全低电平）
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5, GPIO_PIN_RESET);
}

// 构造函数：接收并保存所有引脚的GPIO端口和引脚号
AD9959::AD9959(
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
) :
    sclkPin_({sclkPort, sclkPin}),
    csPin_({csPort, csPin}),
    ioUpPin_({ioUpPort, ioUpPin}),
    sd0Pin_({sd0Port, sd0Pin}),
    p0Pin_({p0Port, p0Pin}),
    p1Pin_({p1Port, p1Pin}),
    p2Pin_({p2Port, p2Pin}),
    p3Pin_({p3Port, p3Pin}),
    sd1Pin_({sd1Port, sd1Pin}),
    sd2Pin_({sd2Port, sd2Pin}),
    sd3Pin_({sd3Port, sd3Pin}),
    pwrPin_({pwrPort, pwrPin}),
    resetPin_({resetPort, resetPin})
{
    // 在这里，我们不进行HAL_GPIO_Init()，因为这些引脚通常在CubeMX生成的主函数中初始化
    // 如果需要在此处进行GPIO初始化，可以参考原始C代码中的GPIO_AD9959_Configuration，并使用GPIO_InitTypeDef
    // 例如：
    // GPIO_InitTypeDef GPIO_InitStruct = {0};
    // GPIO_InitStruct.Pin = sclkPin;
    // GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    // GPIO_InitStruct.Pull = GPIO_NOPULL;
    // GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH; // 或适当的速度
    // HAL_GPIO_Init(sclkPort, &GPIO_InitStruct);
    // ... 对所有引脚重复此操作
}

// 私有辅助函数：设置GPIO引脚状态
void AD9959::setPin(GpioPin gpio, GPIO_PinState state) {
    HAL_GPIO_WritePin(gpio.port, gpio.pin, state);
}

// AD9959初始化
void AD9959::init() {
	Init_GPIOs();
    setPin(csPin_, GPIO_PIN_SET);
    setPin(sclkPin_, GPIO_PIN_RESET);
    setPin(ioUpPin_, GPIO_PIN_RESET);
    setPin(p0Pin_, GPIO_PIN_RESET);
    setPin(p1Pin_, GPIO_PIN_RESET);
    setPin(p2Pin_, GPIO_PIN_RESET);
    setPin(p3Pin_, GPIO_PIN_RESET);

    setPin(sd0Pin_, GPIO_PIN_RESET);
    setPin(sd1Pin_, GPIO_PIN_RESET);
    setPin(sd2Pin_, GPIO_PIN_RESET);
    setPin(sd3Pin_, GPIO_PIN_RESET);

    setPin(resetPin_, GPIO_PIN_RESET);
    HAL_Delay(6);
    setPin(resetPin_, GPIO_PIN_SET);
    HAL_Delay(100);
    setPin(resetPin_, GPIO_PIN_RESET);
}

// 将频率转换为调谐字
void AD9959::writeFrequencyTuningWord(double frequency, uint8_t* channelFrequencyTuningWordData) {
    double ftw = frequency * (4294967296.0 / SYSTEM_CLK);
    uint32_t y = static_cast<uint32_t>(ftw); // 转换为32位整数

    channelFrequencyTuningWordData[0] = static_cast<uint8_t>(y >> 24);
    channelFrequencyTuningWordData[1] = static_cast<uint8_t>(y >> 16);
    channelFrequencyTuningWordData[2] = static_cast<uint8_t>(y >> 8);
    channelFrequencyTuningWordData[3] = static_cast<uint8_t>(y >> 0);
}

// IO更新
void AD9959::ioUpdate() {
    setPin(ioUpPin_, GPIO_PIN_RESET);
    HAL_Delay(12);
    setPin(ioUpPin_, GPIO_PIN_SET);
    HAL_Delay(24);
    setPin(ioUpPin_, GPIO_PIN_RESET);
}

// 通过SPI写入AD9959寄存器
void AD9959::writeToAD9959ViaSpi(uint8_t registerAddress, uint8_t numberOfRegisters, uint8_t* registerData, bool doIoUpdate) {
    uint8_t controlValue = registerAddress;
    uint8_t valueToWrite;

    setPin(sclkPin_, GPIO_PIN_RESET);
    setPin(csPin_, GPIO_PIN_RESET); // 拉低CS

    // 写入寄存器地址
    for (int i = 0; i < 8; i++) {
        setPin(sclkPin_, GPIO_PIN_RESET);
        if ((controlValue & 0x80) == 0x80) {
            setPin(sd0Pin_, GPIO_PIN_SET); // 发送1到SDIO0引脚
        } else {
            setPin(sd0Pin_, GPIO_PIN_RESET); // 发送0到SDIO0引脚
        }
        setPin(sclkPin_, GPIO_PIN_SET);
        controlValue <<= 1; // 数据左移
    }
    setPin(sclkPin_, GPIO_PIN_RESET);

    // 写入寄存器数据
    for (int registerIndex = 0; registerIndex < numberOfRegisters; registerIndex++) {
        valueToWrite = registerData[registerIndex];
        for (int i = 0; i < 8; i++) {
            setPin(sclkPin_, GPIO_PIN_RESET);
            if ((valueToWrite & 0x80) == 0x80) {
                setPin(sd0Pin_, GPIO_PIN_SET); // 发送1到SDIO0引脚
            } else {
                setPin(sd0Pin_, GPIO_PIN_RESET); // 发送0到SDIO0引脚
            }
            setPin(sclkPin_, GPIO_PIN_SET);
            valueToWrite <<= 1; // 数据左移
        }
        setPin(sclkPin_, GPIO_PIN_RESET);
    }

    if (doIoUpdate) {
        ioUpdate();
    }
    setPin(csPin_, GPIO_PIN_SET); // 拉高CS
}

// 将相位转换为调谐字
void AD9959::writePhaseOffsetTuningWord(double phase, uint8_t* channelPhaseOffsetTuningWordData) {
    double ptw = phase * (16384.0 / 360.0); // 相位刻度因子
    uint16_t y = static_cast<uint16_t>(ptw); // 转换为16位整数

    channelPhaseOffsetTuningWordData[0] = static_cast<uint8_t>(y >> 8);
    channelPhaseOffsetTuningWordData[1] = static_cast<uint8_t>(y >> 0);
}

// 写入幅度调谐字 (10位幅度控制)
void AD9959::writeAmplitudeTuningWord(double amplitude, uint8_t* channelAmplitudeTuningWordData) {
    // 假设amplitude范围是0-1023，对应10位控制
    // 如果实际应用中有特定刻度因子，请调整 `x`
    double atw = amplitude * (1024.0 / 1024.0); // 幅度刻度因子，这里假设1024对应满幅度
    uint16_t y = static_cast<uint16_t>(atw); // 转换为10位

    channelAmplitudeTuningWordData[0] = static_cast<uint8_t>(y >> 2); // 取高8位
    channelAmplitudeTuningWordData[1] = static_cast<uint8_t>((y << 6) & 0xC0); // 取低2位，并放到字节的高2位
    channelAmplitudeTuningWordData[2] = 0x00;
    channelAmplitudeTuningWordData[3] = 0x00;
}

// 写入幅度调谐字 (带有ASR控制，通常用于自动扫描或更复杂的幅度调整)
void AD9959::writeAmplitudeTuningWord1(double amplitude, uint8_t* channelAmplitudeTuningWordData, uint8_t* asrAmplitudeWordData) {
    double atw = amplitude * (1024.0 / 1024.0) * 2.1; // 这里的2.1是一个示例乘数，根据您的需求调整
    uint16_t y = static_cast<uint16_t>(atw);

    asrAmplitudeWordData[0] = channelAmplitudeTuningWordData[0]; // 沿用FTW的高8位

    // 将ASR的低2位和FTW的低2位结合起来
    // 假设channelAmplitudeTuningWordData[1]的C0位是FTW的低2位
    channelAmplitudeTuningWordData[1] = (channelAmplitudeTuningWordData[1] & 0xFC); // 清除FTW的低2位
    asrAmplitudeWordData[1] = (channelAmplitudeTuningWordData[1] | static_cast<uint8_t>(y >> 8)); // 组合ASR高2位和FTW的高6位

    asrAmplitudeWordData[2] = static_cast<uint8_t>(y & 0xFF); // ASR的低8位
}

// Outset


#define CSR 0x00               //??????????
#define FR1 0x01               //????????1
#define FR2 0x02               //????????2
#define CFR 0x03               //???????????
#define CFTW0 0x04             //32λ???????????????
#define CPOW0 0x05             //14λ?????λ?????????
#define ACR 0x06               //???????????
#define SRR 0x07               //??????趨???
#define RDW 0x08               //??????????趨???
#define FDW 0x09               //??????????趨???


#define uchar unsigned char
#define uint unsigned int

void AD9959::AD9959_enablechannel0(void) // 通道0使能
{
	uchar ChannelSelectRegisterdata0[1] = {0x10}; // 通道选择寄存器，3线传输，数据高位优先

	writeToAD9959ViaSpi(CSR, 1, ChannelSelectRegisterdata0, 0);
}

void AD9959::AD9959_enablechannel1(void) // 通道1使能
{
	uchar ChannelSelectRegisterdata1[1] = {0x20}; // 通道选择寄存器，3线传输，数据高位优先
												  // SDIO0 数据输入 and SDIO2 数据输出
	writeToAD9959ViaSpi(CSR, 1, ChannelSelectRegisterdata1, 0);
}
void AD9959::AD9959_enablechannel2(void) // 通道1使能
{
	uchar ChannelSelectRegisterdata2[1] = {0x40}; // 通道选择寄存器，3线传输，数据高位优先
												  // SDIO0 数据输入 and SDIO2 数据输出
	writeToAD9959ViaSpi(CSR, 1, ChannelSelectRegisterdata2, 0);
}
void AD9959::AD9959_enablechannel3(void) // 通道1使能
{
	uchar ChannelSelectRegisterdata3[1] = {0x80}; // 通道选择寄存器，3线传输，数据高位优先
												  // SDIO0 数据输入 and SDIO2 数据输出
	writeToAD9959ViaSpi(CSR, 1, ChannelSelectRegisterdata3, 0);
}

void AD9959::AD9959_Setwavefrequency(double f) // 输出单个频率
{
	uchar ChannelFrequencyTuningWord1data[4];
	uchar ChannelFunctionRegisterdata[3] = {0x00, 0x23, 0x35}; // 单频模式时选择，启用正弦功能（sine）
	uchar FunctionRegister1data[3] = {0xD0, 0x00, 0x00};

	writeToAD9959ViaSpi(FR1, 3, FunctionRegister1data, 0);		 // 设置功能寄存器
	writeToAD9959ViaSpi(CFR, 3, ChannelFunctionRegisterdata, 0); // 设置通道功能寄存器，目的是为了实现正余弦功能
																 // （没有要求时可以不设置该寄存器同样也可以正常输出）
																 // 寄存器8、9位控制输出幅度，分四个档位
	writeFrequencyTuningWord(f, ChannelFrequencyTuningWord1data);	   // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(CFTW0, 4, ChannelFrequencyTuningWord1data, 1); // 写入频率控制字
}

void AD9959::AD9959_Setwavephase(double f, int p) // 输出相位程序可调
{
	uchar ChannelPhaseOffsetTuningWorddata[2];
	uchar ChannelFrequencyTuningWorddata[4];

	uchar ChannelFunctionRegisterdata[3] = {0x00, 0x23, 0x35}; // 单频模式时选择，启用正弦功能（sine）
	uchar FunctionRegister1data[3] = {0xD0, 0x00, 0x00};
	writeToAD9959ViaSpi(FR1, 3, FunctionRegister1data, 0); // 设置功能寄存器

	writeToAD9959ViaSpi(CFR, 3, ChannelFunctionRegisterdata, 0); // 设置通道功能寄存器，目的是为了实现正余弦功能

	writePhaseOffsetTuningWord(p, ChannelPhaseOffsetTuningWorddata);	// 设置相位转换字
	writeToAD9959ViaSpi(CPOW0, 2, ChannelPhaseOffsetTuningWorddata, 0); // 写入相位控制字

	writeFrequencyTuningWord(f, ChannelFrequencyTuningWorddata);	  // 设置频率转换字
	writeToAD9959ViaSpi(CFTW0, 4, ChannelFrequencyTuningWorddata, 1); // 写入频率控制字
}

void AD9959::AD9959_Setwaveamplitute(double f, int a) // 输出幅度可自定义调节
{
	uchar ChannelFrequencyTuningWorddata[4];
	uchar ASRAmplituteWordata[3];
	uchar AmplitudeControldata[3] = {0xff, 0x17, 0xff};		   // 手动控制输出幅度
	uchar ChannelFunctionRegisterdata[3] = {0x00, 0x23, 0x35}; // 单频模式时选择，启用正弦功能（sine）

	uchar FunctionRegister1data[3] = {0xD0, 0x00, 0x00};
	writeToAD9959ViaSpi(FR1, 3, FunctionRegister1data, 0);		 // 设置功能寄存器
	writeToAD9959ViaSpi(CFR, 3, ChannelFunctionRegisterdata, 0); // 设置通道功能寄存器，目的是为了实现正余弦功能
	writeAmplitudeTuningWord1(a, AmplitudeControldata, ASRAmplituteWordata);
	writeToAD9959ViaSpi(ACR, 3, ASRAmplituteWordata, 0); // 设置输出幅度控制模式

	writeFrequencyTuningWord(f, ChannelFrequencyTuningWorddata);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(CFTW0, 4, ChannelFrequencyTuningWorddata, 1); // 写入频率转换字
}
// 设置二阶调制
//////////////////////////////////////////////////////
void AD9959::AD9959_SetFremodulation2(double f1, double f2)
{
	// 	 u8 a2 = 0;
	uchar ChannelFrequencyTuningWord0data[4];
	uchar ChannelFrequencyTuningWord1data[4];
	uchar ChannelFunctionRegisterdata[3] = {0x80, 0x23, 0x30}; // 无RU/RD
	uchar FunctionRegister1data[3] = {0xD0, 0x00, 0x00};

	writeToAD9959ViaSpi(FR1, 3, FunctionRegister1data, 0);		 // 设置功能寄存器
	writeToAD9959ViaSpi(CFR, 3, ChannelFunctionRegisterdata, 0); // 设置通道功能寄存器

	// 设置起始频率S0（0x0A）

	writeFrequencyTuningWord(f1, ChannelFrequencyTuningWord0data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x0A, 4, ChannelFrequencyTuningWord0data, 0); // 写入频率控制字

	// 设置终止频率E0（0x04）

	writeFrequencyTuningWord(f2, ChannelFrequencyTuningWord1data);	   // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(CFTW0, 4, ChannelFrequencyTuningWord1data, 1); // 写入频率控制字
}

void AD9959::AD9959_SetPhamodulation2(double f, int p1, int p2) // p1起始相位，p2终止相位
{
	// 	 uchar ChannelFrequencyTuningWord0data[4];
	uchar ChannelFrequencyTuningWord1data[4];
	uchar ChannelPhaseOffsetTuningWord0data[2];
	uchar ChannelPhaseOffsetTuningWord1data[2];

	uchar ChannelFunctionRegisterdata[3] = {0xc0, 0x03, 0x34}; // 相位调制模式启用（连续扫描可能开启）
//	uchar AmplitudeControldata[3] = {0x00, 0x03, 0xff};		   // 二级调制时启用，默认输出最大
	uchar FunctionRegister1data[3] = {0xD0, 0x00, 0x00};
	// 	 uchar FunctionRegister1data[3] = {0xD0,0x54,0x00};    						//二级调制时需进行幅度RU/RD时选择
	// PO进行调频，P2进行幅度RU/RD
	// 	 uchar AmplitudeControldata[3] = {0xff,0x1f,0xff}; 								//开启RU/RD

	writeToAD9959ViaSpi(FR1, 3, FunctionRegister1data, 0); // 设置功能寄存器

	writeToAD9959ViaSpi(CFR, 3, ChannelFunctionRegisterdata, 0); // 设置通道功能寄存器，即相位调制启动
	//    writeToAD9959ViaSpi(ACR,3,AmplitudeControldata,0);    					//需输出幅度RU/RD模式才选择,且要修改功能寄存器FR1
	// 设置地址0x05（CPOW0） 的相位

	writePhaseOffsetTuningWord(p1, ChannelPhaseOffsetTuningWord0data);	 // 将十进制相位数转换为2进制相位控制字
	writeToAD9959ViaSpi(CPOW0, 2, ChannelPhaseOffsetTuningWord0data, 0); // 写入相位控制字
	// 设置地址0x0a（CW1） 的相位

	writePhaseOffsetTuningWord(p2, ChannelPhaseOffsetTuningWord1data); // 将十进制相位数转换为2进制相位控制字�
	writeToAD9959ViaSpi(0x0a, 4, ChannelPhaseOffsetTuningWord1data, 0); // 写入相位控制字
	// f=2000000;    //设置输出频率
	writeFrequencyTuningWord(f, ChannelFrequencyTuningWord1data);	   // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(CFTW0, 4, ChannelFrequencyTuningWord1data, 1); // 写入频率控制字
}

void AD9959::AD9959_SetAM2(double f)
{
	uchar ChannelFrequencyTuningWord0data[4];
	uchar ChannelAmplitudeTuningWorddata[4] = {0x1f, 0x30, 0x00, 0x00}; // 72mV
	uchar ChannelFunctionRegisterdata[3] = {0x40, 0x03, 0x30};
	uchar AmplitudeControldata[3] = {0x00, 0x03, 0xff}; // 500mV
	uchar FunctionRegister1data[3] = {0xD0, 0x00, 0x00};

	writeToAD9959ViaSpi(FR1, 3, FunctionRegister1data, 0);		 // 设置功能寄存器
	writeToAD9959ViaSpi(CFR, 3, ChannelFunctionRegisterdata, 0); // 设置通道功能寄存器

	writeToAD9959ViaSpi(ACR, 3, AmplitudeControldata, 0); // 设置幅度大小（S1）

	writeToAD9959ViaSpi(0x0a, 4, ChannelAmplitudeTuningWorddata, 0); // 设置幅度大小（S2）
																	 // 设置输出频率大小
	writeFrequencyTuningWord(f, ChannelFrequencyTuningWord0data);	 // 写频率控制字
	writeToAD9959ViaSpi(CFTW0, 4, ChannelFrequencyTuningWord0data, 1);
}

/////////////////////////////////////////////////////////////////////

// 四阶调制   CH0/1
//////////////////////////////////////////////////////////////////////
void AD9959::AD9959_SetFremodulation4(double f1, double f2, double f3, double f4)
{
	// 	 u8 a = 0;
	uchar ChannelFrequencyTuningWord0data[4];
	uchar ChannelFrequencyTuningWord1data[4];
	uchar ChannelFrequencyTuningWord2data[4];
	uchar ChannelFrequencyTuningWord3data[4];
	uchar ChannelFunctionRegisterdata[3] = {0x80, 0x23, 0x30}; // 无RU/RD
	uchar FunctionRegister1data[3] = {0xD0, 0x01, 0x00};

	writeToAD9959ViaSpi(FR1, 3, FunctionRegister1data, 0);		 // 设置功能寄存器
	writeToAD9959ViaSpi(CFR, 3, ChannelFunctionRegisterdata, 0); // 设置通道功能寄存器

	writeFrequencyTuningWord(f1, ChannelFrequencyTuningWord0data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x04, 4, ChannelFrequencyTuningWord0data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f2, ChannelFrequencyTuningWord1data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x0A, 4, ChannelFrequencyTuningWord1data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f3, ChannelFrequencyTuningWord2data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x0B, 4, ChannelFrequencyTuningWord2data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f4, ChannelFrequencyTuningWord3data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x0C, 4, ChannelFrequencyTuningWord3data, 1); // 写入频率控制字
}
void AD9959::AD9959_SetPhamodulation4(double f, int p1, int p2, int p3, int p4)
{
	// 	 u8 b = 0;
	uchar ChannelFrequencyTuningWord0data[4];
	uchar ChannelPhaseOffsetTuningWord0data[4];
	uchar ChannelPhaseOffsetTuningWord1data[4];
	uchar ChannelPhaseOffsetTuningWord2data[4];
	uchar ChannelPhaseOffsetTuningWord3data[4];

	uchar ChannelFunctionRegisterdata[3] = {0xc0, 0x03, 0x30}; // 相位调制模式启用（连续扫描可能开启）
//	uchar AmplitudeControldata[3] = {0x00, 0x03, 0xff};		   // 四级调制时启用，默认输出最大
	uchar FunctionRegister1data[3] = {0xD0, 0x01, 0x00};

	writeToAD9959ViaSpi(FR1, 3, FunctionRegister1data, 0);		 // 设置功能寄存器
	writeToAD9959ViaSpi(CFR, 3, ChannelFunctionRegisterdata, 0); // 设置通道功能寄存器，即相位调制启动

	writePhaseOffsetTuningWord(p1, ChannelPhaseOffsetTuningWord0data);	 // 将十进制相位数转换为2进制相位控制字
	writeToAD9959ViaSpi(CPOW0, 2, ChannelPhaseOffsetTuningWord0data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p2, ChannelPhaseOffsetTuningWord1data); // 将十进制相位数转换为2进制相位控制字�
	writeToAD9959ViaSpi(0x0a, 4, ChannelPhaseOffsetTuningWord1data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p3, ChannelPhaseOffsetTuningWord2data); // 将十进制相位数转换为2进制相位控制字
	writeToAD9959ViaSpi(0x0b, 4, ChannelPhaseOffsetTuningWord2data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p4, ChannelPhaseOffsetTuningWord3data); // 将十进制相位数转换为2进制相位控制字�
	writeToAD9959ViaSpi(0x0c, 4, ChannelPhaseOffsetTuningWord3data, 1); // 写入相位控制字
	f = 200000;
	writeFrequencyTuningWord(f, ChannelFrequencyTuningWord0data);	   // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(CFTW0, 4, ChannelFrequencyTuningWord0data, 1); // 写入频率控制字
}
void AD9959::AD9959_SetAM4(double f)
{
	// 	 u8 cn = 0;
	uchar ChannelFrequencyTuningWord0data[4];
	uchar AmplitudeControldata[3] = {0x00, 0x03, 0xff};
	uchar ChannelAmplitudeTuningWord1data[4] = {0xdf, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord2data[4] = {0x8f, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord3data[4] = {0x5f, 0x30, 0x00, 0x00};

	uchar ChannelFunctionRegisterdata[3] = {0x40, 0x03, 0x30};
	uchar FunctionRegister1data[3] = {0xD0, 0x01, 0x00};

	writeToAD9959ViaSpi(FR1, 3, FunctionRegister1data, 0);		 // 设置功能寄存器
	writeToAD9959ViaSpi(CFR, 3, ChannelFunctionRegisterdata, 0); // 设置通道功能寄存器

	writeToAD9959ViaSpi(ACR, 3, AmplitudeControldata, 0); // 设置幅度大小（S1）

	writeToAD9959ViaSpi(0x0a, 4, ChannelAmplitudeTuningWord1data, 0); // 设置幅度大小（S2）

	writeToAD9959ViaSpi(0x0b, 4, ChannelAmplitudeTuningWord2data, 0);

	writeToAD9959ViaSpi(0x0c, 4, ChannelAmplitudeTuningWord3data, 0);

	writeFrequencyTuningWord(f, ChannelFrequencyTuningWord0data); // 写频率控制字
	writeToAD9959ViaSpi(CFTW0, 4, ChannelFrequencyTuningWord0data, 1);
}

/////////////////////////////////////////////////////////////////////////
//  四阶调制 CH2/3
/////////////////////////////////////////////////////////////////////////

void AD9959::AD9959_SetFremodulation42(double f1, double f2, double f3, double f4)
{
	// 	 u8 a = 0;
	uchar ChannelFrequencyTuningWord0data[4];
	uchar ChannelFrequencyTuningWord1data[4];
	uchar ChannelFrequencyTuningWord2data[4];
	uchar ChannelFrequencyTuningWord3data[4];
	uchar ChannelFunctionRegisterdata[3] = {0x80, 0x23, 0x30}; // 无RU/RD
	uchar FunctionRegister1data[3] = {0xD0, 0x51, 0x00};

	writeToAD9959ViaSpi(FR1, 3, FunctionRegister1data, 0);		 // 设置功能寄存器
	writeToAD9959ViaSpi(CFR, 3, ChannelFunctionRegisterdata, 0); // 设置通道功能寄存器

	writeFrequencyTuningWord(f1, ChannelFrequencyTuningWord0data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x04, 4, ChannelFrequencyTuningWord0data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f2, ChannelFrequencyTuningWord1data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x0A, 4, ChannelFrequencyTuningWord1data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f3, ChannelFrequencyTuningWord2data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x0B, 4, ChannelFrequencyTuningWord2data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f4, ChannelFrequencyTuningWord3data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x0C, 4, ChannelFrequencyTuningWord3data, 1); // 写入频率控制字
}
void AD9959::AD9959_SetPhamodulation42(double f, int p1, int p2, int p3, int p4)
{
	// 	 u8 b = 0;
	uchar ChannelFrequencyTuningWord0data[4];
	uchar ChannelPhaseOffsetTuningWord0data[4];
	uchar ChannelPhaseOffsetTuningWord1data[4];
	uchar ChannelPhaseOffsetTuningWord2data[4];
	uchar ChannelPhaseOffsetTuningWord3data[4];

	uchar ChannelFunctionRegisterdata[3] = {0xc0, 0x03, 0x30}; // 相位调制模式启用（连续扫描可能开启）
//	uchar AmplitudeControldata[3] = {0x00, 0x03, 0xff};		   // 四级调制时启用，默认输出最大
	uchar FunctionRegister1data[3] = {0xD0, 0x51, 0x00};

	writeToAD9959ViaSpi(FR1, 3, FunctionRegister1data, 0);		 // 设置功能寄存器
	writeToAD9959ViaSpi(CFR, 3, ChannelFunctionRegisterdata, 0); // 设置通道功能寄存器，即相位调制启动

	writePhaseOffsetTuningWord(p1, ChannelPhaseOffsetTuningWord0data);	 // 将十进制相位数转换为2进制相位控制字
	writeToAD9959ViaSpi(CPOW0, 2, ChannelPhaseOffsetTuningWord0data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p2, ChannelPhaseOffsetTuningWord1data); // 将十进制相位数转换为2进制相位控制字�
	writeToAD9959ViaSpi(0x0a, 4, ChannelPhaseOffsetTuningWord1data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p3, ChannelPhaseOffsetTuningWord2data); // 将十进制相位数转换为2进制相位控制字
	writeToAD9959ViaSpi(0x0b, 4, ChannelPhaseOffsetTuningWord2data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p4, ChannelPhaseOffsetTuningWord3data); // 将十进制相位数转换为2进制相位控制字�
	writeToAD9959ViaSpi(0x0c, 4, ChannelPhaseOffsetTuningWord3data, 1); // 写入相位控制字
	f = 200000;
	writeFrequencyTuningWord(f, ChannelFrequencyTuningWord0data);	   // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(CFTW0, 4, ChannelFrequencyTuningWord0data, 1); // 写入频率控制字
}
void AD9959::AD9959_SetAM42(double f)
{
	// 	 u8 cn = 0;
	uchar ChannelFrequencyTuningWord0data[4];
	uchar AmplitudeControldata[3] = {0x00, 0x03, 0xff};
	uchar ChannelAmplitudeTuningWord1data[4] = {0xdf, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord2data[4] = {0x8f, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord3data[4] = {0x5f, 0x30, 0x00, 0x00};

	uchar ChannelFunctionRegisterdata[3] = {0x40, 0x03, 0x30};
	uchar FunctionRegister1data[3] = {0xD0, 0x51, 0x00};

	writeToAD9959ViaSpi(FR1, 3, FunctionRegister1data, 0);		 // 设置功能寄存器
	writeToAD9959ViaSpi(CFR, 3, ChannelFunctionRegisterdata, 0); // 设置通道功能寄存器

	writeToAD9959ViaSpi(ACR, 3, AmplitudeControldata, 0); // 设置幅度大小（S1）

	writeToAD9959ViaSpi(0x0a, 4, ChannelAmplitudeTuningWord1data, 0); // 设置幅度大小（S2）

	writeToAD9959ViaSpi(0x0b, 4, ChannelAmplitudeTuningWord2data, 0);

	writeToAD9959ViaSpi(0x0c, 4, ChannelAmplitudeTuningWord3data, 0);

	writeFrequencyTuningWord(f, ChannelFrequencyTuningWord0data); // 写频率控制字
	writeToAD9959ViaSpi(CFTW0, 4, ChannelFrequencyTuningWord0data, 1);
}

/////////////////////////////////////////////////////////////////////////
//  八阶调制 CH0
/////////////////////////////////////////////////////////////////////////

void AD9959::AD9959_SetFremodulation80(double f1, double f2, double f3, double f4, double f5, double f6, double f7, double f8)
{
	// 	 u8 a = 0;
	uchar ChannelFrequencyTuningWord0data[4];
	uchar ChannelFrequencyTuningWord1data[4];
	uchar ChannelFrequencyTuningWord2data[4];
	uchar ChannelFrequencyTuningWord3data[4];
	uchar ChannelFrequencyTuningWord4data[4];
	uchar ChannelFrequencyTuningWord5data[4];
	uchar ChannelFrequencyTuningWord6data[4];
	uchar ChannelFrequencyTuningWord7data[4];
	uchar ChannelFunctionRegisterdata[3] = {0x80, 0x23, 0x30}; // 无RU/RD
	uchar FunctionRegister1data[3] = {0xD0, 0xc2, 0x00};

	writeToAD9959ViaSpi(FR1, 3, FunctionRegister1data, 0);		 // 设置功能寄存器
	writeToAD9959ViaSpi(CFR, 3, ChannelFunctionRegisterdata, 0); // 设置通道功能寄存器

	writeFrequencyTuningWord(f1, ChannelFrequencyTuningWord0data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x04, 4, ChannelFrequencyTuningWord0data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f2, ChannelFrequencyTuningWord1data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x0A, 4, ChannelFrequencyTuningWord1data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f3, ChannelFrequencyTuningWord2data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x0B, 4, ChannelFrequencyTuningWord2data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f4, ChannelFrequencyTuningWord3data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x0C, 4, ChannelFrequencyTuningWord3data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f5, ChannelFrequencyTuningWord4data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x0D, 4, ChannelFrequencyTuningWord4data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f6, ChannelFrequencyTuningWord5data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x0E, 4, ChannelFrequencyTuningWord5data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f7, ChannelFrequencyTuningWord6data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x0F, 4, ChannelFrequencyTuningWord6data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f8, ChannelFrequencyTuningWord7data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x10, 4, ChannelFrequencyTuningWord7data, 1); // 写入频率控制字
}
void AD9959::AD9959_SetPhamodulation80(double f, int p1, int p2, int p3, int p4, int p5, int p6, int p7, int p8)
{
	// 	 u8 b = 0;
	uchar ChannelFrequencyTuningWord0data[4];
	uchar ChannelPhaseOffsetTuningWord0data[4];
	uchar ChannelPhaseOffsetTuningWord1data[4];
	uchar ChannelPhaseOffsetTuningWord2data[4];
	uchar ChannelPhaseOffsetTuningWord3data[4];
	uchar ChannelPhaseOffsetTuningWord4data[4];
	uchar ChannelPhaseOffsetTuningWord5data[4];
	uchar ChannelPhaseOffsetTuningWord6data[4];
	uchar ChannelPhaseOffsetTuningWord7data[4];

	uchar ChannelFunctionRegisterdata[3] = {0xc0, 0x03, 0x30}; // 相位调制模式启用（连续扫描可能开启）
//	uchar AmplitudeControldata[3] = {0x00, 0x03, 0xff};		   // 八级调制时启用，默认输出最大
	uchar FunctionRegister1data[3] = {0xD0, 0xc2, 0x00};

	writeToAD9959ViaSpi(FR1, 3, FunctionRegister1data, 0);		 // 设置功能寄存器
	writeToAD9959ViaSpi(CFR, 3, ChannelFunctionRegisterdata, 0); // 设置通道功能寄存器，即相位调制启动

	writePhaseOffsetTuningWord(p1, ChannelPhaseOffsetTuningWord0data);	 // 将十进制相位数转换为2进制相位控制字
	writeToAD9959ViaSpi(CPOW0, 2, ChannelPhaseOffsetTuningWord0data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p2, ChannelPhaseOffsetTuningWord1data); // 将十进制相位数转换为2进制相位控制字�
	writeToAD9959ViaSpi(0x0a, 4, ChannelPhaseOffsetTuningWord1data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p3, ChannelPhaseOffsetTuningWord2data); // 将十进制相位数转换为2进制相位控制字
	writeToAD9959ViaSpi(0x0b, 4, ChannelPhaseOffsetTuningWord2data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p4, ChannelPhaseOffsetTuningWord3data); // 将十进制相位数转换为2进制相位控制字�
	writeToAD9959ViaSpi(0x0c, 4, ChannelPhaseOffsetTuningWord3data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p5, ChannelPhaseOffsetTuningWord4data); // 将十进制相位数转换为2进制相位控制字
	writeToAD9959ViaSpi(0x0d, 4, ChannelPhaseOffsetTuningWord4data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p6, ChannelPhaseOffsetTuningWord5data); // 将十进制相位数转换为2进制相位控制字�
	writeToAD9959ViaSpi(0x0e, 4, ChannelPhaseOffsetTuningWord5data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p7, ChannelPhaseOffsetTuningWord6data); // 将十进制相位数转换为2进制相位控制字
	writeToAD9959ViaSpi(0x0f, 4, ChannelPhaseOffsetTuningWord6data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p8, ChannelPhaseOffsetTuningWord7data); // 将十进制相位数转换为2进制相位控制字�
	writeToAD9959ViaSpi(0x10, 4, ChannelPhaseOffsetTuningWord7data, 1); // 写入相位控制字
	f = 200000;
	writeFrequencyTuningWord(f, ChannelFrequencyTuningWord0data);	   // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(CFTW0, 4, ChannelFrequencyTuningWord0data, 1); // 写入频率控制字
}
void AD9959::AD9959_SetAM80(double f)
{
	// 	 u8 cn = 0;
	uchar ChannelFrequencyTuningWord0data[4];

	uchar AmplitudeControldata[3] = {0x00, 0x03, 0xff};
	uchar ChannelAmplitudeTuningWord1data[4] = {0xdf, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord2data[4] = {0xbf, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord3data[4] = {0x9f, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord4data[4] = {0x7f, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord5data[4] = {0x5f, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord6data[4] = {0x3f, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord7data[4] = {0x1f, 0x30, 0x00, 0x00};

	uchar ChannelFunctionRegisterdata[3] = {0x40, 0x03, 0x30};
	uchar FunctionRegister1data[3] = {0xD0, 0xc2, 0x00};

	writeToAD9959ViaSpi(FR1, 3, FunctionRegister1data, 0);		 // 设置功能寄存器
	writeToAD9959ViaSpi(CFR, 3, ChannelFunctionRegisterdata, 0); // 设置通道功能寄存器

	writeToAD9959ViaSpi(ACR, 3, AmplitudeControldata, 0); // 设置幅度大小（S1）

	writeToAD9959ViaSpi(0x0a, 4, ChannelAmplitudeTuningWord1data, 0); // 设置幅度大小（S2）

	writeToAD9959ViaSpi(0x0b, 4, ChannelAmplitudeTuningWord2data, 0);

	writeToAD9959ViaSpi(0x0c, 4, ChannelAmplitudeTuningWord3data, 0);

	writeToAD9959ViaSpi(0x0d, 4, ChannelAmplitudeTuningWord4data, 0); // 设置幅度大小（S1）

	writeToAD9959ViaSpi(0x0e, 4, ChannelAmplitudeTuningWord5data, 0); // 设置幅度大小（S2）

	writeToAD9959ViaSpi(0x0f, 4, ChannelAmplitudeTuningWord6data, 0);

	writeToAD9959ViaSpi(0x10, 4, ChannelAmplitudeTuningWord7data, 0);

	writeFrequencyTuningWord(f, ChannelFrequencyTuningWord0data); // 写频率控制字
	writeToAD9959ViaSpi(CFTW0, 4, ChannelFrequencyTuningWord0data, 1);
}
/////////////////////////////////////////////////////////////////////////
//  八阶调制 CH1
/////////////////////////////////////////////////////////////////////////

void AD9959::AD9959_SetFremodulation81(double f1, double f2, double f3, double f4, double f5, double f6, double f7, double f8)
{
	// 	 u8 a = 0;
	uchar ChannelFrequencyTuningWord0data[4];
	uchar ChannelFrequencyTuningWord1data[4];
	uchar ChannelFrequencyTuningWord2data[4];
	uchar ChannelFrequencyTuningWord3data[4];
	uchar ChannelFrequencyTuningWord4data[4];
	uchar ChannelFrequencyTuningWord5data[4];
	uchar ChannelFrequencyTuningWord6data[4];
	uchar ChannelFrequencyTuningWord7data[4];
	uchar ChannelFunctionRegisterdata[3] = {0x80, 0x23, 0x30}; // 无RU/RD
	uchar FunctionRegister1data[3] = {0xD0, 0xd2, 0x00};

	writeToAD9959ViaSpi(FR1, 3, FunctionRegister1data, 0);		 // 设置功能寄存器
	writeToAD9959ViaSpi(CFR, 3, ChannelFunctionRegisterdata, 0); // 设置通道功能寄存器

	writeFrequencyTuningWord(f1, ChannelFrequencyTuningWord0data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x04, 4, ChannelFrequencyTuningWord0data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f2, ChannelFrequencyTuningWord1data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x0A, 4, ChannelFrequencyTuningWord1data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f3, ChannelFrequencyTuningWord2data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x0B, 4, ChannelFrequencyTuningWord2data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f4, ChannelFrequencyTuningWord3data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x0C, 4, ChannelFrequencyTuningWord3data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f5, ChannelFrequencyTuningWord4data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x0D, 4, ChannelFrequencyTuningWord4data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f6, ChannelFrequencyTuningWord5data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x0E, 4, ChannelFrequencyTuningWord5data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f7, ChannelFrequencyTuningWord6data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x0F, 4, ChannelFrequencyTuningWord6data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f8, ChannelFrequencyTuningWord7data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x10, 4, ChannelFrequencyTuningWord7data, 1); // 写入频率控制字
}
void AD9959::AD9959_SetPhamodulation81(double f, int p1, int p2, int p3, int p4, int p5, int p6, int p7, int p8)
{
	// 	 u8 b = 0;
	uchar ChannelFrequencyTuningWord0data[4];
	uchar ChannelPhaseOffsetTuningWord0data[4];
	uchar ChannelPhaseOffsetTuningWord1data[4];
	uchar ChannelPhaseOffsetTuningWord2data[4];
	uchar ChannelPhaseOffsetTuningWord3data[4];
	uchar ChannelPhaseOffsetTuningWord4data[4];
	uchar ChannelPhaseOffsetTuningWord5data[4];
	uchar ChannelPhaseOffsetTuningWord6data[4];
	uchar ChannelPhaseOffsetTuningWord7data[4];

	uchar ChannelFunctionRegisterdata[3] = {0xc0, 0x03, 0x30}; // 相位调制模式启用（连续扫描可能开启）
//	uchar AmplitudeControldata[3] = {0x00, 0x03, 0xff};		   // 八级调制时启用，默认输出最大
	uchar FunctionRegister1data[3] = {0xD0, 0xd2, 0x00};

	writeToAD9959ViaSpi(FR1, 3, FunctionRegister1data, 0);		 // 设置功能寄存器
	writeToAD9959ViaSpi(CFR, 3, ChannelFunctionRegisterdata, 0); // 设置通道功能寄存器，即相位调制启动

	writePhaseOffsetTuningWord(p1, ChannelPhaseOffsetTuningWord0data);	 // 将十进制相位数转换为2进制相位控制字
	writeToAD9959ViaSpi(CPOW0, 2, ChannelPhaseOffsetTuningWord0data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p2, ChannelPhaseOffsetTuningWord1data); // 将十进制相位数转换为2进制相位控制字�
	writeToAD9959ViaSpi(0x0a, 4, ChannelPhaseOffsetTuningWord1data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p3, ChannelPhaseOffsetTuningWord2data); // 将十进制相位数转换为2进制相位控制字
	writeToAD9959ViaSpi(0x0b, 4, ChannelPhaseOffsetTuningWord2data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p4, ChannelPhaseOffsetTuningWord3data); // 将十进制相位数转换为2进制相位控制字�
	writeToAD9959ViaSpi(0x0c, 4, ChannelPhaseOffsetTuningWord3data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p5, ChannelPhaseOffsetTuningWord4data); // 将十进制相位数转换为2进制相位控制字
	writeToAD9959ViaSpi(0x0d, 4, ChannelPhaseOffsetTuningWord4data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p6, ChannelPhaseOffsetTuningWord5data); // 将十进制相位数转换为2进制相位控制字�
	writeToAD9959ViaSpi(0x0e, 4, ChannelPhaseOffsetTuningWord5data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p7, ChannelPhaseOffsetTuningWord6data); // 将十进制相位数转换为2进制相位控制字
	writeToAD9959ViaSpi(0x0f, 4, ChannelPhaseOffsetTuningWord6data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p8, ChannelPhaseOffsetTuningWord7data); // 将十进制相位数转换为2进制相位控制字�
	writeToAD9959ViaSpi(0x10, 4, ChannelPhaseOffsetTuningWord7data, 1); // 写入相位控制字
	f = 200000;
	writeFrequencyTuningWord(f, ChannelFrequencyTuningWord0data);	   // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(CFTW0, 4, ChannelFrequencyTuningWord0data, 1); // 写入频率控制字
}
void AD9959::AD9959_SetAM81(double f)
{
	// 	 u8 cn = 0;
	uchar ChannelFrequencyTuningWord0data[4];

	uchar AmplitudeControldata[3] = {0x00, 0x03, 0xff};
	uchar ChannelAmplitudeTuningWord1data[4] = {0xdf, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord2data[4] = {0xbf, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord3data[4] = {0x9f, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord4data[4] = {0x7f, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord5data[4] = {0x5f, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord6data[4] = {0x3f, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord7data[4] = {0x1f, 0x30, 0x00, 0x00};

	uchar ChannelFunctionRegisterdata[3] = {0x40, 0x03, 0x30};
	uchar FunctionRegister1data[3] = {0xD0, 0xd2, 0x00};

	writeToAD9959ViaSpi(FR1, 3, FunctionRegister1data, 0);		 // 设置功能寄存器
	writeToAD9959ViaSpi(CFR, 3, ChannelFunctionRegisterdata, 0); // 设置通道功能寄存器

	writeToAD9959ViaSpi(ACR, 3, AmplitudeControldata, 0); // 设置幅度大小（S1）

	writeToAD9959ViaSpi(0x0a, 4, ChannelAmplitudeTuningWord1data, 0); // 设置幅度大小（S2）

	writeToAD9959ViaSpi(0x0b, 4, ChannelAmplitudeTuningWord2data, 0);

	writeToAD9959ViaSpi(0x0c, 4, ChannelAmplitudeTuningWord3data, 0);

	writeToAD9959ViaSpi(0x0d, 4, ChannelAmplitudeTuningWord4data, 0); // 设置幅度大小（S1）

	writeToAD9959ViaSpi(0x0e, 4, ChannelAmplitudeTuningWord5data, 0); // 设置幅度大小（S2）

	writeToAD9959ViaSpi(0x0f, 4, ChannelAmplitudeTuningWord6data, 0);

	writeToAD9959ViaSpi(0x10, 4, ChannelAmplitudeTuningWord7data, 0);

	writeFrequencyTuningWord(f, ChannelFrequencyTuningWord0data); // 写频率控制字
	writeToAD9959ViaSpi(CFTW0, 4, ChannelFrequencyTuningWord0data, 1);
}
/////////////////////////////////////////////////////////////////////////
//  八阶调制 CH2
/////////////////////////////////////////////////////////////////////////

void AD9959::AD9959_SetFremodulation82(double f1, double f2, double f3, double f4, double f5, double f6, double f7, double f8)
{
	// 	 u8 a = 0;
	uchar ChannelFrequencyTuningWord0data[4];
	uchar ChannelFrequencyTuningWord1data[4];
	uchar ChannelFrequencyTuningWord2data[4];
	uchar ChannelFrequencyTuningWord3data[4];
	uchar ChannelFrequencyTuningWord4data[4];
	uchar ChannelFrequencyTuningWord5data[4];
	uchar ChannelFrequencyTuningWord6data[4];
	uchar ChannelFrequencyTuningWord7data[4];
	uchar ChannelFunctionRegisterdata[3] = {0x80, 0x23, 0x30}; // 无RU/RD
	uchar FunctionRegister1data[3] = {0xD0, 0xe2, 0x00};

	writeToAD9959ViaSpi(FR1, 3, FunctionRegister1data, 0);		 // 设置功能寄存器
	writeToAD9959ViaSpi(CFR, 3, ChannelFunctionRegisterdata, 0); // 设置通道功能寄存器

	writeFrequencyTuningWord(f1, ChannelFrequencyTuningWord0data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x04, 4, ChannelFrequencyTuningWord0data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f2, ChannelFrequencyTuningWord1data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x0A, 4, ChannelFrequencyTuningWord1data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f3, ChannelFrequencyTuningWord2data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x0B, 4, ChannelFrequencyTuningWord2data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f4, ChannelFrequencyTuningWord3data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x0C, 4, ChannelFrequencyTuningWord3data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f5, ChannelFrequencyTuningWord4data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x0D, 4, ChannelFrequencyTuningWord4data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f6, ChannelFrequencyTuningWord5data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x0E, 4, ChannelFrequencyTuningWord5data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f7, ChannelFrequencyTuningWord6data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x0F, 4, ChannelFrequencyTuningWord6data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f8, ChannelFrequencyTuningWord7data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x10, 4, ChannelFrequencyTuningWord7data, 1); // 写入频率控制字
}
void AD9959::AD9959_SetPhamodulation82(double f, int p1, int p2, int p3, int p4, int p5, int p6, int p7, int p8)
{
	// 	 u8 b = 0;
	uchar ChannelFrequencyTuningWord0data[4];
	uchar ChannelPhaseOffsetTuningWord0data[4];
	uchar ChannelPhaseOffsetTuningWord1data[4];
	uchar ChannelPhaseOffsetTuningWord2data[4];
	uchar ChannelPhaseOffsetTuningWord3data[4];
	uchar ChannelPhaseOffsetTuningWord4data[4];
	uchar ChannelPhaseOffsetTuningWord5data[4];
	uchar ChannelPhaseOffsetTuningWord6data[4];
	uchar ChannelPhaseOffsetTuningWord7data[4];

	uchar ChannelFunctionRegisterdata[3] = {0xc0, 0x03, 0x30}; // 相位调制模式启用（连续扫描可能开启）
//	uchar AmplitudeControldata[3] = {0x00, 0x03, 0xff};		   // 八级调制时启用，默认输出最大
	uchar FunctionRegister1data[3] = {0xD0, 0xe2, 0x00};

	writeToAD9959ViaSpi(FR1, 3, FunctionRegister1data, 0);		 // 设置功能寄存器
	writeToAD9959ViaSpi(CFR, 3, ChannelFunctionRegisterdata, 0); // 设置通道功能寄存器，即相位调制启动

	writePhaseOffsetTuningWord(p1, ChannelPhaseOffsetTuningWord0data);	 // 将十进制相位数转换为2进制相位控制字
	writeToAD9959ViaSpi(CPOW0, 2, ChannelPhaseOffsetTuningWord0data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p2, ChannelPhaseOffsetTuningWord1data); // 将十进制相位数转换为2进制相位控制字�
	writeToAD9959ViaSpi(0x0a, 4, ChannelPhaseOffsetTuningWord1data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p3, ChannelPhaseOffsetTuningWord2data); // 将十进制相位数转换为2进制相位控制字
	writeToAD9959ViaSpi(0x0b, 4, ChannelPhaseOffsetTuningWord2data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p4, ChannelPhaseOffsetTuningWord3data); // 将十进制相位数转换为2进制相位控制字�
	writeToAD9959ViaSpi(0x0c, 4, ChannelPhaseOffsetTuningWord3data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p5, ChannelPhaseOffsetTuningWord4data); // 将十进制相位数转换为2进制相位控制字
	writeToAD9959ViaSpi(0x0d, 4, ChannelPhaseOffsetTuningWord4data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p6, ChannelPhaseOffsetTuningWord5data); // 将十进制相位数转换为2进制相位控制字�
	writeToAD9959ViaSpi(0x0e, 4, ChannelPhaseOffsetTuningWord5data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p7, ChannelPhaseOffsetTuningWord6data); // 将十进制相位数转换为2进制相位控制字
	writeToAD9959ViaSpi(0x0f, 4, ChannelPhaseOffsetTuningWord6data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p8, ChannelPhaseOffsetTuningWord7data); // 将十进制相位数转换为2进制相位控制字�
	writeToAD9959ViaSpi(0x10, 4, ChannelPhaseOffsetTuningWord7data, 1); // 写入相位控制字
	f = 200000;
	writeFrequencyTuningWord(f, ChannelFrequencyTuningWord0data);	   // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(CFTW0, 4, ChannelFrequencyTuningWord0data, 1); // 写入频率控制字
}
void AD9959::AD9959_SetAM82(double f)
{
	// 	 u8 cn = 0;
	uchar ChannelFrequencyTuningWord0data[4];

	uchar AmplitudeControldata[3] = {0x00, 0x03, 0xff};
	uchar ChannelAmplitudeTuningWord1data[4] = {0xdf, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord2data[4] = {0xbf, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord3data[4] = {0x9f, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord4data[4] = {0x7f, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord5data[4] = {0x5f, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord6data[4] = {0x3f, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord7data[4] = {0x1f, 0x30, 0x00, 0x00};

	uchar ChannelFunctionRegisterdata[3] = {0x40, 0x03, 0x30};
	uchar FunctionRegister1data[3] = {0xD0, 0xe2, 0x00};

	writeToAD9959ViaSpi(FR1, 3, FunctionRegister1data, 0);		 // 设置功能寄存器
	writeToAD9959ViaSpi(CFR, 3, ChannelFunctionRegisterdata, 0); // 设置通道功能寄存器

	writeToAD9959ViaSpi(ACR, 3, AmplitudeControldata, 0); // 设置幅度大小（S1）

	writeToAD9959ViaSpi(0x0a, 4, ChannelAmplitudeTuningWord1data, 0); // 设置幅度大小（S2）

	writeToAD9959ViaSpi(0x0b, 4, ChannelAmplitudeTuningWord2data, 0);

	writeToAD9959ViaSpi(0x0c, 4, ChannelAmplitudeTuningWord3data, 0);

	writeToAD9959ViaSpi(0x0d, 4, ChannelAmplitudeTuningWord4data, 0); // 设置幅度大小（S1）

	writeToAD9959ViaSpi(0x0e, 4, ChannelAmplitudeTuningWord5data, 0); // 设置幅度大小（S2）

	writeToAD9959ViaSpi(0x0f, 4, ChannelAmplitudeTuningWord6data, 0);

	writeToAD9959ViaSpi(0x10, 4, ChannelAmplitudeTuningWord7data, 0);

	writeFrequencyTuningWord(f, ChannelFrequencyTuningWord0data); // 写频率控制字
	writeToAD9959ViaSpi(CFTW0, 4, ChannelFrequencyTuningWord0data, 1);
}

/////////////////////////////////////////////////////////////////////////
//  八阶调制 CH3
/////////////////////////////////////////////////////////////////////////

void AD9959::AD9959_SetFremodulation83(double f1, double f2, double f3, double f4, double f5, double f6, double f7, double f8)
{
	// 	 u8 a = 0;
	uchar ChannelFrequencyTuningWord0data[4];
	uchar ChannelFrequencyTuningWord1data[4];
	uchar ChannelFrequencyTuningWord2data[4];
	uchar ChannelFrequencyTuningWord3data[4];
	uchar ChannelFrequencyTuningWord4data[4];
	uchar ChannelFrequencyTuningWord5data[4];
	uchar ChannelFrequencyTuningWord6data[4];
	uchar ChannelFrequencyTuningWord7data[4];
	uchar ChannelFunctionRegisterdata[3] = {0x80, 0x23, 0x30}; // 无RU/RD
	uchar FunctionRegister1data[3] = {0xD0, 0xf2, 0x00};

	writeToAD9959ViaSpi(FR1, 3, FunctionRegister1data, 0);		 // 设置功能寄存器
	writeToAD9959ViaSpi(CFR, 3, ChannelFunctionRegisterdata, 0); // 设置通道功能寄存器

	writeFrequencyTuningWord(f1, ChannelFrequencyTuningWord0data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x04, 4, ChannelFrequencyTuningWord0data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f2, ChannelFrequencyTuningWord1data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x0A, 4, ChannelFrequencyTuningWord1data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f3, ChannelFrequencyTuningWord2data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x0B, 4, ChannelFrequencyTuningWord2data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f4, ChannelFrequencyTuningWord3data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x0C, 4, ChannelFrequencyTuningWord3data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f5, ChannelFrequencyTuningWord4data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x0D, 4, ChannelFrequencyTuningWord4data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f6, ChannelFrequencyTuningWord5data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x0E, 4, ChannelFrequencyTuningWord5data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f7, ChannelFrequencyTuningWord6data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x0F, 4, ChannelFrequencyTuningWord6data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f8, ChannelFrequencyTuningWord7data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x10, 4, ChannelFrequencyTuningWord7data, 1); // 写入频率控制字
}
void AD9959::AD9959_SetPhamodulation83(double f, int p1, int p2, int p3, int p4, int p5, int p6, int p7, int p8)
{
	// 	 u8 b = 0;
	uchar ChannelFrequencyTuningWord0data[4];
	uchar ChannelPhaseOffsetTuningWord0data[4];
	uchar ChannelPhaseOffsetTuningWord1data[4];
	uchar ChannelPhaseOffsetTuningWord2data[4];
	uchar ChannelPhaseOffsetTuningWord3data[4];
	uchar ChannelPhaseOffsetTuningWord4data[4];
	uchar ChannelPhaseOffsetTuningWord5data[4];
	uchar ChannelPhaseOffsetTuningWord6data[4];
	uchar ChannelPhaseOffsetTuningWord7data[4];

	uchar ChannelFunctionRegisterdata[3] = {0xc0, 0x03, 0x30}; // 相位调制模式启用（连续扫描可能开启）
//	uchar AmplitudeControldata[3] = {0x00, 0x03, 0xff};		   // 八级调制时启用，默认输出最大
	uchar FunctionRegister1data[3] = {0xD0, 0xf2, 0x00};

	writeToAD9959ViaSpi(FR1, 3, FunctionRegister1data, 0);		 // 设置功能寄存器
	writeToAD9959ViaSpi(CFR, 3, ChannelFunctionRegisterdata, 0); // 设置通道功能寄存器，即相位调制启动

	writePhaseOffsetTuningWord(p1, ChannelPhaseOffsetTuningWord0data);	 // 将十进制相位数转换为2进制相位控制字
	writeToAD9959ViaSpi(CPOW0, 2, ChannelPhaseOffsetTuningWord0data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p2, ChannelPhaseOffsetTuningWord1data); // 将十进制相位数转换为2进制相位控制字�
	writeToAD9959ViaSpi(0x0a, 4, ChannelPhaseOffsetTuningWord1data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p3, ChannelPhaseOffsetTuningWord2data); // 将十进制相位数转换为2进制相位控制字
	writeToAD9959ViaSpi(0x0b, 4, ChannelPhaseOffsetTuningWord2data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p4, ChannelPhaseOffsetTuningWord3data); // 将十进制相位数转换为2进制相位控制字�
	writeToAD9959ViaSpi(0x0c, 4, ChannelPhaseOffsetTuningWord3data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p5, ChannelPhaseOffsetTuningWord4data); // 将十进制相位数转换为2进制相位控制字
	writeToAD9959ViaSpi(0x0d, 4, ChannelPhaseOffsetTuningWord4data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p6, ChannelPhaseOffsetTuningWord5data); // 将十进制相位数转换为2进制相位控制字�
	writeToAD9959ViaSpi(0x0e, 4, ChannelPhaseOffsetTuningWord5data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p7, ChannelPhaseOffsetTuningWord6data); // 将十进制相位数转换为2进制相位控制字
	writeToAD9959ViaSpi(0x0f, 4, ChannelPhaseOffsetTuningWord6data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p8, ChannelPhaseOffsetTuningWord7data); // 将十进制相位数转换为2进制相位控制字�
	writeToAD9959ViaSpi(0x10, 4, ChannelPhaseOffsetTuningWord7data, 1); // 写入相位控制字
	f = 200000;
	writeFrequencyTuningWord(f, ChannelFrequencyTuningWord0data);	   // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(CFTW0, 4, ChannelFrequencyTuningWord0data, 1); // 写入频率控制字
}
void AD9959::AD9959_SetAM83(double f)
{
	// 	 u8 cn = 0;
	uchar ChannelFrequencyTuningWord0data[4];

	uchar AmplitudeControldata[3] = {0x00, 0x03, 0xff};
	uchar ChannelAmplitudeTuningWord1data[4] = {0xdf, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord2data[4] = {0xbf, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord3data[4] = {0x9f, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord4data[4] = {0x7f, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord5data[4] = {0x5f, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord6data[4] = {0x3f, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord7data[4] = {0x1f, 0x30, 0x00, 0x00};

	uchar ChannelFunctionRegisterdata[3] = {0x40, 0x03, 0x30};
	uchar FunctionRegister1data[3] = {0xD0, 0xf2, 0x00};

	writeToAD9959ViaSpi(FR1, 3, FunctionRegister1data, 0);		 // 设置功能寄存器
	writeToAD9959ViaSpi(CFR, 3, ChannelFunctionRegisterdata, 0); // 设置通道功能寄存器

	writeToAD9959ViaSpi(ACR, 3, AmplitudeControldata, 0); // 设置幅度大小（S1）

	writeToAD9959ViaSpi(0x0a, 4, ChannelAmplitudeTuningWord1data, 0); // 设置幅度大小（S2）

	writeToAD9959ViaSpi(0x0b, 4, ChannelAmplitudeTuningWord2data, 0);

	writeToAD9959ViaSpi(0x0c, 4, ChannelAmplitudeTuningWord3data, 0);

	writeToAD9959ViaSpi(0x0d, 4, ChannelAmplitudeTuningWord4data, 0); // 设置幅度大小（S1）

	writeToAD9959ViaSpi(0x0e, 4, ChannelAmplitudeTuningWord5data, 0); // 设置幅度大小（S2）

	writeToAD9959ViaSpi(0x0f, 4, ChannelAmplitudeTuningWord6data, 0);

	writeToAD9959ViaSpi(0x10, 4, ChannelAmplitudeTuningWord7data, 0);

	writeFrequencyTuningWord(f, ChannelFrequencyTuningWord0data); // 写频率控制字
	writeToAD9959ViaSpi(CFTW0, 4, ChannelFrequencyTuningWord0data, 1);
}

/////////////////////////////////////////////////////////////////////////
//  十六阶调制 CH0
/////////////////////////////////////////////////////////////////////////
void AD9959::AD9959_SetFremodulation160(double f1, double f2, double f3, double f4, double f5, double f6, double f7, double f8, double f9, double f10, double f11, double f12, double f13, double f14, double f15, double f16)
{
	// 	 u8 a = 0;
	uchar ChannelFrequencyTuningWord0data[4];
	uchar ChannelFrequencyTuningWord1data[4];
	uchar ChannelFrequencyTuningWord2data[4];
	uchar ChannelFrequencyTuningWord3data[4];
	uchar ChannelFrequencyTuningWord4data[4];
	uchar ChannelFrequencyTuningWord5data[4];
	uchar ChannelFrequencyTuningWord6data[4];
	uchar ChannelFrequencyTuningWord7data[4];
	uchar ChannelFrequencyTuningWord8data[4];
	uchar ChannelFrequencyTuningWord9data[4];
	uchar ChannelFrequencyTuningWord10data[4];
	uchar ChannelFrequencyTuningWord11data[4];
	uchar ChannelFrequencyTuningWord12data[4];
	uchar ChannelFrequencyTuningWord13data[4];
	uchar ChannelFrequencyTuningWord14data[4];
	uchar ChannelFrequencyTuningWord15data[4];
	uchar ChannelFunctionRegisterdata[3] = {0x80, 0x23, 0x30}; // 无RU/RD
	uchar FunctionRegister1data[3] = {0xD0, 0xc3, 0x00};

	writeToAD9959ViaSpi(FR1, 3, FunctionRegister1data, 0);		 // 设置功能寄存器
	writeToAD9959ViaSpi(CFR, 3, ChannelFunctionRegisterdata, 0); // 设置通道功能寄存器

	writeFrequencyTuningWord(f1, ChannelFrequencyTuningWord0data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x04, 4, ChannelFrequencyTuningWord0data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f2, ChannelFrequencyTuningWord1data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x0A, 4, ChannelFrequencyTuningWord1data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f3, ChannelFrequencyTuningWord2data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x0B, 4, ChannelFrequencyTuningWord2data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f4, ChannelFrequencyTuningWord3data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x0C, 4, ChannelFrequencyTuningWord3data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f5, ChannelFrequencyTuningWord4data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x0D, 4, ChannelFrequencyTuningWord4data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f6, ChannelFrequencyTuningWord5data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x0E, 4, ChannelFrequencyTuningWord5data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f7, ChannelFrequencyTuningWord6data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x0F, 4, ChannelFrequencyTuningWord6data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f8, ChannelFrequencyTuningWord7data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x10, 4, ChannelFrequencyTuningWord7data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f9, ChannelFrequencyTuningWord8data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x11, 4, ChannelFrequencyTuningWord8data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f10, ChannelFrequencyTuningWord9data);  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x12, 4, ChannelFrequencyTuningWord9data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f11, ChannelFrequencyTuningWord10data);  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x13, 4, ChannelFrequencyTuningWord10data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f12, ChannelFrequencyTuningWord11data);  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x14, 4, ChannelFrequencyTuningWord11data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f13, ChannelFrequencyTuningWord12data);  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x15, 4, ChannelFrequencyTuningWord12data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f14, ChannelFrequencyTuningWord13data);  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x16, 4, ChannelFrequencyTuningWord13data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f15, ChannelFrequencyTuningWord14data);	// 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x017, 4, ChannelFrequencyTuningWord14data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f16, ChannelFrequencyTuningWord15data);  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x18, 4, ChannelFrequencyTuningWord15data, 1); // 写入频率控制字
}
void AD9959::AD9959_SetPhamodulation160(double f, int p1, int p2, int p3, int p4, int p5, int p6, int p7, int p8, int p9, int p10, int p11, int p12, int p13, int p14, int p15, int p16)
{
	// 	 u8 b = 0;
	uchar ChannelFrequencyTuningWord0data[4];
	uchar ChannelPhaseOffsetTuningWord0data[4];
	uchar ChannelPhaseOffsetTuningWord1data[4];
	uchar ChannelPhaseOffsetTuningWord2data[4];
	uchar ChannelPhaseOffsetTuningWord3data[4];
	uchar ChannelPhaseOffsetTuningWord4data[4];
	uchar ChannelPhaseOffsetTuningWord5data[4];
	uchar ChannelPhaseOffsetTuningWord6data[4];
	uchar ChannelPhaseOffsetTuningWord7data[4];
	uchar ChannelPhaseOffsetTuningWord8data[4];
	uchar ChannelPhaseOffsetTuningWord9data[4];
	uchar ChannelPhaseOffsetTuningWord10data[4];
	uchar ChannelPhaseOffsetTuningWord11data[4];
	uchar ChannelPhaseOffsetTuningWord12data[4];
	uchar ChannelPhaseOffsetTuningWord13data[4];
	uchar ChannelPhaseOffsetTuningWord14data[4];
	uchar ChannelPhaseOffsetTuningWord15data[4];

	uchar ChannelFunctionRegisterdata[3] = {0xc0, 0x03, 0x30}; // 相位调制模式启用（连续扫描可能开启）
//	uchar AmplitudeControldata[3] = {0x00, 0x03, 0xff};		   // 十六级调制时启用，默认输出最大
	uchar FunctionRegister1data[3] = {0xD0, 0xc3, 0x00};

	writeToAD9959ViaSpi(FR1, 3, FunctionRegister1data, 0);		 // 设置功能寄存器
	writeToAD9959ViaSpi(CFR, 3, ChannelFunctionRegisterdata, 0); // 设置通道功能寄存器，即相位调制启动

	writePhaseOffsetTuningWord(p1, ChannelPhaseOffsetTuningWord0data);	 // 将十进制相位数转换为2进制相位控制字
	writeToAD9959ViaSpi(CPOW0, 2, ChannelPhaseOffsetTuningWord0data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p2, ChannelPhaseOffsetTuningWord1data); // 将十进制相位数转换为2进制相位控制字�
	writeToAD9959ViaSpi(0x0a, 4, ChannelPhaseOffsetTuningWord1data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p3, ChannelPhaseOffsetTuningWord2data); // 将十进制相位数转换为2进制相位控制字
	writeToAD9959ViaSpi(0x0b, 4, ChannelPhaseOffsetTuningWord2data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p4, ChannelPhaseOffsetTuningWord3data); // 将十进制相位数转换为2进制相位控制字�
	writeToAD9959ViaSpi(0x0c, 4, ChannelPhaseOffsetTuningWord3data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p5, ChannelPhaseOffsetTuningWord4data); // 将十进制相位数转换为2进制相位控制字
	writeToAD9959ViaSpi(0x0d, 4, ChannelPhaseOffsetTuningWord4data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p6, ChannelPhaseOffsetTuningWord5data); // 将十进制相位数转换为2进制相位控制字�
	writeToAD9959ViaSpi(0x0e, 4, ChannelPhaseOffsetTuningWord5data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p7, ChannelPhaseOffsetTuningWord6data); // 将十进制相位数转换为2进制相位控制字
	writeToAD9959ViaSpi(0x0f, 4, ChannelPhaseOffsetTuningWord6data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p8, ChannelPhaseOffsetTuningWord7data); // 将十进制相位数转换为2进制相位控制字�
	writeToAD9959ViaSpi(0x10, 4, ChannelPhaseOffsetTuningWord7data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p9, ChannelPhaseOffsetTuningWord8data); // 将十进制相位数转换为2进制相位控制字
	writeToAD9959ViaSpi(0x11, 4, ChannelPhaseOffsetTuningWord8data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p10, ChannelPhaseOffsetTuningWord9data); // 将十进制相位数转换为2进制相位控制字�
	writeToAD9959ViaSpi(0x12, 4, ChannelPhaseOffsetTuningWord9data, 1);	 // 写入相位控制字

	writePhaseOffsetTuningWord(p11, ChannelPhaseOffsetTuningWord10data); // 将十进制相位数转换为2进制相位控制字
	writeToAD9959ViaSpi(0x13, 4, ChannelPhaseOffsetTuningWord10data, 1);  // 写入相位控制字

	writePhaseOffsetTuningWord(p12, ChannelPhaseOffsetTuningWord11data); // 将十进制相位数转换为2进制相位控制字�
	writeToAD9959ViaSpi(0x14, 4, ChannelPhaseOffsetTuningWord11data, 1);  // 写入相位控制字

	writePhaseOffsetTuningWord(p13, ChannelPhaseOffsetTuningWord12data); // 将十进制相位数转换为2进制相位控制字
	writeToAD9959ViaSpi(0x15, 4, ChannelPhaseOffsetTuningWord12data, 1);  // 写入相位控制字

	writePhaseOffsetTuningWord(p14, ChannelPhaseOffsetTuningWord13data); // 将十进制相位数转换为2进制相位控制字�
	writeToAD9959ViaSpi(0x16, 4, ChannelPhaseOffsetTuningWord13data, 1);  // 写入相位控制字

	writePhaseOffsetTuningWord(p15, ChannelPhaseOffsetTuningWord14data); // 将十进制相位数转换为2进制相位控制字
	writeToAD9959ViaSpi(0x17, 4, ChannelPhaseOffsetTuningWord14data, 1);  // 写入相位控制字

	writePhaseOffsetTuningWord(p16, ChannelPhaseOffsetTuningWord15data); // 将十进制相位数转换为2进制相位控制字
	writeToAD9959ViaSpi(0x18, 4, ChannelPhaseOffsetTuningWord15data, 1);  // 写入相位控制字

	f = 200000;
	writeFrequencyTuningWord(f, ChannelFrequencyTuningWord0data);	   // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(CFTW0, 4, ChannelFrequencyTuningWord0data, 1); // 写入频率控制字
}
void AD9959::AD9959_SetAM160(double f)
{
	// 	 u8 cn = 0;
	uchar ChannelFrequencyTuningWord0data[4];

	uchar AmplitudeControldata[3] = {0x00, 0x03, 0xff};
	uchar ChannelAmplitudeTuningWord1data[4] = {0xef, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord2data[4] = {0xdf, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord3data[4] = {0xcf, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord4data[4] = {0xbf, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord5data[4] = {0xaf, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord6data[4] = {0x9f, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord7data[4] = {0x8f, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord8data[4] = {0x7f, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord9data[4] = {0x6f, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord10data[4] = {0x5f, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord11data[4] = {0x4f, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord12data[4] = {0x3f, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord13data[4] = {0x2f, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord14data[4] = {0x1f, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord15data[4] = {0x0f, 0x30, 0x00, 0x00};
	uchar ChannelFunctionRegisterdata[3] = {0x40, 0x03, 0x30};
	uchar FunctionRegister1data[3] = {0xD0, 0xc3, 0x00};

	writeToAD9959ViaSpi(FR1, 3, FunctionRegister1data, 0);		 // 设置功能寄存器
	writeToAD9959ViaSpi(CFR, 3, ChannelFunctionRegisterdata, 0); // 设置通道功能寄存器

	writeToAD9959ViaSpi(ACR, 3, AmplitudeControldata, 0); // 设置幅度大小（S1）

	writeToAD9959ViaSpi(0x0a, 4, ChannelAmplitudeTuningWord1data, 0); // 设置幅度大小（S2）

	writeToAD9959ViaSpi(0x0b, 4, ChannelAmplitudeTuningWord2data, 0);

	writeToAD9959ViaSpi(0x0c, 4, ChannelAmplitudeTuningWord3data, 0);

	writeToAD9959ViaSpi(0x0d, 4, ChannelAmplitudeTuningWord4data, 0); // 设置幅度大小（S1）

	writeToAD9959ViaSpi(0x0e, 4, ChannelAmplitudeTuningWord5data, 0); // 设置幅度大小（S2）

	writeToAD9959ViaSpi(0x0f, 4, ChannelAmplitudeTuningWord6data, 0);

	writeToAD9959ViaSpi(0x10, 4, ChannelAmplitudeTuningWord7data, 0);

	writeToAD9959ViaSpi(0x11, 4, ChannelAmplitudeTuningWord8data, 0); // 设置幅度大小（S1）

	writeToAD9959ViaSpi(0x12, 4, ChannelAmplitudeTuningWord9data, 0); // 设置幅度大小（S2）

	writeToAD9959ViaSpi(0x13, 4, ChannelAmplitudeTuningWord10data, 0);

	writeToAD9959ViaSpi(0x14, 4, ChannelAmplitudeTuningWord11data, 0);

	writeToAD9959ViaSpi(0x15, 4, ChannelAmplitudeTuningWord12data, 0); // 设置幅度大小（S1）

	writeToAD9959ViaSpi(0x16, 4, ChannelAmplitudeTuningWord13data, 0); // 设置幅度大小（S2）

	writeToAD9959ViaSpi(0x17, 4, ChannelAmplitudeTuningWord14data, 0);

	writeToAD9959ViaSpi(0x18, 4, ChannelAmplitudeTuningWord15data, 0);

	writeFrequencyTuningWord(f, ChannelFrequencyTuningWord0data); // 写频率控制字
	writeToAD9959ViaSpi(CFTW0, 4, ChannelFrequencyTuningWord0data, 1);
}
/////////////////////////////////////////////////////////////////////////
//  十六阶调制 CH1
/////////////////////////////////////////////////////////////////////////
void AD9959::AD9959_SetFremodulation161(double f1, double f2, double f3, double f4, double f5, double f6, double f7, double f8, double f9, double f10, double f11, double f12, double f13, double f14, double f15, double f16)
{
	// 	 u8 a = 0;
	uchar ChannelFrequencyTuningWord0data[4];
	uchar ChannelFrequencyTuningWord1data[4];
	uchar ChannelFrequencyTuningWord2data[4];
	uchar ChannelFrequencyTuningWord3data[4];
	uchar ChannelFrequencyTuningWord4data[4];
	uchar ChannelFrequencyTuningWord5data[4];
	uchar ChannelFrequencyTuningWord6data[4];
	uchar ChannelFrequencyTuningWord7data[4];
	uchar ChannelFrequencyTuningWord8data[4];
	uchar ChannelFrequencyTuningWord9data[4];
	uchar ChannelFrequencyTuningWord10data[4];
	uchar ChannelFrequencyTuningWord11data[4];
	uchar ChannelFrequencyTuningWord12data[4];
	uchar ChannelFrequencyTuningWord13data[4];
	uchar ChannelFrequencyTuningWord14data[4];
	uchar ChannelFrequencyTuningWord15data[4];
	uchar ChannelFunctionRegisterdata[3] = {0x80, 0x23, 0x30}; // 无RU/RD
	uchar FunctionRegister1data[3] = {0xD0, 0xd3, 0x00};

	writeToAD9959ViaSpi(FR1, 3, FunctionRegister1data, 0);		 // 设置功能寄存器
	writeToAD9959ViaSpi(CFR, 3, ChannelFunctionRegisterdata, 0); // 设置通道功能寄存器

	writeFrequencyTuningWord(f1, ChannelFrequencyTuningWord0data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x04, 4, ChannelFrequencyTuningWord0data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f2, ChannelFrequencyTuningWord1data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x0A, 4, ChannelFrequencyTuningWord1data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f3, ChannelFrequencyTuningWord2data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x0B, 4, ChannelFrequencyTuningWord2data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f4, ChannelFrequencyTuningWord3data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x0C, 4, ChannelFrequencyTuningWord3data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f5, ChannelFrequencyTuningWord4data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x0D, 4, ChannelFrequencyTuningWord4data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f6, ChannelFrequencyTuningWord5data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x0E, 4, ChannelFrequencyTuningWord5data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f7, ChannelFrequencyTuningWord6data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x0F, 4, ChannelFrequencyTuningWord6data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f8, ChannelFrequencyTuningWord7data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x10, 4, ChannelFrequencyTuningWord7data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f9, ChannelFrequencyTuningWord8data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x11, 4, ChannelFrequencyTuningWord8data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f10, ChannelFrequencyTuningWord9data);  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x12, 4, ChannelFrequencyTuningWord9data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f11, ChannelFrequencyTuningWord10data);  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x13, 4, ChannelFrequencyTuningWord10data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f12, ChannelFrequencyTuningWord11data);  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x14, 4, ChannelFrequencyTuningWord11data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f13, ChannelFrequencyTuningWord12data);  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x15, 4, ChannelFrequencyTuningWord12data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f14, ChannelFrequencyTuningWord13data);  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x16, 4, ChannelFrequencyTuningWord13data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f15, ChannelFrequencyTuningWord14data);	// 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x017, 4, ChannelFrequencyTuningWord14data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f16, ChannelFrequencyTuningWord15data);  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x18, 4, ChannelFrequencyTuningWord15data, 1); // 写入频率控制字
}
void AD9959::AD9959_SetPhamodulation161(double f, int p1, int p2, int p3, int p4, int p5, int p6, int p7, int p8, int p9, int p10, int p11, int p12, int p13, int p14, int p15, int p16)
{
	// 	 u8 b = 0;
	uchar ChannelFrequencyTuningWord0data[4];
	uchar ChannelPhaseOffsetTuningWord0data[4];
	uchar ChannelPhaseOffsetTuningWord1data[4];
	uchar ChannelPhaseOffsetTuningWord2data[4];
	uchar ChannelPhaseOffsetTuningWord3data[4];
	uchar ChannelPhaseOffsetTuningWord4data[4];
	uchar ChannelPhaseOffsetTuningWord5data[4];
	uchar ChannelPhaseOffsetTuningWord6data[4];
	uchar ChannelPhaseOffsetTuningWord7data[4];
	uchar ChannelPhaseOffsetTuningWord8data[4];
	uchar ChannelPhaseOffsetTuningWord9data[4];
	uchar ChannelPhaseOffsetTuningWord10data[4];
	uchar ChannelPhaseOffsetTuningWord11data[4];
	uchar ChannelPhaseOffsetTuningWord12data[4];
	uchar ChannelPhaseOffsetTuningWord13data[4];
	uchar ChannelPhaseOffsetTuningWord14data[4];
	uchar ChannelPhaseOffsetTuningWord15data[4];

	uchar ChannelFunctionRegisterdata[3] = {0xc0, 0x03, 0x30}; // 相位调制模式启用（连续扫描可能开启）
//	uchar AmplitudeControldata[3] = {0x00, 0x03, 0xff};		   // 十六级调制时启用，默认输出最大
	uchar FunctionRegister1data[3] = {0xD0, 0xd3, 0x00};

	writeToAD9959ViaSpi(FR1, 3, FunctionRegister1data, 0);		 // 设置功能寄存器
	writeToAD9959ViaSpi(CFR, 3, ChannelFunctionRegisterdata, 0); // 设置通道功能寄存器，即相位调制启动

	writePhaseOffsetTuningWord(p1, ChannelPhaseOffsetTuningWord0data);	 // 将十进制相位数转换为2进制相位控制字
	writeToAD9959ViaSpi(CPOW0, 2, ChannelPhaseOffsetTuningWord0data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p2, ChannelPhaseOffsetTuningWord1data); // 将十进制相位数转换为2进制相位控制字�
	writeToAD9959ViaSpi(0x0a, 4, ChannelPhaseOffsetTuningWord1data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p3, ChannelPhaseOffsetTuningWord2data); // 将十进制相位数转换为2进制相位控制字
	writeToAD9959ViaSpi(0x0b, 4, ChannelPhaseOffsetTuningWord2data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p4, ChannelPhaseOffsetTuningWord3data); // 将十进制相位数转换为2进制相位控制字�
	writeToAD9959ViaSpi(0x0c, 4, ChannelPhaseOffsetTuningWord3data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p5, ChannelPhaseOffsetTuningWord4data); // 将十进制相位数转换为2进制相位控制字
	writeToAD9959ViaSpi(0x0d, 4, ChannelPhaseOffsetTuningWord4data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p6, ChannelPhaseOffsetTuningWord5data); // 将十进制相位数转换为2进制相位控制字�
	writeToAD9959ViaSpi(0x0e, 4, ChannelPhaseOffsetTuningWord5data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p7, ChannelPhaseOffsetTuningWord6data); // 将十进制相位数转换为2进制相位控制字
	writeToAD9959ViaSpi(0x0f, 4, ChannelPhaseOffsetTuningWord6data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p8, ChannelPhaseOffsetTuningWord7data); // 将十进制相位数转换为2进制相位控制字�
	writeToAD9959ViaSpi(0x10, 4, ChannelPhaseOffsetTuningWord7data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p9, ChannelPhaseOffsetTuningWord8data); // 将十进制相位数转换为2进制相位控制字
	writeToAD9959ViaSpi(0x11, 4, ChannelPhaseOffsetTuningWord8data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p10, ChannelPhaseOffsetTuningWord9data); // 将十进制相位数转换为2进制相位控制字�
	writeToAD9959ViaSpi(0x12, 4, ChannelPhaseOffsetTuningWord9data, 1);	 // 写入相位控制字

	writePhaseOffsetTuningWord(p11, ChannelPhaseOffsetTuningWord10data); // 将十进制相位数转换为2进制相位控制字
	writeToAD9959ViaSpi(0x13, 4, ChannelPhaseOffsetTuningWord10data, 1);  // 写入相位控制字

	writePhaseOffsetTuningWord(p12, ChannelPhaseOffsetTuningWord11data); // 将十进制相位数转换为2进制相位控制字�
	writeToAD9959ViaSpi(0x14, 4, ChannelPhaseOffsetTuningWord11data, 1);  // 写入相位控制字

	writePhaseOffsetTuningWord(p13, ChannelPhaseOffsetTuningWord12data); // 将十进制相位数转换为2进制相位控制字
	writeToAD9959ViaSpi(0x15, 4, ChannelPhaseOffsetTuningWord12data, 1);  // 写入相位控制字

	writePhaseOffsetTuningWord(p14, ChannelPhaseOffsetTuningWord13data); // 将十进制相位数转换为2进制相位控制字�
	writeToAD9959ViaSpi(0x16, 4, ChannelPhaseOffsetTuningWord13data, 1);  // 写入相位控制字

	writePhaseOffsetTuningWord(p15, ChannelPhaseOffsetTuningWord14data); // 将十进制相位数转换为2进制相位控制字
	writeToAD9959ViaSpi(0x17, 4, ChannelPhaseOffsetTuningWord14data, 1);  // 写入相位控制字

	writePhaseOffsetTuningWord(p16, ChannelPhaseOffsetTuningWord15data); // 将十进制相位数转换为2进制相位控制字
	writeToAD9959ViaSpi(0x18, 4, ChannelPhaseOffsetTuningWord15data, 1);  // 写入相位控制字

	f = 200000;
	writeFrequencyTuningWord(f, ChannelFrequencyTuningWord0data);	   // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(CFTW0, 4, ChannelFrequencyTuningWord0data, 1); // 写入频率控制字
}
void AD9959::AD9959_SetAM161(double f)
{
	// 	 u8 cn = 0;
	uchar ChannelFrequencyTuningWord0data[4];

	uchar AmplitudeControldata[3] = {0x00, 0x03, 0xff};
	uchar ChannelAmplitudeTuningWord1data[4] = {0xef, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord2data[4] = {0xdf, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord3data[4] = {0xcf, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord4data[4] = {0xbf, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord5data[4] = {0xaf, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord6data[4] = {0x9f, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord7data[4] = {0x8f, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord8data[4] = {0x7f, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord9data[4] = {0x6f, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord10data[4] = {0x5f, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord11data[4] = {0x4f, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord12data[4] = {0x3f, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord13data[4] = {0x2f, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord14data[4] = {0x1f, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord15data[4] = {0x0f, 0x30, 0x00, 0x00};
	uchar ChannelFunctionRegisterdata[3] = {0x40, 0x03, 0x30};
	uchar FunctionRegister1data[3] = {0xD0, 0xd3, 0x00};

	writeToAD9959ViaSpi(FR1, 3, FunctionRegister1data, 0);		 // 设置功能寄存器
	writeToAD9959ViaSpi(CFR, 3, ChannelFunctionRegisterdata, 0); // 设置通道功能寄存器

	writeToAD9959ViaSpi(ACR, 3, AmplitudeControldata, 0); // 设置幅度大小（S1）

	writeToAD9959ViaSpi(0x0a, 4, ChannelAmplitudeTuningWord1data, 0); // 设置幅度大小（S2）

	writeToAD9959ViaSpi(0x0b, 4, ChannelAmplitudeTuningWord2data, 0);

	writeToAD9959ViaSpi(0x0c, 4, ChannelAmplitudeTuningWord3data, 0);

	writeToAD9959ViaSpi(0x0d, 4, ChannelAmplitudeTuningWord4data, 0); // 设置幅度大小（S1）

	writeToAD9959ViaSpi(0x0e, 4, ChannelAmplitudeTuningWord5data, 0); // 设置幅度大小（S2）

	writeToAD9959ViaSpi(0x0f, 4, ChannelAmplitudeTuningWord6data, 0);

	writeToAD9959ViaSpi(0x10, 4, ChannelAmplitudeTuningWord7data, 0);

	writeToAD9959ViaSpi(0x11, 4, ChannelAmplitudeTuningWord8data, 0); // 设置幅度大小（S1）

	writeToAD9959ViaSpi(0x12, 4, ChannelAmplitudeTuningWord9data, 0); // 设置幅度大小（S2）

	writeToAD9959ViaSpi(0x13, 4, ChannelAmplitudeTuningWord10data, 0);

	writeToAD9959ViaSpi(0x14, 4, ChannelAmplitudeTuningWord11data, 0);

	writeToAD9959ViaSpi(0x15, 4, ChannelAmplitudeTuningWord12data, 0); // 设置幅度大小（S1）

	writeToAD9959ViaSpi(0x16, 4, ChannelAmplitudeTuningWord13data, 0); // 设置幅度大小（S2）

	writeToAD9959ViaSpi(0x17, 4, ChannelAmplitudeTuningWord14data, 0);

	writeToAD9959ViaSpi(0x18, 4, ChannelAmplitudeTuningWord15data, 0);

	writeFrequencyTuningWord(f, ChannelFrequencyTuningWord0data); // 写频率控制字
	writeToAD9959ViaSpi(CFTW0, 4, ChannelFrequencyTuningWord0data, 1);
}

/////////////////////////////////////////////////////////////////////////
//  十六阶调制 CH2
/////////////////////////////////////////////////////////////////////////
void AD9959::AD9959_SetFremodulation162(double f1, double f2, double f3, double f4, double f5, double f6, double f7, double f8, double f9, double f10, double f11, double f12, double f13, double f14, double f15, double f16)
{
	// 	 u8 a = 0;
	uchar ChannelFrequencyTuningWord0data[4];
	uchar ChannelFrequencyTuningWord1data[4];
	uchar ChannelFrequencyTuningWord2data[4];
	uchar ChannelFrequencyTuningWord3data[4];
	uchar ChannelFrequencyTuningWord4data[4];
	uchar ChannelFrequencyTuningWord5data[4];
	uchar ChannelFrequencyTuningWord6data[4];
	uchar ChannelFrequencyTuningWord7data[4];
	uchar ChannelFrequencyTuningWord8data[4];
	uchar ChannelFrequencyTuningWord9data[4];
	uchar ChannelFrequencyTuningWord10data[4];
	uchar ChannelFrequencyTuningWord11data[4];
	uchar ChannelFrequencyTuningWord12data[4];
	uchar ChannelFrequencyTuningWord13data[4];
	uchar ChannelFrequencyTuningWord14data[4];
	uchar ChannelFrequencyTuningWord15data[4];
	uchar ChannelFunctionRegisterdata[3] = {0x80, 0x23, 0x30}; // 无RU/RD
	uchar FunctionRegister1data[3] = {0xD0, 0xe3, 0x00};

	writeToAD9959ViaSpi(FR1, 3, FunctionRegister1data, 0);		 // 设置功能寄存器
	writeToAD9959ViaSpi(CFR, 3, ChannelFunctionRegisterdata, 0); // 设置通道功能寄存器

	writeFrequencyTuningWord(f1, ChannelFrequencyTuningWord0data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x04, 4, ChannelFrequencyTuningWord0data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f2, ChannelFrequencyTuningWord1data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x0A, 4, ChannelFrequencyTuningWord1data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f3, ChannelFrequencyTuningWord2data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x0B, 4, ChannelFrequencyTuningWord2data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f4, ChannelFrequencyTuningWord3data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x0C, 4, ChannelFrequencyTuningWord3data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f5, ChannelFrequencyTuningWord4data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x0D, 4, ChannelFrequencyTuningWord4data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f6, ChannelFrequencyTuningWord5data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x0E, 4, ChannelFrequencyTuningWord5data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f7, ChannelFrequencyTuningWord6data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x0F, 4, ChannelFrequencyTuningWord6data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f8, ChannelFrequencyTuningWord7data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x10, 4, ChannelFrequencyTuningWord7data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f9, ChannelFrequencyTuningWord8data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x11, 4, ChannelFrequencyTuningWord8data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f10, ChannelFrequencyTuningWord9data);  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x12, 4, ChannelFrequencyTuningWord9data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f11, ChannelFrequencyTuningWord10data);  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x13, 4, ChannelFrequencyTuningWord10data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f12, ChannelFrequencyTuningWord11data);  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x14, 4, ChannelFrequencyTuningWord11data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f13, ChannelFrequencyTuningWord12data);  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x15, 4, ChannelFrequencyTuningWord12data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f14, ChannelFrequencyTuningWord13data);  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x16, 4, ChannelFrequencyTuningWord13data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f15, ChannelFrequencyTuningWord14data);	// 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x017, 4, ChannelFrequencyTuningWord14data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f16, ChannelFrequencyTuningWord15data);  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x18, 4, ChannelFrequencyTuningWord15data, 1); // 写入频率控制字
}
void AD9959::AD9959_SetPhamodulation162(double f, int p1, int p2, int p3, int p4, int p5, int p6, int p7, int p8, int p9, int p10, int p11, int p12, int p13, int p14, int p15, int p16)
{
	// 	 u8 b = 0;
	uchar ChannelFrequencyTuningWord0data[4];
	uchar ChannelPhaseOffsetTuningWord0data[4];
	uchar ChannelPhaseOffsetTuningWord1data[4];
	uchar ChannelPhaseOffsetTuningWord2data[4];
	uchar ChannelPhaseOffsetTuningWord3data[4];
	uchar ChannelPhaseOffsetTuningWord4data[4];
	uchar ChannelPhaseOffsetTuningWord5data[4];
	uchar ChannelPhaseOffsetTuningWord6data[4];
	uchar ChannelPhaseOffsetTuningWord7data[4];
	uchar ChannelPhaseOffsetTuningWord8data[4];
	uchar ChannelPhaseOffsetTuningWord9data[4];
	uchar ChannelPhaseOffsetTuningWord10data[4];
	uchar ChannelPhaseOffsetTuningWord11data[4];
	uchar ChannelPhaseOffsetTuningWord12data[4];
	uchar ChannelPhaseOffsetTuningWord13data[4];
	uchar ChannelPhaseOffsetTuningWord14data[4];
	uchar ChannelPhaseOffsetTuningWord15data[4];

	uchar ChannelFunctionRegisterdata[3] = {0xc0, 0x03, 0x30}; // 相位调制模式启用（连续扫描可能开启）
//	uchar AmplitudeControldata[3] = {0x00, 0x03, 0xff};		   // 十六级调制时启用，默认输出最大
	uchar FunctionRegister1data[3] = {0xD0, 0xe3, 0x00};

	writeToAD9959ViaSpi(FR1, 3, FunctionRegister1data, 0);		 // 设置功能寄存器
	writeToAD9959ViaSpi(CFR, 3, ChannelFunctionRegisterdata, 0); // 设置通道功能寄存器，即相位调制启动

	writePhaseOffsetTuningWord(p1, ChannelPhaseOffsetTuningWord0data);	 // 将十进制相位数转换为2进制相位控制字
	writeToAD9959ViaSpi(CPOW0, 2, ChannelPhaseOffsetTuningWord0data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p2, ChannelPhaseOffsetTuningWord1data); // 将十进制相位数转换为2进制相位控制字�
	writeToAD9959ViaSpi(0x0a, 4, ChannelPhaseOffsetTuningWord1data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p3, ChannelPhaseOffsetTuningWord2data); // 将十进制相位数转换为2进制相位控制字
	writeToAD9959ViaSpi(0x0b, 4, ChannelPhaseOffsetTuningWord2data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p4, ChannelPhaseOffsetTuningWord3data); // 将十进制相位数转换为2进制相位控制字�
	writeToAD9959ViaSpi(0x0c, 4, ChannelPhaseOffsetTuningWord3data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p5, ChannelPhaseOffsetTuningWord4data); // 将十进制相位数转换为2进制相位控制字
	writeToAD9959ViaSpi(0x0d, 4, ChannelPhaseOffsetTuningWord4data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p6, ChannelPhaseOffsetTuningWord5data); // 将十进制相位数转换为2进制相位控制字�
	writeToAD9959ViaSpi(0x0e, 4, ChannelPhaseOffsetTuningWord5data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p7, ChannelPhaseOffsetTuningWord6data); // 将十进制相位数转换为2进制相位控制字
	writeToAD9959ViaSpi(0x0f, 4, ChannelPhaseOffsetTuningWord6data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p8, ChannelPhaseOffsetTuningWord7data); // 将十进制相位数转换为2进制相位控制字�
	writeToAD9959ViaSpi(0x10, 4, ChannelPhaseOffsetTuningWord7data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p9, ChannelPhaseOffsetTuningWord8data); // 将十进制相位数转换为2进制相位控制字
	writeToAD9959ViaSpi(0x11, 4, ChannelPhaseOffsetTuningWord8data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p10, ChannelPhaseOffsetTuningWord9data); // 将十进制相位数转换为2进制相位控制字�
	writeToAD9959ViaSpi(0x12, 4, ChannelPhaseOffsetTuningWord9data, 1);	 // 写入相位控制字

	writePhaseOffsetTuningWord(p11, ChannelPhaseOffsetTuningWord10data); // 将十进制相位数转换为2进制相位控制字
	writeToAD9959ViaSpi(0x13, 4, ChannelPhaseOffsetTuningWord10data, 1);  // 写入相位控制字

	writePhaseOffsetTuningWord(p12, ChannelPhaseOffsetTuningWord11data); // 将十进制相位数转换为2进制相位控制字�
	writeToAD9959ViaSpi(0x14, 4, ChannelPhaseOffsetTuningWord11data, 1);  // 写入相位控制字

	writePhaseOffsetTuningWord(p13, ChannelPhaseOffsetTuningWord12data); // 将十进制相位数转换为2进制相位控制字
	writeToAD9959ViaSpi(0x15, 4, ChannelPhaseOffsetTuningWord12data, 1);  // 写入相位控制字

	writePhaseOffsetTuningWord(p14, ChannelPhaseOffsetTuningWord13data); // 将十进制相位数转换为2进制相位控制字�
	writeToAD9959ViaSpi(0x16, 4, ChannelPhaseOffsetTuningWord13data, 1);  // 写入相位控制字

	writePhaseOffsetTuningWord(p15, ChannelPhaseOffsetTuningWord14data); // 将十进制相位数转换为2进制相位控制字
	writeToAD9959ViaSpi(0x17, 4, ChannelPhaseOffsetTuningWord14data, 1);  // 写入相位控制字

	writePhaseOffsetTuningWord(p16, ChannelPhaseOffsetTuningWord15data); // 将十进制相位数转换为2进制相位控制字
	writeToAD9959ViaSpi(0x18, 4, ChannelPhaseOffsetTuningWord15data, 1);  // 写入相位控制字

	f = 200000;
	writeFrequencyTuningWord(f, ChannelFrequencyTuningWord0data);	   // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(CFTW0, 4, ChannelFrequencyTuningWord0data, 1); // 写入频率控制字
}
void AD9959::AD9959_SetAM162(double f)
{
	// 	 u8 cn = 0;
	uchar ChannelFrequencyTuningWord0data[4];

	uchar AmplitudeControldata[3] = {0x00, 0x03, 0xff};
	uchar ChannelAmplitudeTuningWord1data[4] = {0xef, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord2data[4] = {0xdf, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord3data[4] = {0xcf, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord4data[4] = {0xbf, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord5data[4] = {0xaf, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord6data[4] = {0x9f, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord7data[4] = {0x8f, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord8data[4] = {0x7f, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord9data[4] = {0x6f, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord10data[4] = {0x5f, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord11data[4] = {0x4f, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord12data[4] = {0x3f, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord13data[4] = {0x2f, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord14data[4] = {0x1f, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord15data[4] = {0x0f, 0x30, 0x00, 0x00};
	uchar ChannelFunctionRegisterdata[3] = {0x40, 0x03, 0x30};
	uchar FunctionRegister1data[3] = {0xD0, 0xe3, 0x00};

	writeToAD9959ViaSpi(FR1, 3, FunctionRegister1data, 0);		 // 设置功能寄存器
	writeToAD9959ViaSpi(CFR, 3, ChannelFunctionRegisterdata, 0); // 设置通道功能寄存器

	writeToAD9959ViaSpi(ACR, 3, AmplitudeControldata, 0); // 设置幅度大小（S1）

	writeToAD9959ViaSpi(0x0a, 4, ChannelAmplitudeTuningWord1data, 0); // 设置幅度大小（S2）

	writeToAD9959ViaSpi(0x0b, 4, ChannelAmplitudeTuningWord2data, 0);

	writeToAD9959ViaSpi(0x0c, 4, ChannelAmplitudeTuningWord3data, 0);

	writeToAD9959ViaSpi(0x0d, 4, ChannelAmplitudeTuningWord4data, 0); // 设置幅度大小（S1）

	writeToAD9959ViaSpi(0x0e, 4, ChannelAmplitudeTuningWord5data, 0); // 设置幅度大小（S2）

	writeToAD9959ViaSpi(0x0f, 4, ChannelAmplitudeTuningWord6data, 0);

	writeToAD9959ViaSpi(0x10, 4, ChannelAmplitudeTuningWord7data, 0);

	writeToAD9959ViaSpi(0x11, 4, ChannelAmplitudeTuningWord8data, 0); // 设置幅度大小（S1）

	writeToAD9959ViaSpi(0x12, 4, ChannelAmplitudeTuningWord9data, 0); // 设置幅度大小（S2）

	writeToAD9959ViaSpi(0x13, 4, ChannelAmplitudeTuningWord10data, 0);

	writeToAD9959ViaSpi(0x14, 4, ChannelAmplitudeTuningWord11data, 0);

	writeToAD9959ViaSpi(0x15, 4, ChannelAmplitudeTuningWord12data, 0); // 设置幅度大小（S1）

	writeToAD9959ViaSpi(0x16, 4, ChannelAmplitudeTuningWord13data, 0); // 设置幅度大小（S2）

	writeToAD9959ViaSpi(0x17, 4, ChannelAmplitudeTuningWord14data, 0);

	writeToAD9959ViaSpi(0x18, 4, ChannelAmplitudeTuningWord15data, 0);

	writeFrequencyTuningWord(f, ChannelFrequencyTuningWord0data); // 写频率控制字
	writeToAD9959ViaSpi(CFTW0, 4, ChannelFrequencyTuningWord0data, 1);
}

/////////////////////////////////////////////////////////////////////////
//  十六阶调制 CH3
/////////////////////////////////////////////////////////////////////////

void AD9959::AD9959_SetFremodulation163(double f1, double f2, double f3, double f4, double f5, double f6, double f7, double f8, double f9, double f10, double f11, double f12, double f13, double f14, double f15, double f16)
{
	// 	 u8 a = 0;
	uchar ChannelFrequencyTuningWord0data[4];
	uchar ChannelFrequencyTuningWord1data[4];
	uchar ChannelFrequencyTuningWord2data[4];
	uchar ChannelFrequencyTuningWord3data[4];
	uchar ChannelFrequencyTuningWord4data[4];
	uchar ChannelFrequencyTuningWord5data[4];
	uchar ChannelFrequencyTuningWord6data[4];
	uchar ChannelFrequencyTuningWord7data[4];
	uchar ChannelFrequencyTuningWord8data[4];
	uchar ChannelFrequencyTuningWord9data[4];
	uchar ChannelFrequencyTuningWord10data[4];
	uchar ChannelFrequencyTuningWord11data[4];
	uchar ChannelFrequencyTuningWord12data[4];
	uchar ChannelFrequencyTuningWord13data[4];
	uchar ChannelFrequencyTuningWord14data[4];
	uchar ChannelFrequencyTuningWord15data[4];
	uchar ChannelFunctionRegisterdata[3] = {0x80, 0x23, 0x30}; // 无RU/RD
	uchar FunctionRegister1data[3] = {0xD0, 0xf3, 0x00};

	writeToAD9959ViaSpi(FR1, 3, FunctionRegister1data, 0);		 // 设置功能寄存器
	writeToAD9959ViaSpi(CFR, 3, ChannelFunctionRegisterdata, 0); // 设置通道功能寄存器

	writeFrequencyTuningWord(f1, ChannelFrequencyTuningWord0data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x04, 4, ChannelFrequencyTuningWord0data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f2, ChannelFrequencyTuningWord1data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x0A, 4, ChannelFrequencyTuningWord1data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f3, ChannelFrequencyTuningWord2data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x0B, 4, ChannelFrequencyTuningWord2data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f4, ChannelFrequencyTuningWord3data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x0C, 4, ChannelFrequencyTuningWord3data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f5, ChannelFrequencyTuningWord4data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x0D, 4, ChannelFrequencyTuningWord4data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f6, ChannelFrequencyTuningWord5data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x0E, 4, ChannelFrequencyTuningWord5data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f7, ChannelFrequencyTuningWord6data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x0F, 4, ChannelFrequencyTuningWord6data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f8, ChannelFrequencyTuningWord7data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x10, 4, ChannelFrequencyTuningWord7data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f9, ChannelFrequencyTuningWord8data);	  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x11, 4, ChannelFrequencyTuningWord8data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f10, ChannelFrequencyTuningWord9data);  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x12, 4, ChannelFrequencyTuningWord9data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f11, ChannelFrequencyTuningWord10data);  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x13, 4, ChannelFrequencyTuningWord10data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f12, ChannelFrequencyTuningWord11data);  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x14, 4, ChannelFrequencyTuningWord11data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f13, ChannelFrequencyTuningWord12data);  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x15, 4, ChannelFrequencyTuningWord12data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f14, ChannelFrequencyTuningWord13data);  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x16, 4, ChannelFrequencyTuningWord13data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f15, ChannelFrequencyTuningWord14data);	// 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x017, 4, ChannelFrequencyTuningWord14data, 0); // 写入频率控制字

	writeFrequencyTuningWord(f16, ChannelFrequencyTuningWord15data);  // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(0x18, 4, ChannelFrequencyTuningWord15data, 1); // 写入频率控制字
}
void AD9959::AD9959_SetPhamodulation163(double f, int p1, int p2, int p3, int p4, int p5, int p6, int p7, int p8, int p9, int p10, int p11, int p12, int p13, int p14, int p15, int p16)
{
	// 	 u8 b = 0;
	uchar ChannelFrequencyTuningWord0data[4];
	uchar ChannelPhaseOffsetTuningWord0data[4];
	uchar ChannelPhaseOffsetTuningWord1data[4];
	uchar ChannelPhaseOffsetTuningWord2data[4];
	uchar ChannelPhaseOffsetTuningWord3data[4];
	uchar ChannelPhaseOffsetTuningWord4data[4];
	uchar ChannelPhaseOffsetTuningWord5data[4];
	uchar ChannelPhaseOffsetTuningWord6data[4];
	uchar ChannelPhaseOffsetTuningWord7data[4];
	uchar ChannelPhaseOffsetTuningWord8data[4];
	uchar ChannelPhaseOffsetTuningWord9data[4];
	uchar ChannelPhaseOffsetTuningWord10data[4];
	uchar ChannelPhaseOffsetTuningWord11data[4];
	uchar ChannelPhaseOffsetTuningWord12data[4];
	uchar ChannelPhaseOffsetTuningWord13data[4];
	uchar ChannelPhaseOffsetTuningWord14data[4];
	uchar ChannelPhaseOffsetTuningWord15data[4];

	uchar ChannelFunctionRegisterdata[3] = {0xc0, 0x03, 0x30}; // 相位调制模式启用（连续扫描可能开启）
//	uchar AmplitudeControldata[3] = {0x00, 0x03, 0xff};		   // 十六级调制时启用，默认输出最大
	uchar FunctionRegister1data[3] = {0xD0, 0xf3, 0x00};

	writeToAD9959ViaSpi(FR1, 3, FunctionRegister1data, 0);		 // 设置功能寄存器
	writeToAD9959ViaSpi(CFR, 3, ChannelFunctionRegisterdata, 0); // 设置通道功能寄存器，即相位调制启动

	writePhaseOffsetTuningWord(p1, ChannelPhaseOffsetTuningWord0data);	 // 将十进制相位数转换为2进制相位控制字
	writeToAD9959ViaSpi(CPOW0, 2, ChannelPhaseOffsetTuningWord0data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p2, ChannelPhaseOffsetTuningWord1data); // 将十进制相位数转换为2进制相位控制字�
	writeToAD9959ViaSpi(0x0a, 4, ChannelPhaseOffsetTuningWord1data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p3, ChannelPhaseOffsetTuningWord2data); // 将十进制相位数转换为2进制相位控制字
	writeToAD9959ViaSpi(0x0b, 4, ChannelPhaseOffsetTuningWord2data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p4, ChannelPhaseOffsetTuningWord3data); // 将十进制相位数转换为2进制相位控制字�
	writeToAD9959ViaSpi(0x0c, 4, ChannelPhaseOffsetTuningWord3data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p5, ChannelPhaseOffsetTuningWord4data); // 将十进制相位数转换为2进制相位控制字
	writeToAD9959ViaSpi(0x0d, 4, ChannelPhaseOffsetTuningWord4data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p6, ChannelPhaseOffsetTuningWord5data); // 将十进制相位数转换为2进制相位控制字�
	writeToAD9959ViaSpi(0x0e, 4, ChannelPhaseOffsetTuningWord5data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p7, ChannelPhaseOffsetTuningWord6data); // 将十进制相位数转换为2进制相位控制字
	writeToAD9959ViaSpi(0x0f, 4, ChannelPhaseOffsetTuningWord6data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p8, ChannelPhaseOffsetTuningWord7data); // 将十进制相位数转换为2进制相位控制字�
	writeToAD9959ViaSpi(0x10, 4, ChannelPhaseOffsetTuningWord7data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p9, ChannelPhaseOffsetTuningWord8data); // 将十进制相位数转换为2进制相位控制字
	writeToAD9959ViaSpi(0x11, 4, ChannelPhaseOffsetTuningWord8data, 1); // 写入相位控制字

	writePhaseOffsetTuningWord(p10, ChannelPhaseOffsetTuningWord9data); // 将十进制相位数转换为2进制相位控制字�
	writeToAD9959ViaSpi(0x12, 4, ChannelPhaseOffsetTuningWord9data, 1);	 // 写入相位控制字

	writePhaseOffsetTuningWord(p11, ChannelPhaseOffsetTuningWord10data); // 将十进制相位数转换为2进制相位控制字
	writeToAD9959ViaSpi(0x13, 4, ChannelPhaseOffsetTuningWord10data, 1);  // 写入相位控制字

	writePhaseOffsetTuningWord(p12, ChannelPhaseOffsetTuningWord11data); // 将十进制相位数转换为2进制相位控制字�
	writeToAD9959ViaSpi(0x14, 4, ChannelPhaseOffsetTuningWord11data, 1);  // 写入相位控制字

	writePhaseOffsetTuningWord(p13, ChannelPhaseOffsetTuningWord12data); // 将十进制相位数转换为2进制相位控制字
	writeToAD9959ViaSpi(0x15, 4, ChannelPhaseOffsetTuningWord12data, 1);  // 写入相位控制字

	writePhaseOffsetTuningWord(p14, ChannelPhaseOffsetTuningWord13data); // 将十进制相位数转换为2进制相位控制字�
	writeToAD9959ViaSpi(0x16, 4, ChannelPhaseOffsetTuningWord13data, 1);  // 写入相位控制字

	writePhaseOffsetTuningWord(p15, ChannelPhaseOffsetTuningWord14data); // 将十进制相位数转换为2进制相位控制字
	writeToAD9959ViaSpi(0x17, 4, ChannelPhaseOffsetTuningWord14data, 1);  // 写入相位控制字

	writePhaseOffsetTuningWord(p16, ChannelPhaseOffsetTuningWord15data); // 将十进制相位数转换为2进制相位控制字
	writeToAD9959ViaSpi(0x18, 4, ChannelPhaseOffsetTuningWord15data, 1);  // 写入相位控制字

	f = 200000;
	writeFrequencyTuningWord(f, ChannelFrequencyTuningWord0data);	   // 将十进制频率数转换为2进制频率控制字
	writeToAD9959ViaSpi(CFTW0, 4, ChannelFrequencyTuningWord0data, 1); // 写入频率控制字
}
void AD9959::AD9959_SetAM163(double f)
{
	// 	 u8 cn = 0;
	uchar ChannelFrequencyTuningWord0data[4];

	uchar AmplitudeControldata[3] = {0x00, 0x03, 0xff};
	uchar ChannelAmplitudeTuningWord1data[4] = {0xef, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord2data[4] = {0xdf, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord3data[4] = {0xcf, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord4data[4] = {0xbf, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord5data[4] = {0xaf, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord6data[4] = {0x9f, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord7data[4] = {0x8f, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord8data[4] = {0x7f, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord9data[4] = {0x6f, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord10data[4] = {0x5f, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord11data[4] = {0x4f, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord12data[4] = {0x3f, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord13data[4] = {0x2f, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord14data[4] = {0x1f, 0x30, 0x00, 0x00};
	uchar ChannelAmplitudeTuningWord15data[4] = {0x0f, 0x30, 0x00, 0x00};
	uchar ChannelFunctionRegisterdata[3] = {0x40, 0x03, 0x30};
	uchar FunctionRegister1data[3] = {0xD0, 0xf3, 0x00};

	writeToAD9959ViaSpi(FR1, 3, FunctionRegister1data, 0);		 // 设置功能寄存器
	writeToAD9959ViaSpi(CFR, 3, ChannelFunctionRegisterdata, 0); // 设置通道功能寄存器

	writeToAD9959ViaSpi(ACR, 3, AmplitudeControldata, 0); // 设置幅度大小（S1）

	writeToAD9959ViaSpi(0x0a, 4, ChannelAmplitudeTuningWord1data, 0); // 设置幅度大小（S2）

	writeToAD9959ViaSpi(0x0b, 4, ChannelAmplitudeTuningWord2data, 0);

	writeToAD9959ViaSpi(0x0c, 4, ChannelAmplitudeTuningWord3data, 0);

	writeToAD9959ViaSpi(0x0d, 4, ChannelAmplitudeTuningWord4data, 0); // 设置幅度大小（S1）

	writeToAD9959ViaSpi(0x0e, 4, ChannelAmplitudeTuningWord5data, 0); // 设置幅度大小（S2）

	writeToAD9959ViaSpi(0x0f, 4, ChannelAmplitudeTuningWord6data, 0);

	writeToAD9959ViaSpi(0x10, 4, ChannelAmplitudeTuningWord7data, 0);

	writeToAD9959ViaSpi(0x11, 4, ChannelAmplitudeTuningWord8data, 0); // 设置幅度大小（S1）

	writeToAD9959ViaSpi(0x12, 4, ChannelAmplitudeTuningWord9data, 0); // 设置幅度大小（S2）

	writeToAD9959ViaSpi(0x13, 4, ChannelAmplitudeTuningWord10data, 0);

	writeToAD9959ViaSpi(0x14, 4, ChannelAmplitudeTuningWord11data, 0);

	writeToAD9959ViaSpi(0x15, 4, ChannelAmplitudeTuningWord12data, 0); // 设置幅度大小（S1）

	writeToAD9959ViaSpi(0x16, 4, ChannelAmplitudeTuningWord13data, 0); // 设置幅度大小（S2）

	writeToAD9959ViaSpi(0x17, 4, ChannelAmplitudeTuningWord14data, 0);

	writeToAD9959ViaSpi(0x18, 4, ChannelAmplitudeTuningWord15data, 0);

	writeFrequencyTuningWord(f, ChannelFrequencyTuningWord0data); // 写频率控制字
	writeToAD9959ViaSpi(CFTW0, 4, ChannelFrequencyTuningWord0data, 1);
}

/////////////////////////////////////////////////////////////////////////
void AD9959::AD9959_Frequency_Sweep(double f1, double f2, double a1, double a2) // f1起始频率，f2终止频率，a1上升δ，a2下降δ
{
	uchar ChannelFrequencyTuningWorddata1[4];
	uchar ChannelFrequencyTuningWorddata2[4];
	uchar ChannelFrequencyTuningWorddata3[4];
	uchar ChannelFrequencyTuningWorddata4[4];
	uchar FunctionRegister1data[3] = {0xD0, 0x00, 0x00};	   // 默认情况下选择
	uchar ChannelFunctionRegisterdata[3] = {0x80, 0x43, 0x20}; // 扫频模式时选择;连续扫描没有启用，无RU/RD
	uchar SweepRampRatedata[2] = {0xff, 0xff};				   // 默认单位扫描时间最长
	// uchar FunctionRegister1data[3] = {0xD0,0x04,0x00};    							//线性扫描时需要RU/RD时选择开启
	//  uchar AmplitudeControldata[3] = {0xff,0x1f,0xff};									//开启RU/RD

	writeToAD9959ViaSpi(FR1, 3, FunctionRegister1data, 0);		 // 设置功能寄存器
	writeToAD9959ViaSpi(CFR, 3, ChannelFunctionRegisterdata, 0); //  设置通道功能寄存器
																 // 	writeToAD9959ViaSpi(ACR,3,AmplitudeControldata,1); 								//需输出幅度RU/RD模式才选择,且要修改功能寄存器FR1
																 // 	f=500000;    // 设置起始频率S0（0x04）

	writeFrequencyTuningWord(f1, ChannelFrequencyTuningWorddata1); // 写频率控制字
	writeToAD9959ViaSpi(CFTW0, 4, ChannelFrequencyTuningWorddata1, 0);
	// 	f=5000000; //设置终止频率(0x0A)

	writeFrequencyTuningWord(f2, ChannelFrequencyTuningWorddata2); // 写频率控制字
	writeToAD9959ViaSpi(0x0A, 4, ChannelFrequencyTuningWorddata2, 0);
	// 	f=100;   //设置上升δ

	writeFrequencyTuningWord(a1, ChannelFrequencyTuningWorddata3); // 写频率控制字
	writeToAD9959ViaSpi(RDW, 4, ChannelFrequencyTuningWorddata3, 0);

	//   f=100;  //设置下降δ
	writeFrequencyTuningWord(a2, ChannelFrequencyTuningWorddata4); // 写频率控制字
	writeToAD9959ViaSpi(FDW, 4, ChannelFrequencyTuningWorddata4, 0);

	writeToAD9959ViaSpi(SRR, 2, SweepRampRatedata, 1); // 设置单位步进时间
}

void AD9959::AD9959_Phase_Sweep(int p1, int p2, int a1, int a2, double f) // p1起始相位，p2终止相位，a1设置上升δ，a2设置下降δ
{
	uchar ChannelPhaseOffsetTuningWorddata1[2];
	uchar ChannelPhaseOffsetTuningWorddata2[2];
	uchar ChannelPhaseOffsetTuningWorddata3[2];
	uchar ChannelPhaseOffsetTuningWorddata4[2];
	uchar ChannelFrequencyTuningWord0data[4];

	uchar FunctionRegister1data[3] = {0xD0, 0x00, 0x00};	   // 默认情况下选择
	uchar ChannelFunctionRegisterdata[3] = {0xc0, 0xC3, 0x30}; // 扫相模式时选择，连续扫描没有启用，无RU/RD
	uchar SweepRampRatedata[2] = {0xff, 0xff};				   // 默认单位扫描时间最长
	// uchar FunctionRegister1data[3] = {0xD0,0x04,0x00};    						 //线性扫描时需要RU/RD时选择开启
	//  uchar AmplitudeControldata[3] = {0xff,0x1f,0xff};								 //开启RU/RD

	writeToAD9959ViaSpi(FR1, 3, FunctionRegister1data, 0);		 // 设置功能寄存器
	writeToAD9959ViaSpi(CFR, 3, ChannelFunctionRegisterdata, 0); //  设置通道功能寄存器
																 // 	 writeToAD9959ViaSpi(ACR,3,AmplitudeControldata,1); 						 //需输出幅度RU/RD模式才选择,且要修改功能寄存器FR1
																 // 	 p=0;// 设置起始相位S0（0x04）   数据类型与子函数里面不一样
	writePhaseOffsetTuningWord(p1, ChannelPhaseOffsetTuningWorddata1);
	writeToAD9959ViaSpi(CPOW0, 2, ChannelPhaseOffsetTuningWorddata1, 0);
	//    p=360;//设置终止相位E0(0x0A)
	writePhaseOffsetTuningWord(p2, ChannelPhaseOffsetTuningWorddata2);
	writeToAD9959ViaSpi(0x0a, 4, ChannelPhaseOffsetTuningWorddata2, 0);
	//    p=10;//设置上升δ
	writePhaseOffsetTuningWord(a1, ChannelPhaseOffsetTuningWorddata3);
	writeToAD9959ViaSpi(RDW, 4, ChannelPhaseOffsetTuningWorddata3, 0);
	//    p=10;//设置下降δ
	writePhaseOffsetTuningWord(a2, ChannelPhaseOffsetTuningWorddata4);
	writeToAD9959ViaSpi(FDW, 4, ChannelPhaseOffsetTuningWorddata4, 0);
	writeToAD9959ViaSpi(SRR, 2, SweepRampRatedata, 0); // 写单位步进时间
	// 	 f=400000;    //设置输出频率大小
	writeFrequencyTuningWord(f, ChannelFrequencyTuningWord0data); // 写频率控制字
	writeToAD9959ViaSpi(CFTW0, 4, ChannelFrequencyTuningWord0data, 1);
}

void AD9959::AD9959_Amplitute_Sweep(int a, int a1, int a2, double f) // a终止幅度，a1上升δ，a2下降δ，f输出频率
{
	uchar ChannelAmplitudeTuningWorddata1[4];
	uchar ChannelAmplitudeTuningWorddata2[4];
	uchar ChannelAmplitudeTuningWorddata3[4];
	uchar ChannelFrequencyTuningWorddata4[4];

	uchar FunctionRegister1data[3] = {0xD0, 0x00, 0x00};	   // 默认情况下选择
	uchar ChannelFunctionRegisterdata[3] = {0x40, 0x43, 0x20}; // 幅度扫描位启动
	uchar AmplitudeControldata[3] = {0x00, 0x0, 0x3f};		   // 幅度扫描模式时启用
	uchar SweepRampRatedata[2] = {0xff, 0xff};				   // 默认单位扫描时间最长

	writeToAD9959ViaSpi(FR1, 3, FunctionRegister1data, 0); // 设置功能寄存器
	writeToAD9959ViaSpi(CFR, 3, ChannelFunctionRegisterdata, 0);
	// 写起始幅度S0
	writeToAD9959ViaSpi(ACR, 3, AmplitudeControldata, 0);
	// 写终止幅度E0
	// 	a=1023; //写上升δ,不能写到1024，最大为1023
	writeAmplitudeTuningWord(a, ChannelAmplitudeTuningWorddata1);
	writeToAD9959ViaSpi(0x0a, 4, ChannelAmplitudeTuningWorddata1, 0); // CTW0 address 0x04.输出10MHZ频率
	// 	a=10; //写上升δ
	writeAmplitudeTuningWord(a1, ChannelAmplitudeTuningWorddata2); // 写频率控制字
	writeToAD9959ViaSpi(RDW, 4, ChannelAmplitudeTuningWorddata2, 0);
	//   a=10;  //写下降δ
	writeAmplitudeTuningWord(a2, ChannelAmplitudeTuningWorddata3); // 写频率控制字
	writeToAD9959ViaSpi(FDW, 4, ChannelAmplitudeTuningWorddata3, 0);
	writeToAD9959ViaSpi(SRR, 2, SweepRampRatedata, 0); // 写单位步进时间
	// 	f=400000;
	writeFrequencyTuningWord(f, ChannelFrequencyTuningWorddata4); // 写频率控制字
	writeToAD9959ViaSpi(CFTW0, 4, ChannelFrequencyTuningWorddata4, 1);
}

