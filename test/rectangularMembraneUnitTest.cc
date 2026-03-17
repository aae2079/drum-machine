#include <iostream>
#include <vector>
#include "RectangularMembrane.hpp"
#include "wav.hpp"
#include "simDefs.hpp"
#include <fstream>
#include <cstdint>
#include <chrono>
#include "portaudio.h"
#include <algorithm>
#include <cstring>

#define WAVE_FILE 0
#define PORT_AUDIO 1

#define NUM_FRAMES 5

struct Data{
    std::vector<float> audio_buffer; 
    int frameCount{0};
};

std::vector<Data> ringBuf(NUM_FRAMES);
int fill_ix = 0;
int read_ix = 0;
int buf_pos = 0;

Data gBuf;
int firstTime = 1;

void convertFloatToInt16(const std::vector<float> &input, std::vector<int16_t> &output) {
    output.resize(input.size());
    for (size_t i = 0; i < input.size(); ++i) {
        output[i] = static_cast<int16_t>(input[i] * 32767); // Scale float to int16 range
    }
}

#if PORT_AUDIO

static int paStreamCB(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer,
                    const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData) {
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

static void paStreamFinished(void *userData) {
    std::cout << "PortAudio stream finished callback called." << std::endl;
}
#endif

int main(int argc, char** argv){

    #if PORT_AUDIO
    PaStreamParameters outputParameters;
    PaStream *stream;
    PaError err;

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
    #endif

    std::string input;
    float sim_time = 2.0f;
    int num_samples = sim_time * SAMPLE_RATE;




    // Initialize the stream ONCE before the loop
    #if PORT_AUDIO
    PaStream *mainStream = nullptr;

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
    
    #endif

    // Initialize buffers
    std::vector<float> audio_buffer;
    std::vector<int16_t> int16_buffer;

    //Real-time mechanicism 
    while (1){

        if (firstTime){
            std::cout << "Press S to start Drum Simulation: (E to exit) " << std::endl;
            firstTime = 0;
        }else{
            std::cout << "Press S to go again! (E to exit) " << std::endl;
        }
        
        std::cin >> input;
        
        if (input == "S" || input == "s"){
            std::cout << "Starting Drum Simulation..." << std::endl;
            int sampsProc = 0;
            RectangularMembrane membrane;
            while (sampsProc < num_samples) {
                gBuf.audio_buffer.clear();
                auto start = std::chrono::high_resolution_clock::now();
                
                // Generate ONE chunk of 1024 samples
                membrane.Simulate();
                
                auto end = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double, std::milli> duration = end - start;

                //Fill msg
                gBuf.audio_buffer = membrane.getAudioBuffer();
                gBuf.frameCount++;
                while (ringBuf[fill_ix].frameCount != 0) {
                    Pa_Sleep(1); // Wait for the callback to consume the slot
                }
                
                ringBuf[fill_ix] = gBuf; // Copy current buffer to ring buffer
                fill_ix = (fill_ix + 1) % NUM_FRAMES; // Increment fill index with wrap-around
                
                // // Reset frame count for callback
                // std::cout << "Frame: " << gBuf.frameCount << ", Samples Processed: " << sampsProc << "/" << num_samples << std::endl;
                // std::cout << "Time taken for chunk: " << duration.count() << " ms" << std::endl;

                #if WAVE_FILE
                // Append current audio buffer to the main audio buffer
                //Be aware of overlap!
                audio_buffer.insert(audio_buffer.end(), membrane.getAudioBuffer().begin(), membrane.getAudioBuffer().end()-(int)OVERLAP);
                convertFloatToInt16(audio_buffer, int16_buffer);
                #endif

                
                
                // Small delay to prevent busy-waiting
                #if PORT_AUDIO
                Pa_Sleep(BUFFER_SIZE / SAMPLE_RATE * 1000); // Sleep for the duration of one buffer
                #endif

                sampsProc += BUFFER_SIZE - (int)OVERLAP; // Account for overlap
            }
        } else if(input == "E" || input == "e"){
            std::cout << "Exiting Drum Simulation..." << std::endl;
            break;
        } else {
            std::cout << "Invalid input. Please press S to start or E to exit." << std::endl;
        }

    }

    #if PORT_AUDIO
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

    Pa_Terminate();
    #endif



    // Build wav file
#if WAVE_FILE

    std::string output_path = "output.wav";

    WAV_HEADER head;
    head.riff[0] = 'R';
    head.riff[1] = 'I';
    head.riff[2] = 'F';
    head.riff[3] = 'F';
    head.chunkSize = 36 + audio_buffer.size() * NUM_CHANNELS * BIT_DEPTH/8;

    head.wave[0] = 'W';
    head.wave[1] = 'A';
    head.wave[2] = 'V';
    head.wave[3] = 'E';
    head.fmt[0] = 'f';
    head.fmt[1] = 'm';
    head.fmt[2] = 't';
    head.fmt[3] = ' ';

    head.subchunk1Size = SUBCHUNK1SIZE;
    head.audioFormat = AUDIO_FORMAT_PCM;
    head.numChannels = NUM_CHANNELS;
    head.sampleRate = SAMPLE_RATE;
    head.byteRate = BYTE_RATE;
    head.blockAlign = BLOCK_ALIGN;
    head.bitsPerSample = BIT_DEPTH;

    head.subchunk2ID[0] = 'd';
    head.subchunk2ID[1] = 'a';
    head.subchunk2ID[2] = 't';
    head.subchunk2ID[3] = 'a';
    
    head.subchunk2Size = audio_buffer.size() * BIT_DEPTH/8;


    std::ofstream outFile;
    outFile.open(output_path, std::ios::binary);
    outFile.write((char*)&head, sizeof(WAV_HEADER));
    outFile.write(reinterpret_cast<const char*>(int16_buffer.data()), int16_buffer.size());
    outFile.close();
#endif

    

    return 0;
}