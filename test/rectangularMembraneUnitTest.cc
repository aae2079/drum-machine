#include <iostream>
#include <vector>
#include "RectangularMembrane.hpp"
#include "wav.hpp"
#include "simDefs.hpp"
#include "audioEngine.hpp"
#include "JsonParser.hpp"
#include <fstream>
#include <cstdint>
#include <chrono>
#include <algorithm>
#include <cstring>
#include <string>

#define WAVE_FILE 1
#define PORT_AUDIO 0

int firstTime = 1;

void convertFloatToInt16(const std::vector<float> &input, std::vector<int16_t> &output) {
    output.resize(input.size());
    for (size_t i = 0; i < input.size(); ++i) {
        output[i] = static_cast<int16_t>(input[i] * 32767); // Scale float to int16 range
    }
}

int main(int argc, char** argv){

    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <config_file.json>" << std::endl;
        return 1;
    }

    Params params;

    parseJsonSettings(argv[1], params);
    AudioDefinitions& audioDefs = params.audio;
    int sim_time = 1;
    int num_samples = sim_time * audioDefs.sampleRate;
    int sampsProc = 0;

    #if PORT_AUDIO
    AudioEngine audio;
    audio.start();
    #endif

    // Initialize buffers
    std::vector<float> audio_buffer;
    std::vector<int16_t> int16_buffer;

    RectangularMembrane membrane(GRID_X, GRID_Y, 1.5f, 100.0f, 0.001f, sim_time);
    //Real-time mechanicism 
    while (sampsProc < num_samples) {
        auto start = std::chrono::high_resolution_clock::now();
        
        // Generate ONE chunk of 1024 samples
        membrane.Simulate();
        
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end - start;

        #if PORT_AUDIO
        audio.pushChunk(membrane.getAudioBuffer().data(),membrane.getAudioBuffer().size());
        #endif
        
        //Need to set some kind of logger here
        // std::cout << "Frame: " << gBuf.frameCount << ", Samples Processed: " << sampsProc << "/" << num_samples << std::endl;
        // std::cout << "Time taken for chunk: " << duration.count() << " ms" << std::endl;

        #if WAVE_FILE
        // Append current audio buffer to the main audio buffer
        //Be aware of overlap!
        audio_buffer.insert(audio_buffer.end(), membrane.getAudioBuffer().begin(), membrane.getAudioBuffer().end());
        convertFloatToInt16(audio_buffer, int16_buffer);
        #endif
        
        // Small delay to prevent busy-waiting
        #if PORT_AUDIO
        audio.delay();
        #endif
        sampsProc += audioDefs.bufferSize; // Account for overlap
    } 

    // Build wav file
#if WAVE_FILE
    std::string output_path = "output.wav";

    WAV_HEADER head;
    head.riff[0] = 'R';
    head.riff[1] = 'I';
    head.riff[2] = 'F';
    head.riff[3] = 'F';
    head.chunkSize = 36 + audio_buffer.size() * audioDefs.numChannels * audioDefs.bitDepth/8;

    head.wave[0] = 'W';
    head.wave[1] = 'A';
    head.wave[2] = 'V';
    head.wave[3] = 'E';
    head.fmt[0] = 'f';
    head.fmt[1] = 'm';
    head.fmt[2] = 't';
    head.fmt[3] = ' ';

    head.subchunk1Size = 16; // PCM
    head.audioFormat = audioDefs.audioFormatPCM;
    head.numChannels = audioDefs.numChannels;
    head.sampleRate = audioDefs.sampleRate;
    head.byteRate = audioDefs.byteRate;
    head.blockAlign = audioDefs.blockAlign;
    head.bitsPerSample = audioDefs.bitDepth;

    head.subchunk2ID[0] = 'd';
    head.subchunk2ID[1] = 'a';
    head.subchunk2ID[2] = 't';
    head.subchunk2ID[3] = 'a';
    
    head.subchunk2Size = audio_buffer.size() * audioDefs.bitDepth/8;


    std::ofstream outFile;
    outFile.open(output_path, std::ios::binary);
    outFile.write((char*)&head, sizeof(WAV_HEADER));
    outFile.write(reinterpret_cast<const char*>(int16_buffer.data()), int16_buffer.size());
    outFile.close();
#endif
    return 0;
}