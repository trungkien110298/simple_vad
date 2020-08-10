#define BUFFER_LEN 1024
#define FRAME_SIZE 10       //1O ms
#define SAMPLING_RATE 16000 // Config SAMPLING RATE 16000 or 44100

#if SAMPLING_RATE == 16000
#define FFT_POINTS 128
#else
#define FFT_POINTS 256
#endif

#define FFT_STEP (SAMPLING_RATE / FFT_POINTS)
#define FRAME_LENGTH (SAMPLING_RATE * FRAME_SIZE / 1000)

#define ENERGY_PRIMTHRESH 40
#define F_PRIMTHRESH 185
#define SF_PRIMTHRESH 5

#define NORM_INT16 32767

typedef struct
{
    long double energy;
    short F;
    float SFM;
} Features;
