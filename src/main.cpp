#include <iostream>
#include <math.h>
#include <sndfile.hh>
#include <Eigen/Dense>
#include <unsupported/Eigen/FFT>
#include <unsupported/Eigen/MatrixFunctions>
#include <chrono>

#include "wavfile.h"

#define BUFFER_LEN 1024
#define FRAME_SIZE 10
#define ENERGY_PRIMTHRESH 40
#define F_PRIMTHRESH 185
#define SF_PRIMTHRESH 5

using namespace std;
using namespace Eigen;
using namespace std::chrono;

double calculate_energy(MatrixXd signal)
{
    int i;
    int frameLenght = signal.rows();
    long double sum = 0;

    for (i = 0; i < ; i++)
    {
        sum += powl(signal[i], 2);
    }

    return sqrtl(sum / FFT_POINTS);
}

int main()
{

    char fileName[] = "/home/trungkien1102/VTCC/vad/data/sample.wav";

    // Read file
    SF_INFO sfinfo;
    SNDFILE *f = sf_open(fileName, SFM_READ, &sfinfo);
    MatrixXd audio = readfile(f, sfinfo);
    long long numSamples = sfinfo.frames;
    long long sampleRate = sfinfo.samplerate;
    int numChannels = sfinfo.channels;
    sf_close(f);

    long long frameLength = sampleRate * 1000 / FRAME_SIZE;
    long long numFrames = round(1.0 * numSamples / frameLength);

    for (long long i = 0; i < numFrames; i++)
    {
        MatrixXd frame = audio.block(i * frameLength, 0, numChannels, frameLength);
    }

    return 0;
}