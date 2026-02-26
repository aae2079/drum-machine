#ifndef WAV_HPP
#define WAV_HPP

#include <iostream>
#include <fstream>
#include <cstdio>
#include <vector>
using namespace std;


typedef struct WAV_HEADER {
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
} wavHeader;




#endif