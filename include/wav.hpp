#ifndef WAV_HPP
#define WAV_HPP

#include <iostream>
#include <fstream>
#include <cstdio>
#include <vector>
#include audioDefs.hpp


struct WAV_HEADER {
    /* RIFF Chunk Descriptor */
    char    riff[4];
    int32_t chunkSize;
    char    wave[4];
    /*"fmt" sub-chunk */
    char    fmt[4];
    int32_t subchunk1Size;
    int16_t audioFormat;
    int16_t numChannels;
    int32_t sampleRate;
    int32_t byteRate;
    int16_t blockAlign;
    int16_t bitsPerSample;
    /*"data" sub-chunk */
    char    subchunk2ID[4];
    int32_t subchunk2Size;
};

/*
WAV I/O class
*/
template <typename T>
class WavEngine{
    public:
        WavEngine(std::string& filename);
        ~WavEngine();
        void buildHeader(AudioDefinitions& audioDefs){
            
        }



    private:
        WAV_HEADER wav_;
        void convertTypeToInt16(const std::vector<T> &input, std::vector<int16_t> &output){
            output.resize(input.size());
            for (int ii = 0; ii < input.size(); ii++){
                ouput[i] = static_cast<int16_t>(input[ii] * 32767);
            }
        }
}


#endif