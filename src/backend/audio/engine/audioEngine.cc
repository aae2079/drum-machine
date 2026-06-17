#include "audioEngine.hpp"

AudioEngine::AudioEngine(int sampleRate, int bufferSize)
    : _sampleRate(sampleRate), _bufferSize(bufferSize)
{
    deviceManager_.initialiseWithDefaultDevices(0, 1);

    juce::AudioDeviceManager::AudioDeviceSetup setup;
    deviceManager_.getAudioDeviceSetup(setup);
    setup.sampleRate = sampleRate;
    setup.bufferSize = bufferSize;
    auto result = deviceManager_.setAudioDeviceSetup(setup, true);
    if (result.isNotEmpty())
        std::cerr << "Warning: could not set sample rate to " 
                  << sampleRate << ": " << result.toStdString() << "\n";
}

AudioEngine::~AudioEngine() {
    stop();
}

void AudioEngine::start() {
    deviceManager_.addAudioCallback(this);
}

void AudioEngine::stop() {
    deviceManager_.removeAudioCallback(this);
    // Wake any blocked pushChunk so the physics thread can exit cleanly.
    slotCV_.notify_all();
}

void AudioEngine::pushChunk(const float* buffer, size_t numSamples) {

    // Block until the target slot is free (audio callback consumed it).
    std::unique_lock<std::mutex> lock(slotMtx_);
    slotCV_.wait(lock, [this] {
        return ringBuf[fill_ix.load(std::memory_order_relaxed)].full.load(std::memory_order_acquire) == 0;
    });

    int ix = fill_ix.load(std::memory_order_relaxed);
    ringBuf[ix].audio_buffer.assign(buffer, buffer + numSamples);
    ringBuf[ix].full.store(1, std::memory_order_release);
    fill_ix.store((ix + 1) % NUM_FRAMES, std::memory_order_relaxed);
}

void AudioEngine::delay() {
    juce::Thread::sleep(_bufferSize * 1000 / _sampleRate);
}
void AudioEngine::audioDeviceAboutToStart(juce::AudioIODevice* /*device*/) {
    
}

void AudioEngine::audioDeviceStopped() {
    
}
void AudioEngine::audioDeviceIOCallbackWithContext(
    const float* const* /*inputChannelData*/,
    int /*numInputChannels*/,
    float* const* outputChannelData,
    int numOutputChannels,
    int numSamples,
    const juce::AudioIODeviceCallbackContext& /*context*/)
{
    // Clear any channels beyond the first (safety for unexpected device configs).
    for (int ch = 1; ch < numOutputChannels; ++ch)
        if (outputChannelData[ch])
            std::fill(outputChannelData[ch], outputChannelData[ch] + numSamples, 0.0f);
 
    if (outputChannelData[0])
        internalAudioCB(outputChannelData[0], numSamples);
}


void AudioEngine::internalAudioCB(float* out, int frames) {
    unsigned long filled = 0;
    while (filled < frames) {
        int ix = read_ix.load(std::memory_order_relaxed);
        Data& cur = ringBuf[ix];

        if (cur.full.load(std::memory_order_acquire) == 0) {
            // Ring buffer empty — output silence for remaining frames.
            std::fill(out + filled, out + frames, 0.0f);
            break;
        }

        int available = (int)cur.audio_buffer.size() - buf_pos;
        int needed    = (int)(frames - filled);
        int to_copy   = std::min(available, needed);

        if (muted_){
            std::fill(out + filled, out + filled + to_copy, 0.0f);
        } else {
            std::memcpy(out + filled, cur.audio_buffer.data() + buf_pos, to_copy * sizeof(float));
        }

        filled  += to_copy;
        buf_pos += to_copy;

        if (buf_pos >= (int)cur.audio_buffer.size()) {
            // Slot fully consumed — release it and notify the producer.
            cur.full.store(0, std::memory_order_release);
            read_ix.store((ix + 1) % NUM_FRAMES, std::memory_order_relaxed);
            buf_pos = 0;
            slotCV_.notify_one();
        }
    }

    
}
