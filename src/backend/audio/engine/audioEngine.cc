#include "audioEngine.hpp"

AudioEngine::AudioEngine(int sampleRate, int bufferSize)
    : _sampleRate(sampleRate), _bufferSize(bufferSize)
{
    err = Pa_Initialize();
    if (err != paNoError) {
        std::cerr << "PortAudio initialization failed: " << Pa_GetErrorText(err)
                  << " (" << err << ")" << std::endl;
        Pa_Terminate();
        return;
    }

    outputParameters.device = Pa_GetDefaultOutputDevice();
    if (outputParameters.device == paNoDevice) {
        std::cerr << "No default output device." << std::endl;
        Pa_Terminate();
        return;
    }
    outputParameters.channelCount = 1;
    outputParameters.sampleFormat = paFloat32;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = nullptr;
}

AudioEngine::~AudioEngine() {
    stop();
    Pa_Terminate();
}

void AudioEngine::start() {
    err = Pa_OpenStream(&mainStream, nullptr, &outputParameters, _sampleRate, _bufferSize,
                        paClipOff, paStreamCB, this);
    if (err != paNoError) {
        std::cerr << "PortAudio open stream failed: " << Pa_GetErrorText(err)
                  << " (" << err << ")" << std::endl;
        Pa_Terminate();
        return;
    }

    err = Pa_SetStreamFinishedCallback(mainStream, paStreamFinished);
    if (err != paNoError) {
        std::cerr << "PortAudio set stream finished callback failed: " << Pa_GetErrorText(err)
                  << " (" << err << ")" << std::endl;
        Pa_CloseStream(mainStream);
        Pa_Terminate();
        return;
    }

    err = Pa_StartStream(mainStream);
    if (err != paNoError) {
        std::cerr << "PortAudio start stream failed: " << Pa_GetErrorText(err)
                  << " (" << err << ")" << std::endl;
        Pa_CloseStream(mainStream);
        Pa_Terminate();
        return;
    }
}

void AudioEngine::stop() {
    if (mainStream != nullptr) {
        err = Pa_StopStream(mainStream);
        if (err != paNoError)
            std::cerr << "PortAudio stop stream failed: " << Pa_GetErrorText(err) << std::endl;
        err = Pa_CloseStream(mainStream);
        if (err != paNoError)
            std::cerr << "PortAudio close stream failed: " << Pa_GetErrorText(err) << std::endl;
    }
    // Wake any blocked pushChunk so the physics thread can exit cleanly.
    slotCV_.notify_all();
}

void AudioEngine::pushChunk(const float* buffer, size_t numSamples) {

    // Block until the target slot is free (audio callback consumed it).
    std::unique_lock<std::mutex> lock(slotMtx_);
    slotCV_.wait(lock, [this] {
        return ringBuf[fill_ix.load(std::memory_order_relaxed)].full.load(std::memory_order_acquire) == 0;
    });

    int ix = fill_ix.load(std::memory_order_relaxed);
    ringBuf[ix].audio_buffer.assign(buffer, buffer + numSamples);
    ringBuf[ix].full.store(1, std::memory_order_release);
    fill_ix.store((ix + 1) % NUM_FRAMES, std::memory_order_relaxed);
}

void AudioEngine::delay() {
    Pa_Sleep(_bufferSize / _sampleRate * 1000);
}

int AudioEngine::paStreamCB(const void* /*inputBuffer*/, void* outputBuffer,
                             unsigned long framesPerBuffer,
                             const PaStreamCallbackTimeInfo* /*timeInfo*/,
                             PaStreamCallbackFlags /*statusFlags*/, void* userData)
{
    return static_cast<AudioEngine*>(userData)->internalAudioCB(
        static_cast<float*>(outputBuffer), framesPerBuffer);
}

void AudioEngine::paStreamFinished(void*) {
    std::cout << "PortAudio stream finished.\n";
}

int AudioEngine::internalAudioCB(float* out, unsigned long frames) {
    unsigned long filled = 0;
    while (filled < frames) {
        int ix = read_ix.load(std::memory_order_relaxed);
        Data& cur = ringBuf[ix];

        if (cur.full.load(std::memory_order_acquire) == 0) {
            // Ring buffer empty — output silence for remaining frames.
            std::fill(out + filled, out + frames, 0.0f);
            break;
        }

        int available = (int)cur.audio_buffer.size() - buf_pos;
        int needed    = (int)(frames - filled);
        int to_copy   = std::min(available, needed);

        if (muted_){
            std::fill(out + filled, out + filled + to_copy, 0.0f);
        } else {
            std::memcpy(out + filled, cur.audio_buffer.data() + buf_pos, to_copy * sizeof(float));
        }

        filled  += to_copy;
        buf_pos += to_copy;

        if (buf_pos >= (int)cur.audio_buffer.size()) {
            // Slot fully consumed — release it and notify the producer.
            cur.full.store(0, std::memory_order_release);
            read_ix.store((ix + 1) % NUM_FRAMES, std::memory_order_relaxed);
            buf_pos = 0;
            slotCV_.notify_one();
        }
    }

    return paContinue;
}
