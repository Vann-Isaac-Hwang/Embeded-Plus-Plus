#ifndef __FFT_H
#define __FFT_H

#include "main.h"
#include "string.h"
#include "stdlib.h"
#include "errno.h"
#include "arm_math.h"


class FFT {
private:
    arm_cfft_radix4_instance_f32* scfft;
    int  fft_length;
    float sample_rate;
    float* fft_inputbuf;
    float main_Frequencies[5];
    void find_main_freq(float* fft_outputbuf);
public:
    FFT(int fft_length,float sample_rate);
    ~FFT();

    void FFT_PROCESS(uint16_t* adc_buffer);
};

#endif