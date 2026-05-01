#ifndef DRUM_MACHINE_AUDIO_DEFS_H
#define DRUM_MACHINE_AUDIO_DEFS_H


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