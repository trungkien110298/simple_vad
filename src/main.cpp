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

float calculate_sfm(VectorXcf &spectrum)
{
    long long i;
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

float calculate_dominant(VectorXcf &spectrum)
{
    long long i;
    long long i_real, i_imag;
    float real, imag;
    float max_real, max_imag;
    float max, max_i;

    for (i = 0; i < FFT_POINTS / 2; i++)
    {
        real = spectrum(i).real();
        imag = spectrum(i).imag();

        if (i == 0)
        {
            max = abs(spectrum(i));
            max_i = i;
        }
        else
        {
            if (abs(spectrum(i)) > max)
            {
                max = abs(spectrum(i));
                max_i = i;
            }
        }
    }

    return max_i * FFT_STEP;
}

float calculate_energy(VectorXf &frame)
{
    long long i;
    long long frameLength = frame.size();
    long double sum = 0;

    for (i = 0; i < FRAME_LENGTH; i++)
    {
        sum += powl(frame[i] * NORM_INT16, 2);
    }

    return sqrtl(sum / FRAME_LENGTH);
}

void vad(const char fileNameIn[], const char fileNameOut[])
{
    FFT<float> fft;
    int counter;
    long long silence_run, speech_run, silence_count;
    int vad_dec;
    bool *vad;

    // Read file
    SF_INFO sfinfo;
    SNDFILE *f = sf_open(fileNameIn, SFM_READ, &sfinfo);
    MatrixXf audio = readfile(f, sfinfo);
    long long numSamples = sfinfo.frames;
    long long sampleRate = sfinfo.samplerate;
    int numChannels = sfinfo.channels;
    sf_close(f);

    Features frameFeature;
    Features minFeature;
    Features currThresh;

    vad_dec = 0;
    silence_run = 0;
    speech_run = 0;
    silence_count = 0;
    currThresh.F = F_PRIMTHRESH; // moved from 3-4 for opt
    currThresh.SFM = SF_PRIMTHRESH;
    currThresh.energy = ENERGY_PRIMTHRESH;

    long long frameLength = FRAME_LENGTH;
    long long numFrames = ceil(1.0 * numSamples / frameLength);
    vad = new bool[numFrames];

    for (long long i = 0; i < numFrames - 1; i++)
    {
        VectorXf frame = audio.block(i * frameLength, 0, frameLength, 1);
        frameFeature.energy = calculate_energy(frame);

        VectorXcf audio_frame_fft(frameLength);
        fft.fwd(audio_frame_fft, frame);

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
            speech_run++;
            silence_run = 0;
        }
        else
        {
            /* silence run or silence count? */
            silence_run++;
            silence_count++;
            minFeature.energy = ((silence_count * minFeature.energy) + frameFeature.energy) / (silence_count + 1);
            speech_run = 0;
        }

        currThresh.energy = ENERGY_PRIMTHRESH * log10f(minFeature.energy);

        /* 4-0 ignore silence run if less than 10 frames*/
        if (silence_run > 9)
        {
            vad_dec = 0;
            if (silence_run == 10)
                for (long long j = i; j > i - silence_run; j--)
                {
                    vad[j] = 0;
                }
        }

        /* 5-0 ignore speech run if less than 5 frames */
        if (speech_run > 4)
        {
            vad_dec = 1;
            if (speech_run == 5)
                for (long long j = i; j > i - speech_run; j--)
                {
                    vad[j] = 1;
                }
        }
        vad[i] = vad_dec;
    }
    vad[numFrames - 1] = vad[numFrames - 2]; //Last frame

    MatrixXf new_audio(numSamples, numChannels);
    long long count = 0;

    for (long long i = 0; i < numSamples; i++)
    {
        if (vad[i / frameLength])
        {
            new_audio(count++) = audio(i);
        }
    }

    // Read file

    SF_INFO sfinfo_out;
    sfinfo_out.samplerate = sampleRate;
    sfinfo_out.channels = numChannels;
    sfinfo_out.sections = 1;
    sfinfo_out.seekable = 1;
    sfinfo_out.frames = numSamples;
    sfinfo_out.format = SF_FORMAT_WAVEX | SF_FORMAT_FLOAT;
    SNDFILE *g = sf_open(fileNameOut, SFM_WRITE, &sfinfo_out);
    writefile(g, sfinfo_out, new_audio.block(0, 0, count, numChannels));
    sf_close(g);
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        cout << "Usage: ./main <input_file_path> <output_file_path>";
        return 1;
    }
    vad(argv[1], argv[2]);
    return 0;
}