#include "FFT.hpp"

FFT::FFT(int fft_length, float sample_rate) : fft_length(fft_length), sample_rate(sample_rate)
{
    scfft = (arm_cfft_radix4_instance_f32 *)malloc(sizeof(arm_cfft_radix4_instance_f32));
    if (scfft == NULL)
    {
        perror("Failed to allocate memory for FFT instance");
        exit(EXIT_FAILURE);
    }
    fft_inputbuf = (float *)malloc(fft_length * 2 * sizeof(float));
    if (fft_inputbuf == NULL)
    {
        perror("Failed to allocate memory for FFT input buffer");
        free(scfft);
        exit(EXIT_FAILURE);
    }
    arm_cfft_radix4_init_f32(scfft, fft_length, 0, 1);
}

FFT::~FFT()
{
    free(fft_inputbuf);
    free(scfft);
}


void FFT::find_main_freq(float *fft_outputbuf)
{
    float max_mag = 0;
    uint16_t max_idx = 0;
    for (int i = 1; i < fft_length / 2; i++)
    { 
        if (fft_outputbuf[i] > max_mag)
        {
            max_mag = fft_outputbuf[i];
            max_idx = i;
        }
    }

    float top5_mag[5] = {0};
    uint16_t top5_idx[5] = {0};

    for (int i = 1; i < fft_length / 2; i++)
    { 
        float current_mag = fft_outputbuf[i];

        for (int j = 0; j < 5; j++)
        {
            if (current_mag > top5_mag[j])
            {
                for (int k = 4; k > j; k--)
                {
                    top5_mag[k] = top5_mag[k - 1];
                    top5_idx[k] = top5_idx[k - 1];
                }
                top5_mag[j] = current_mag;
                top5_idx[j] = i;
                break;
            }
        }
    }

    for (int i = 0; i < 5; i++)
    {
        main_Frequencies[i] = (top5_idx[i] * sample_rate) / fft_length;
    }
}

void FFT::FFT_PROCESS(uint16_t *adc_buffer)
{
    float *fft_outputbuf = (float *)malloc(fft_length * sizeof(float));

    if (fft_outputbuf == NULL)
    {
        perror("Failed to allocate memory for FFT output buffer");
        free(fft_inputbuf);
        free(scfft);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < fft_length; i++)
    {
        fft_inputbuf[2 * i] = (float)adc_buffer[i];
        fft_inputbuf[2 * i + 1] = 0;
    }

    arm_cfft_radix4_f32(scfft, fft_inputbuf);

    arm_cmplx_mag_f32(fft_inputbuf, fft_outputbuf, fft_length);

    find_main_freq(fft_outputbuf);

    free(fft_outputbuf);
}
