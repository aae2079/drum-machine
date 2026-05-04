#ifndef AUDIO_PACKET_HPP
#define AUDIO_PACKET_HPP

#include <cstddef>
#include "audioDefs.hpp"

// Extra headroom beyond BUFFER_SIZE to absorb ceiling rounding in sampleInterp.
constexpr size_t MAX_PACKET_SAMPLES = BUFFER_SIZE + 8;

// Fixed-size audio packet pushed by the physics thread and consumed by the
// PortAudio callback. No heap allocation on either end.
struct AudioPacket {
    float  samples[MAX_PACKET_SAMPLES] = {};
    size_t count = 0;
};

#endif // AUDIO_PACKET_HPP
