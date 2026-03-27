#include <iostream>
#include <vector>
#include "RectangularMembrane.hpp"
#include "wav.hpp"
#include "simDefs.hpp"
#include "audioEngine.hpp"
#include <fstream>
#include <cstdint>
#include <chrono>
#include "portaudio.h"
#include <algorithm>
#include <cstring>
#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/level.h>
#include <fmt/format.h>

#define WAVE_FILE 1
#define PORT_AUDIO 0

static log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("RECTANGULAR_MEMBRANE_UNIT_TEST"));


int firstTime = 1;
void convertFloatToInt16(const std::vector<float> &input, std::vector<int16_t> &output) {
    output.resize(input.size());
    for (size_t i = 0; i < input.size(); ++i) {
        output[i] = static_cast<int16_t>(input[i] * 32767); // Scale float to int16 range
    }
}

int main(void){
    log4cxx::BasicConfigurator::configure();
    //logger->setLevel(log4cxx::Level::getInfo());
    std::string input;
    float sim_time = 2.0f;
    int num_samples = sim_time * SAMPLE_RATE;

    #if PORT_AUDIO
    AudioEngine audio;
    audio.start();
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
                auto start = std::chrono::high_resolution_clock::now();
                
                // Generate ONE chunk of 1024 samples
                membrane.Simulate();
                
                auto end = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double, std::milli> duration = end - start;

                #if PORT_AUDIO
                audio.pushChunk(membrane.getAudioBuffer().data(),membrane.getAudioBuffer().size());
                #endif
                
                LOG4CXX_DEBUG_FMT(logger,"Time taken for chunk: {}",duration.count());
                LOG4CXX_DEBUG_FMT(logger, "Samps Processed: {}/{}",sampsProc,num_samples);

                #if WAVE_FILE
                // Append current audio buffer to the main audio buffer
                //Be aware of overlap!
                audio_buffer.insert(audio_buffer.end(), membrane.getAudioBuffer().begin(), membrane.getAudioBuffer().end()-(int)OVERLAP);
                convertFloatToInt16(audio_buffer, int16_buffer);
                #endif
                
                // Small delay to prevent busy-waiting
                #if PORT_AUDIO
                audio.delay();
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