#ifndef PHYSICS_THREAD_HPP
#define PHYSICS_THREAD_HPP

#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include "CircularMembrane.hpp"
#include "audioDSP.hpp"
#include "strikeDefs.hpp"
#include "simDefs.hpp"
#include "audioDefs.hpp"

class AudioEngine;

class PhysicsThread {
public:
    explicit PhysicsThread(AudioEngine& audio);
    ~PhysicsThread();

    void start();
    void stop();

    // Called from main thread (mouse callback).
    void enqueueStrike(const StrikeDefs& strike);

    // Called from main thread (key callback).
    void setRunAudio(bool v) { runAudio_.store(v, std::memory_order_relaxed); }

    // Called from main thread each render frame.
    std::vector<float> getLatestVertexData();
    float              getdB()        const { return dB_.load(std::memory_order_relaxed); }
    bool               isSimRunning() const { return simRunning_.load(std::memory_order_relaxed); }

private:
    void run();

    AudioEngine&       audio_;
    CircularMembrane   membrane_;
    AudioDSP_Toolbox   dsp_;

    std::thread              thread_;
    std::atomic<bool>        alive_{false};
    std::atomic<bool>        simRunning_{false};
    std::atomic<bool>        runAudio_{true};
    std::atomic<float>       dB_{0.0f};

    std::mutex               strikeMutex_;
    std::condition_variable  strikeCV_;
    std::queue<StrikeDefs>   strikeQueue_;

    std::mutex               vertexMutex_;
    std::vector<float>       vertexData_;

    int   physSteps_ = 1;
    float simRate_   = 1.0f;
};

#endif // PHYSICS_THREAD_HPP
