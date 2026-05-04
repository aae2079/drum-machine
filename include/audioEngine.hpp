#ifndef AUDIO_ENGINE_HPP
#define AUDIO_ENGINE_HPP

#include <iostream>
#include <cstring>
#include "portaudio.h"
#include "audioDefs.hpp"
#include "audioPacket.hpp"
#include "spscRingBuffer.hpp"

class AudioEngine {
public:
    AudioEngine(float sampleRate = SAMPLE_RATE, int bufferSize = BUFFER_SIZE);
    ~AudioEngine();

    void start();
    void stop();

    // Called by PhysicsThread to push a resampled, ready-to-play packet.
    // Returns false when the ring buffer is full (caller should retry).
    bool pushPacket(const AudioPacket& pkt);

private:
    static int  paStreamCB(const void* in, void* out, unsigned long frames,
                           const PaStreamCallbackTimeInfo*, PaStreamCallbackFlags, void* ud);
    static void paStreamFinished(void* ud);
    int internalAudioCB(void* out, unsigned long frames);

    PaStream*           mainStream_ = nullptr;
    PaStreamParameters  outputParameters_;
    PaError             err_;

    // SPSC ring buffer: physics thread produces, PortAudio callback consumes.
    SPSCRingBuffer<AudioPacket, 16> packetBuf_;

    // Partially-consumed packet held across callback invocations (callback thread only).
    AudioPacket currentPkt_{};
    size_t      packetOffset_ = 0;

    float _sampleRate;
    int   _bufferSize;
};

#endif // AUDIO_ENGINE_HPP
