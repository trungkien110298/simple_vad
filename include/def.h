#define BUFFER_LEN 1024
#define FRAME_SIZE 10
#define ENERGY_PRIMTHRESH 40
#define F_PRIMTHRESH 185
#define SF_PRIMTHRESH 5

#define ALSA_PCM_NEW_HW_PARAMS_API
#define FRAME_SIZE 0.01 // ms
#define SAMPLING_RATE 44100

#define FFT_POINTS 256
#define FFT_STEP (SAMPLING_RATE / FFT_POINTS)
#define NUM_OF_FRAMES (FRAME_SIZE * SAMPLING_RATE) * 10000

#define DELAY 10000 // us

typedef struct
{
  long double energy;
  short F;
  float SFM;
} Features;
