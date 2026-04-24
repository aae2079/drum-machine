#ifndef AUDIO_DSP_HPP
#define AUDIO_DSP_HPP
#include <vector>

class AudioDSP_Toolbox {
    public:
    AudioDSP_Toolbox();
    ~AudioDSP_Toolbox();
    std::vector<float> sampleInterp(float *in, int inLen, float inFs, float outFs);
};

#endif // AUDIO_DSP_HPP