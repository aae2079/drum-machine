//Audio DSP Toolbox
#include "audioDSP.hpp"
#include <cmath>

AudioDSP_Toolbox::AudioDSP_Toolbox() {
    // Constructor implementation (if needed)
}

AudioDSP_Toolbox::~AudioDSP_Toolbox() {
    // Destructor implementation (if needed)
}

std::vector<float> AudioDSP_Toolbox::sampleInterp(float *in, int inLen, int outLen, float inFs, float outFs){
    if (inLen <= 0 || inFs <= 0.0f || outFs <= 0.0f) return {};

    // Output length = ceil(inLen * outFs / inFs).
    // Caller is responsible for allocating at least that many floats.
    std::vector<float> out(outLen, 0.0f);

    // Step size in input-sample coordinates per output sample.
    // < 1.0 when upsampling (outFs > inFs), > 1.0 when downsampling.
    double step = (double)inFs / outFs;

    for (int n = 0; n < outLen; n++) {
        double pos = n * step;          // fractional index into in[]
        int    i0  = (int)pos;          // left neighbour
        int    i1  = i0 + 1;            // right neighbour
        double alpha = pos - i0;        // weight for i1, in [0, 1)

        // Clamp so we never read past the end of the input.
        i0 = std::min(i0, inLen - 1);
        i1 = std::min(i1, inLen - 1);

        out[n] = (float)((1.0 - alpha) * in[i0] + alpha * in[i1]);
    }

    return out;
}

std::vector<float> AudioDSP_Toolbox::normalizeAudio(const std::vector<float>& buffer) {
    if (buffer.empty()) return {};

    // Find the maximum absolute value in the buffer
    float maxVal = 0.0f;
    for (float sample : buffer) {
        maxVal = std::max(maxVal, std::abs(sample));
    }
    // Normalize the buffer so that the maximum absolute value becomes 1.0
    std::vector<float> normalized(buffer.size());
    for (size_t i = 0; i < buffer.size(); i++) {
        normalized[i] = buffer[i] / maxVal;
    }

    return normalized;
}

