#ifndef AUDIO_DSP_HPP
#define AUDIO_DSP_HPP
#include <vector>
#include <algorithm>
#include <cmath>

class AudioDSP_Toolbox {
    public:
    AudioDSP_Toolbox();
    ~AudioDSP_Toolbox();
    std::vector<float> sampleInterp(float *in, int inLen, float inFs, float outFs);
    float calculateDecibleLevel(const std::vector<float>& buffer) {
        float sumSquares = 0.0f;
        for (float sample : buffer) {
            sumSquares += sample * sample;
        }
        float rms = std::sqrt(sumSquares / buffer.size());
        if (rms == 0.0f) return -100.0f; // Return a very low dB value for silence
        return 20.0f * std::log10(rms); // Convert to dB, add small value to avoid log(0)
    }
};

#endif // AUDIO_DSP_HPP