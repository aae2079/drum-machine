#include <iostream>
#include <vector>
#include "RectangularMembrane.h"
#include "wav.hpp"
#include <fstream>
#include <cstdint>
#include <chrono>

#define WAVE_FILE 1

int frameCount = 0;

struct DATA {
    float buf[BUFFER_SIZE];
    int frameCount;
};
void convertFloatToInt16(const std::vector<float> &input, std::vector<int16_t> &output) {
    output.resize(input.size());
    for (size_t i = 0; i < input.size(); ++i) {
        output[i] = static_cast<int16_t>(input[i] * 32767); // Scale float to int16 range
    }
}

int main(int argc, char** argv){
    std::string input;
    float sim_time = 2.0f;
    int num_samples = sim_time * SAMPLE_RATE;
    int sampsProc = 0;

    std::cout << "Press S to start Drum Simulation: " << std::endl;
    std::cin >> input;
    RectangularMembrane membrane;

     // Store audio buffer
    std::vector<float> audio_buffer;
    std::vector<int16_t> int16_buffer;
    if (input == "S" || input == "s"){
        std::cout << "Starting Drum Simulation..." << std::endl;
        while (sampsProc <= num_samples){
            auto start = std::chrono::high_resolution_clock::now();
            membrane.Simulate();
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> duration = end - start;
            sampsProc += membrane.getAudioBuffer().size();
            frameCount++;

            std::cout << "Frame: " << frameCount << ", Samples Processed: " << sampsProc << "/" << num_samples << std::endl;
            // Append current audio buffer to the main audio buffer

            //print out time taken for each buffer processing
            std::cout << "Time taken for this buffer: " << duration.count() << " ms" << std::endl;
           
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