#include <iostream>
#include <vector>
#include "RectangularMembrane.h"
#include "wav.hpp"
#include <fstream>
#include <cstdint>
#include <chrono>
#include "portaudio.h"

#define WAVE_FILE 0

struct Data{
    std::vector<float> audio_buffer;
    int frameCount{0};
};

Data gBuf;

void convertFloatToInt16(const std::vector<float> &input, std::vector<int16_t> &output) {
    output.resize(input.size());
    for (size_t i = 0; i < input.size(); ++i) {
        output[i] = static_cast<int16_t>(input[i] * 32767); // Scale float to int16 range
    }
}

static int paStreamCB(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer,
                    const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData) {
    Data *data = (Data*)userData;
    float *out = (float*)outputBuffer;

    // Copy audio data from the global buffer to the output buffer
    for (unsigned long i = 0; i < framesPerBuffer; i++) {
        if (i < data->audio_buffer.size()) {
            out[i] = data->audio_buffer[i];
        } else {
            out[i] = 0.0f; // Fill remaining buffer with silence
        }
    }

    return paContinue;
}

static void paStreamFinished(void *userData) {
    std::cout << "PortAudio stream finished callback called." << std::endl;
}


int main(int argc, char** argv){
    PaStreamParameters outputParameters;
    PaStream *stream;
    PaError err;

    err = Pa_Initialize();
    if (err != paNoError) {
        std::cerr << "PortAudio initialization failed: " << Pa_GetErrorText(err)
                    << " (" << err << ")" << std::endl;
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

    std::string input;
    float sim_time = 2.0f;
    int num_samples = sim_time * SAMPLE_RATE;
    int sampsProc = 0;

    std::cout << "Press S to start Drum Simulation: " << std::endl;
    std::cin >> input;
    RectangularMembrane membrane;

    // initialize buffers
    std::vector<float> audio_buffer;
    std::vector<int16_t> int16_buffer;
    if (input == "S" || input == "s"){
        std::cout << "Starting Drum Simulation..." << std::endl;
        while (sampsProc <= num_samples){
            gBuf.audio_buffer.clear();
            auto start = std::chrono::high_resolution_clock::now();
            membrane.Simulate();
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> duration = end - start;
            sampsProc += membrane.getAudioBuffer().size();
            //copy current audio buffer to global buffer for streaming
            gBuf.audio_buffer = membrane.getAudioBuffer();
            gBuf.frameCount++;
            std::cout << "Frame: " << gBuf.frameCount << ", Samples Processed: " << sampsProc << "/" << num_samples << std::endl;
            std::cout << "Time taken for this buffer: " << duration.count() << " ms" << std::endl;

            #if WAVE_FILE
            // Append current audio buffer to the main audio buffer
            audio_buffer.insert(audio_buffer.end(), membrane.getAudioBuffer().begin(), membrane.getAudioBuffer().end());
            convertFloatToInt16(audio_buffer, int16_buffer);
            #endif
            
            // Stream audio to output device
            err = Pa_OpenStream(&stream, nullptr, &outputParameters, SAMPLE_RATE, BUFFER_SIZE, paClipOff, paStreamCB, &gBuf);
            if (err != paNoError) {
                std::cerr << "PortAudio open stream failed: " << Pa_GetErrorText(err) << " (" << err << ")" << std::endl;
                break;
            }

            err = Pa_SetStreamFinishedCallback(stream, paStreamFinished);
            if (err != paNoError) {
                std::cerr << "PortAudio set stream finished callback failed: " << Pa_GetErrorText(err) << " (" << err << ")" << std::endl;
                break;
            }

            err = Pa_StartStream(stream);
            if (err != paNoError) {
                std::cerr << "PortAudio start stream failed: " << Pa_GetErrorText(err) << " (" << err << ")" << std::endl;
                break;
            }

            err = Pa_StopStream(stream);
            if (err != paNoError) {
                std::cerr << "PortAudio stop stream failed: " << Pa_GetErrorText(err) << " (" << err << ")" << std::endl;
                break;
            }
            err = Pa_CloseStream(stream);
            if (err != paNoError) {
                std::cerr << "PortAudio close stream failed: " << Pa_GetErrorText(err) << " (" << err << ")" << std::endl;
                break;
            }
            Pa_Terminate();
            std::cout << "Finished streaming audio buffer." << std::endl;
        }
    } else {
        std::cout << "Invalid input. Exiting." << std::endl;
        return 0;
    }



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