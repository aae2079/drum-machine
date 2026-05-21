#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <atomic>
#include "simDefs.hpp"
#include "strikeDefs.hpp"
#include "audioEngine.hpp"
#include "audioDSP.hpp"
#include "CircularMembrane.hpp"
#include "CylindricalResonator.hpp"

class PhysicsThread {
public:
    PhysicsThread(AudioEngine& audio) : audio_(audio) {}
    ~PhysicsThread() {
        stop();
    }
    void start(Params params);
    void stop();
    void pushStrike(const StrikeDefs& strike);
    bool tryGetGrid(std::vector<float>& out, int timeoutMs);

private:
    void run(Params params);
    void sendAudioChunk(std::vector<float>& chunk, int bufferSize, float sampleRate);

    CircularMembrane membrane_;
    CylindricalResonator shell_;

    std::queue<StrikeDefs> strikeQueue_;
    std::mutex strikeMtx_;
    std::condition_variable strikeCV_;

    std::vector<float> latestGrid_;
    bool gridDirty_ = false;
    std::mutex gridMtx_;
    std::condition_variable gridCV_;

    std::atomic<bool> running_{false};
    std::thread thread_;

    AudioEngine& audio_;
    AudioDSP_Toolbox dspToolbox_;
};
