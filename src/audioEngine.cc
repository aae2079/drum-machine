#include "audioEngine.hpp"
AudioEngine::AudioEngine(int sampleRate, int bufferSize): _sampleRate(sampleRate), _bufferSize(bufferSize) {
    // Initialize PortAudio or other audio resources
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
    outputParameters.channelCount = 1; // Mono output
    outputParameters.sampleFormat = paFloat32; // 32-bit float output
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = nullptr;
}
AudioEngine::~AudioEngine() {
    stop();
    Pa_Terminate();
}

void AudioEngine::start() {
    // Start the audio stream
    err = Pa_OpenStream(&mainStream, nullptr, &outputParameters, _sampleRate, _bufferSize, paClipOff, paStreamCB, this);
    if (err != paNoError) {
        std::cerr << "PortAudio open stream failed: " << Pa_GetErrorText(err) << " (" << err << ")" << std::endl;
        Pa_Terminate();
        return;
    }

    err = Pa_SetStreamFinishedCallback(mainStream, paStreamFinished);
    if (err != paNoError) {
        std::cerr << "PortAudio set stream finished callback failed: " << Pa_GetErrorText(err) << " (" << err << ")" << std::endl;
        Pa_CloseStream(mainStream);
        Pa_Terminate();
        return;
    }

    err = Pa_StartStream(mainStream);
    if (err != paNoError) {
        std::cerr << "PortAudio start stream failed: " << Pa_GetErrorText(err) << " (" << err << ")" << std::endl;
        Pa_CloseStream(mainStream);
        Pa_Terminate();
        return;
    }
}

void AudioEngine::stop() {
    // Stop and close stream
    if (mainStream != nullptr) {
        err = Pa_StopStream(mainStream);
        if (err != paNoError) {
            std::cerr << "PortAudio stop stream failed: " << Pa_GetErrorText(err) << std::endl;
        }
        err = Pa_CloseStream(mainStream);
        if (err != paNoError) {
            std::cerr << "PortAudio close stream failed: " << Pa_GetErrorText(err) << std::endl;
        }
    }
    std::cout << "Simulation complete. Waiting for audio playback to finish..." << std::endl;
    Pa_Sleep(2000); // Wait 2 seconds for audio to finish playing
}

void AudioEngine::pushChunk(const float* buffer, size_t numSamples) {
   //push audio data into the ring buffer for playbac
    Data cur;
    cur.audio_buffer.assign(buffer, buffer + numSamples);
    cur.frameCount = frame++;
    while (ringBuf[fill_ix].frameCount != 0) {
        Pa_Sleep(1); // Wait for the callback to consume the slot
    }
    ringBuf[fill_ix] = cur; // Copy current buffer to ring buffer
    fill_ix = (fill_ix + 1) % NUM_FRAMES;
}

void AudioEngine::delay(){
    Pa_Sleep(_bufferSize / _sampleRate * 1000); //Sleep for duration of one buffer
}

int AudioEngine::paStreamCB(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer,
                    const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData) {
    return static_cast<AudioEngine*>(userData)->internalAudioCB(static_cast<float*>(outputBuffer),framesPerBuffer);
}

void AudioEngine::paStreamFinished(void*) {
    std::cout << "PortAudio stream finished.\n";
}

int AudioEngine::internalAudioCB(float *out, unsigned long frames){
    // Implement the audio callback function
    unsigned long filled = 0;
    
    while(filled < frames){
        Data &cur = ringBuf[read_ix];

        if (cur.audio_buffer.empty() || cur.frameCount == 0){
            // If no data, output silence
            std::fill(out + filled, out + frames, 0.0f);
            break;
        }

        int available = (int)cur.audio_buffer.size() - buf_pos;
        int needed    = (int)(frames - filled);
        int to_copy   = std::min(available, needed);

        std::memcpy(out + filled, cur.audio_buffer.data() + buf_pos, to_copy * sizeof(float));

        filled  += to_copy;
        buf_pos += to_copy;

        // Current Data chunk exhausted — advance ring buffer
        if (buf_pos >= (int)cur.audio_buffer.size()) {
            cur.frameCount = 0;           // Mark slot as consumed so main thread can reuse
            cur.audio_buffer.clear();
            read_ix = (read_ix + 1) % NUM_FRAMES;
            buf_pos = 0;
        }
    }

    return paContinue;
}