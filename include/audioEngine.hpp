#ifndef AUDIO_ENGINE_HPP
#define AUDIO_ENGINE_HPP

#include <array>
#include <vector>
#include <iostream>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <cstring>
#include <juce_audio_devices/juce_audio_devices.h>
#include "simDefs.hpp"

#define NUM_FRAMES 10

struct Data {
    std::vector<float> audio_buffer;
    std::atomic<int> full{0};
};

class AudioEngine : public juce::AudioIODeviceCallback {
public:
    AudioEngine(int sampleRate = 48000, int bufferSize = 512);
    ~AudioEngine() override;
    void start();
    void stop();
    void delay();
    void mute(bool shouldMute) { muted_ = shouldMute; }
    void pushChunk(const float* buffer, size_t numSamples);


private:
    // juce::AudioIODeviceCallback overrides
    void audioDeviceIOCallbackWithContext(
        const float* const* inputChannelData,
        int numInputChannels,
        float* const* outputChannelData,
        int numOutputChannels,
        int numSamples,
        const juce::AudioIODeviceCallbackContext& context) override;
 
    void audioDeviceAboutToStart(juce::AudioIODevice* device) override;
    void audioDeviceStopped() override;

    void internalAudioCB(float *out, int frames);

    juce::AudioDeviceManager deviceManager_;

    std::array<Data, NUM_FRAMES> ringBuf;

    std::atomic<int> fill_ix{0};
    std::atomic<int> read_ix{0};
    int buf_pos = 0;

    std::mutex slotMtx_;
    std::condition_variable slotCV_;

    int _sampleRate;
    int _bufferSize;

    bool muted_;
};
#endif // AUDIO_ENGINE_HPP
