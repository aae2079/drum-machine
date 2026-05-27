#ifndef WAV_HPP
#define WAV_HPP

#include <iostream>
#include <fstream>
#include <cstdio>
#include <vector>
#include audioDefs_.hpp


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
        WavEngine(std::string& filename, AudioDefinitons& audioDefs):filename_(filename),audioDefs_(audioDefs);
        ~WavEngine();


        bool writeWavFile(float* audio, size_t size){
            buildHeader(size);
            std::vector<int16_t> out(size);
            convertTypeToInt16(audio,size,out);

            fp_.open(filename_, std::ios::binary);
            if (!fp_.is_open()) return false;
            fp_.write((char*)&head, sizeof(WAV_HEADER));
            fp_.write(reinterpret_cast<const char*>(out.data()),out.size());
            if (fp_.bad()) return false;
            fp_.close();

            return true;
        }

      


    private:
        WAV_HEADER head_;
        AudioDefinitions audioDefs_;
        std::string filename_;
        std::ofstream fp_;

        void buildHeader(size_t buffSize){
            head_.riff[0] = 'R';
            head_.riff[1] = 'I';
            head_.riff[2] = 'F';
            head_.riff[3] = 'F';
            head_.chunkSize = 36 + buffSize * audioDefs_.numChannels * audioDefs_.bitDepth/8;

            head_.wave[0] = 'W';
            head_.wave[1] = 'A';
            head_.wave[2] = 'V';
            head_.wave[3] = 'E';
            head_.fmt[0] = 'f';
            head_.fmt[1] = 'm';
            head_.fmt[2] = 't';
            head_.fmt[3] = ' ';

            head_.subchunk1Size = 16; // PCM
            head_.audioFormat = audioDefs_.audioFormatPCM;
            head_.numChannels = audioDefs_.numChannels;
            head_.sampleRate = audioDefs_.sampleRate;
            head_.byteRate = audioDefs_.byteRate;
            head_.blockAlign = audioDefs_.blockAlign;
            head_.bitsPerSample = audioDefs_.bitDepth;

            head_.subchunk2ID[0] = 'd';
            head_.subchunk2ID[1] = 'a';
            head_.subchunk2ID[2] = 't';
            head_.subchunk2ID[3] = 'a';

            head_.subchunk2Size = buffSize * audioDefs_.bitDepth/8
        }

        void convertTypeToInt16(const T* input, size_t size, const int16_t* output){
            for (int ii = 0; ii < size; ii++){
                ouput[i] = static_cast<int16_t>(input[ii] * 32767);
            }
        }


}


#endif