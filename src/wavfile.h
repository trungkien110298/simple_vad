#include <sndfile.hh>
#include <eigen3/Eigen/Dense>
#include <iostream>
#define BUFFER_LEN 1024

using namespace std;
using namespace Eigen;

MatrixXf readfile(SNDFILE *infile, SF_INFO &sfinfo)
{

    int channels = sfinfo.channels;
    int num_samples = sfinfo.frames;

    float buf[BUFFER_LEN];
    sf_count_t frames;
    int k, m, readcount;
    frames = BUFFER_LEN / channels;

    MatrixXf samples(num_samples, channels);

    int count = 0;

    while ((readcount = sf_readf_float(infile, buf, frames)) > 0)
    {
        for (k = 0; k < readcount; k++)
        {
            for (m = 0; m < channels; m++)
            {
                samples(count, m) = buf[k * channels + m];
            }
            count++;
        }
    }

    return samples;
}

void writefile(SNDFILE *outfile, SF_INFO &sfinfo, MatrixXf samples)
{

    SndfileHandle file;
    int channels = samples.cols();
    int num_samples = samples.rows();
    float buf[num_samples * channels];
    int frames = num_samples;

    for (int k = 0; k < num_samples; k++)
    {
        for (int m = 0; m < channels; m++)
        {
            buf[k * channels + m] = samples(k, m);
        }
    }

    sf_writef_float(outfile, buf, frames);
}