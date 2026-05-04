#include "audioEngine.hpp"
#include <algorithm>

AudioEngine::AudioEngine(float sampleRate, int bufferSize)
    : _sampleRate(sampleRate), _bufferSize(bufferSize)
{
    err_ = Pa_Initialize();
    if (err_ != paNoError) {
        std::cerr << "PortAudio init failed: " << Pa_GetErrorText(err_) << "\n";
        Pa_Terminate();
        return;
    }

    outputParameters_.device = Pa_GetDefaultOutputDevice();
    if (outputParameters_.device == paNoDevice) {
        std::cerr << "No default output device.\n";
        Pa_Terminate();
        return;
    }
    outputParameters_.channelCount          = NUM_CHANNELS;
    outputParameters_.sampleFormat          = paFloat32;
    outputParameters_.suggestedLatency      = Pa_GetDeviceInfo(outputParameters_.device)->defaultLowOutputLatency;
    outputParameters_.hostApiSpecificStreamInfo = nullptr;
}

AudioEngine::~AudioEngine() {
    stop();
    Pa_Terminate();
}

void AudioEngine::start() {
    err_ = Pa_OpenStream(&mainStream_, nullptr, &outputParameters_,
                         _sampleRate, (unsigned long)_bufferSize,
                         paClipOff, paStreamCB, this);
    if (err_ != paNoError) {
        std::cerr << "Pa_OpenStream failed: " << Pa_GetErrorText(err_) << "\n";
        Pa_Terminate();
        return;
    }

    Pa_SetStreamFinishedCallback(mainStream_, paStreamFinished);

    err_ = Pa_StartStream(mainStream_);
    if (err_ != paNoError) {
        std::cerr << "Pa_StartStream failed: " << Pa_GetErrorText(err_) << "\n";
        Pa_CloseStream(mainStream_);
        Pa_Terminate();
    }
}

void AudioEngine::stop() {
    if (mainStream_) {
        Pa_StopStream(mainStream_);
        Pa_CloseStream(mainStream_);
        mainStream_ = nullptr;
    }
}

bool AudioEngine::pushPacket(const AudioPacket& pkt) {
    return packetBuf_.push(pkt);
}

// --- PortAudio callbacks (static trampolines) ---

int AudioEngine::paStreamCB(const void*, void* out, unsigned long frames,
                            const PaStreamCallbackTimeInfo*, PaStreamCallbackFlags, void* ud)
{
    return static_cast<AudioEngine*>(ud)->internalAudioCB(out, frames);
}

void AudioEngine::paStreamFinished(void*) {
    std::cout << "PortAudio stream finished.\n";
}

// Runs on the PortAudio callback thread.
// Drains the SPSC ring buffer into the output; fills with silence when empty.
// No heap allocation, no locks.
int AudioEngine::internalAudioCB(void* outputBuffer, unsigned long framesPerBuffer) {
    float*  out     = static_cast<float*>(outputBuffer);
    size_t  need    = (size_t)framesPerBuffer;

    while (need > 0) {
        // Refill staging packet when exhausted
        if (packetOffset_ >= currentPkt_.count) {
            if (!packetBuf_.pop(currentPkt_)) {
                std::memset(out, 0, need * sizeof(float));
                return paContinue;
            }
            packetOffset_ = 0;
        }

        size_t avail  = currentPkt_.count - packetOffset_;
        size_t n      = std::min(avail, need);
        std::memcpy(out, currentPkt_.samples + packetOffset_, n * sizeof(float));
        out           += n;
        packetOffset_ += n;
        need          -= n;
    }

    return paContinue;
}
