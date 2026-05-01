#include "audioEngine.hpp"
AudioEngine::AudioEngine(float sampleRate, int bufferSize): _sampleRate(sampleRate), _bufferSize(bufferSize) {
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
    outputParameters.channelCount = (uint32_t)NUM_CHANNELS; // Mono output
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

void AudioEngine::consumeAudio(const float* buffer, size_t numSamples) {
   //push audio data into the ring buffer for playbac
    audio_buffer.assign(buffer, buffer + numSamples);
}

void AudioEngine::delay(){
    Pa_Sleep(_bufferSize / _sampleRate * 1000); //Sleep for duration of one buffer
}

int AudioEngine::paStreamCB(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer,
                    const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData) {
    return static_cast<AudioEngine*>(userData)->internalAudioCB(inputBuffer, outputBuffer, framesPerBuffer, timeInfo, statusFlags, userData);
}

void AudioEngine::paStreamFinished(void*) {
    std::cout << "PortAudio stream finished.\n";
}

int AudioEngine::internalAudioCB(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer,
                    const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData){
    
    if (outputParameters.channelCount == 1){
        //mono
        if (audio_buffer.empty()) {
            // If no audio data is available, output silence
            std::memset(outputBuffer, 0, framesPerBuffer * sizeof(float));
        } else {
            // Copy audio data to output buffer
            size_t samplesToCopy = std::min((size_t)framesPerBuffer, audio_buffer.size());
            std::memcpy(outputBuffer, audio_buffer.data(), samplesToCopy * sizeof(float));
            if (samplesToCopy < framesPerBuffer) {
                // If we have less data than the buffer size, fill the rest with silence
                std::memset(static_cast<float*>(outputBuffer) + samplesToCopy, 0, (framesPerBuffer - samplesToCopy) * sizeof(float));
            }
        }
    } else if (outputParameters.channelCount == 2){
        //stereo: duplicate mono signal to both channels
        if (audio_buffer.empty()) {
            // If no audio data is available, output silence
            std::memset(outputBuffer, 0, framesPerBuffer * 2 * sizeof(float));
        } else {
            // Copy audio data to output buffer, duplicating for stereo
            size_t samplesToCopy = std::min(framesPerBuffer, audio_buffer.size());
            float* out = static_cast<float*>(outputBuffer);
            for (size_t i = 0; i < samplesToCopy; i++) {
                *out++ = audio_buffer[i];     // Left channel
                *out++ = audio_buffer[i]; // Right channel
            }
            if (samplesToCopy < framesPerBuffer) {
                // If we have less data than the buffer size, fill the rest with silence
                std::memset(out, 0, (framesPerBuffer - samplesToCopy) * 2 * sizeof(float));
            }
        }
    }

    return paContinue;
}