#include <iostream>
#include <vector>
#include "RectangularMembrane.h"
#include "wav.hpp"
#include <fstream>


int main(int argc, char** argv){
    std::string output_path;
    if (argc < 2){
        std::cout << "Usage: " << argv[0] << " /file/path/to/output.wav" << std::endl;
        return -1;
    }else{
        output_path = argv[1];
    }

    RectangularMembrane drum;

    drum.setInitialCondition();
    std::vector<float> audio_buffer;
    drum.Simulate(audio_buffer);
    //convert float buffer to int16_t for wav file
    std::vector<int16_t> int16_buffer = reinterpret_cast<std::vector<int16_t>&>(audio_buffer);


    // Build wav file
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
    outFile.write((char*)int16_buffer.data(), int16_buffer.size() * sizeof(int16_t));
    outFile.close();
    
    // //Write out the audio buffer to a file
    // FILE *fp = fopen(output_path.c_str(), "wb");
    // fwrite(audio_buffer.data(),1, audio_buffer.size() * sizeof(float), fp);
    // fclose(fp);
}