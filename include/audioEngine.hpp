#ifndef AUDIO_ENGINE_HPP
#define AUDIO_ENGINE_HPP

#include <array>
#include <vector>
#include <iostream>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <cstring>
#include "portaudio.h"
#include "simDefs.hpp"

#define NUM_FRAMES 10

struct Data {
    std::vector<float> audio_buffer;
    std::atomic<int> full{0};
};

class AudioEngine {
public:
    AudioEngine(int sampleRate = 48000, int bufferSize = 512);
    ~AudioEngine();
    void start();
    void stop();
    void delay();
    void mute(bool shouldMute) { muted_ = shouldMute; }
    void pushChunk(const float* buffer, size_t numSamples);
private:
    PaStream *mainStream = nullptr;
    PaStreamParameters outputParameters;
    PaError err;

    static int paStreamCB(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer,
                              const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData);
    static void paStreamFinished(void *userData);

    int internalAudioCB(float *out, unsigned long frames);

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
