#ifndef DRUM_MACHINE_AUDIO_DEFS_H
#define DRUM_MACHINE_AUDIO_DEFS_H

// #define BYTE_RATE (SAMPLE_RATE * NUM_CHANNELS * BIT_DEPTH / 8)
// #define BLOCK_ALIGN (NUM_CHANNELS * BIT_DEPTH / 8)

typedef struct {
    int subchunk1Size;
    int audioFormatPCM;
    float sampleRate;
    int bitDepth;
    int numChannels;
    int byteRate;
    int blockAlign;
}AudioDefinitions;

#endif //DRUM_MACHINE_AUDIO_DEFS_H