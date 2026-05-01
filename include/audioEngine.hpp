#ifndef AUDIO_ENGINE_HPP
#define AUDIO_ENGINE_HPP

#include <vector>
#include <iostream>
#include <atomic>
#include <cstring>
#include "portaudio.h"
#include "simDefs.hpp"
#include "audioDefs.hpp"

class AudioEngine {
public:
    AudioEngine(int sampleRate = SAMPLE_RATE, int bufferSize = BUFFER_SIZE);
    ~AudioEngine();
    void start();
    void stop();
    void delay();
    void consumeAudio(const float* buffer, size_t numSamples);
private:
    PaStream *stream;
    PaStream *mainStream = nullptr;
    PaStreamParameters outputParameters;
    PaError err;

    static int paStreamCB(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer,
                              const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData);
    static void paStreamFinished(void *userData);
    
    int internalAudioCB(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer,
                    const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData);


    std::vector<float> audio_buffer; 

    int _sampleRate;
    int _bufferSize;

};
#endif // AUDIO_ENGINE_HPP