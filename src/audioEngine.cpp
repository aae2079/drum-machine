#include "audioEngine.hpp"
AudioEngine::AudioEngine() {
    // Constructor implementation
}
AudioEngine::~AudioEngine() {
    // Destructor implementation
}
void AudioEngine::Initialize() {
    // Initialize PortAudio or other audio resources
    err = Pa_Initialize();
    if (err != paNoError) {
        std::cerr << "PortAudio initialization failed: " << Pa_GetErrorText(err)
                    << " (" << err << ")" << std::endl;
        Pa_Terminate();
        return -1;
    }

    outputParameters.device = Pa_GetDefaultOutputDevice();
    if (outputParameters.device == paNoDevice) {
        std::cerr << "No default output device." << std::endl;
        Pa_Terminate();
        return -1;
    }
    outputParameters.channelCount = 1; // Mono output
    outputParameters.sampleFormat = paFloat32; // 32-bit float output
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = nullptr;

    
}   

void AudioEngine::StartStream() {
    // Start the audio stream
    err = Pa_OpenStream(&mainStream, nullptr, &outputParameters, SAMPLE_RATE, BUFFER_SIZE, paClipOff, paStreamCB, &ringBuf);
    if (err != paNoError) {
        std::cerr << "PortAudio open stream failed: " << Pa_GetErrorText(err) << " (" << err << ")" << std::endl;
        Pa_Terminate();
        return -1;
    }

    err = Pa_SetStreamFinishedCallback(mainStream, paStreamFinished);
    if (err != paNoError) {
        std::cerr << "PortAudio set stream finished callback failed: " << Pa_GetErrorText(err) << " (" << err << ")" << std::endl;
        Pa_CloseStream(mainStream);
        Pa_Terminate();
        return -1;
    }

    err = Pa_StartStream(mainStream);
    if (err != paNoError) {
        std::cerr << "PortAudio start stream failed: " << Pa_GetErrorText(err) << " (" << err << ")" << std::endl;
        Pa_CloseStream(mainStream);
        Pa_Terminate();
        return -1;
    }
}

static int AudioEngine::paStreamCB(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer,
                    const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData) {
    // Implement the audio callback function
    std::vector<Data> *ringBufPtr = static_cast<std::vector<Data>*>(userData);
    float *out = static_cast<float*>(outputBuffer);
    unsigned long filled = 0;
    while(filled < framesPerBuffer){
        Data &cur = (*ringBufPtr)[read_ix];

        if (cur.audio_buffer.empty() || cur.frameCount == 0){
            // If no data, output silence
            std::fill(out + filled, out + framesPerBuffer, 0.0f);
            break;
        }

        int available = (int)cur.audio_buffer.size() - buf_pos;
        int needed    = (int)(framesPerBuffer - filled);
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