#include <iostream>
#include <math.h>
#include <sndfile.hh>
#include <Eigen/Dense>
#include <unsupported/Eigen/FFT>
#include <unsupported/Eigen/MatrixFunctions>
#include <chrono>

#include "def.h"
#include "wavfile.h"

using namespace std;
using namespace Eigen;
using namespace std::chrono;

double calculate_energy(VectorXd &frame)
{
    int i;
    int frameLenght = frame.size();
    long double sum = 0;

    for (i = 0; i < frameLenght; i++)
    {
        sum += powl(frame[i], 2);
    }

    return sqrtl(sum / frameLenght);
}

float calculate_sfm(VectorXcd &spectrum)
{
    int i;
    float sum_ari;
    float sum_geo;
    float sig;

    sum_ari = 0;
    sum_geo = 0;
    for (i = 0; i < FFT_POINTS; i++)
    {
        sig = abs(spectrum(i));
        sum_ari += sig;
        sum_geo += logf(sig);
    }
    sum_ari = sum_ari / FFT_POINTS;
    sum_geo = expf(sum_geo / FFT_POINTS);

    return -10 * log10f(sum_geo / sum_ari);
}

float calculate_dominant(VectorXcd &spectrum)
{
    int i;
    int i_real, i_imag;
    float real, imag;
    float max_real, max_imag;

    for (i = 0; i < FFT_POINTS / 2; i++)
    {
        real = spectrum(i).real();
        imag = spectrum(i).imag();

        if (i == 0)
        {
            max_real = real;
            max_imag = imag;
            i_real = i;
            i_imag = i;
        }
        else
        {
            if (real > max_real)
            {
                max_real = real;
                i_real = i;
            }

            if (imag > max_imag)
            {
                max_imag = imag;
                i_imag = i;
            }
        }
    }

    if (max_real > max_imag)
    {
        return i_real * FFT_STEP;
    }
    else
    {
        return i_imag * FFT_STEP;
    }
}

int main()
{
    FFT<double> fft;
    int counter;
    int silence_count, speech_count;
    int vad_dec;
    char fileName[] = "../data/sample.wav";

    // Read file
    SF_INFO sfinfo;
    SNDFILE *f = sf_open(fileName, SFM_READ, &sfinfo);
    MatrixXd audio = readfile(f, sfinfo);
    long long numSamples = sfinfo.frames;
    long long sampleRate = sfinfo.samplerate;
    int numChannels = sfinfo.channels;
    sf_close(f);

    Features frameFeature;
    Features minFeature;
    Features currThresh;

    vad_dec = 0;
    silence_count = 0;           // TODO: check this (where to put this?)
    currThresh.F = F_PRIMTHRESH; // moved from 3-4 for opt
    currThresh.SFM = SF_PRIMTHRESH;

    long long frameLength = sampleRate * 1000 / FRAME_SIZE;
    long long numFrames = round(1.0 * numSamples / frameLength);

    for (long long i = 0; i < numFrames; i++)
    {
        VectorXd frame = audio.block(i * frameLength, 0, frameLength, 1);
        frameFeature.energy = calculate_energy(frame);

        VectorXcd audio_frame_fft(frameLength);
        fft.fwd(audio_frame_fft, frame.col(0), FFT_POINTS);

        frameFeature.F = calculate_dominant(audio_frame_fft);
        frameFeature.SFM = calculate_sfm(audio_frame_fft);

        /* 3-3 supposing that some of the first 30 frames are silence */
        if (i == 0)
        {
            minFeature.energy = frameFeature.energy;
            minFeature.F = frameFeature.F;
            minFeature.SFM = frameFeature.SFM;
        }
        else if (i < 30)
        {
            minFeature.energy = (frameFeature.energy > minFeature.energy) ? minFeature.energy : frameFeature.energy;
            minFeature.F = (frameFeature.F > minFeature.F) ? minFeature.F : frameFeature.F;
            minFeature.SFM = (frameFeature.SFM > minFeature.SFM) ? minFeature.SFM : frameFeature.SFM;
        }

        /* 3-4 set thresholds */
        currThresh.energy = ENERGY_PRIMTHRESH * log10f(minFeature.energy);

        /* 3-5 calculate decision */
        counter = 0;
        if ((frameFeature.energy - minFeature.energy) >= currThresh.energy)
        {
            counter++;
        }
        
        if ((frameFeature.F - minFeature.F) >= currThresh.F)
        {
            counter++;
        }
       
        if ((frameFeature.SFM - minFeature.SFM) >= currThresh.SFM)
        {
            counter++;
        }
        

        /* 3-6, 3-7, 3-8: VAD */
        if (counter > 1)
        {
            speech_count++;
            silence_count = 0;
        }
        else
        {
            /* silence run or silence count? */
            silence_count++;
            minFeature.energy = ((silence_count * minFeature.energy) + frameFeature.energy) / (silence_count + 1);
            speech_count = 0;
        }

        currThresh.energy = ENERGY_PRIMTHRESH * log10f(minFeature.energy);

        /* 4-0 ignore silence run if less than 10 frames*/
        if (silence_count > 10)
        {
            vad_dec = 0;
        }

        /* 5-0 ignore speech run if less than 5 frames */
        if (speech_count > 4)
        {
            vad_dec = 1;
        }
    }
    return 0;
}