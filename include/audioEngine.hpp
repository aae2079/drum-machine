#ifndef AUDIO_ENGINE_HPP
#define AUDIO_ENGINE_HPP

#include "portaudio.h"
#include <vector>

#define NUM_FRAMES 10

struct Data{
    std::vector<float> audio_buffer; 
    int frameCount{0};
};

class AudioEngine {
public:
    AudioEngine();
    ~AudioEngine();
    void Initialize();
    void StartStream();
    void StopStream();
    void CloseStream();
    void Terminate();
    static int paStreamCB(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer,
                              const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData);
    static void paStreamFinished(void *userData);

    void slotConsumption();
private:
    PaStream *stream;
    PaStream *mainStream = nullptr;
    PaStreamParameters outputParameters;
    PaError err;

    std::vector<Data> ringBuf(NUM_FRAMES);

    int fill_ix = 0;
    int read_ix = 0;
    int buf_pos = 0;
};
#endif // AUDIO_ENGINE_HPP