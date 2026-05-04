#include "physicsThread.hpp"
#include "audioEngine.hpp"
#include "audioPacket.hpp"
#include <chrono>
#include <algorithm>

PhysicsThread::PhysicsThread(AudioEngine& audio) : audio_(audio) {}

PhysicsThread::~PhysicsThread() { stop(); }

void PhysicsThread::start() {
    membrane_.init((float)RADIUS, (float)TENSION, (float)MATERIAL_DENSITY, GRID_R, GRID_TH);
    simRate_   = membrane_.getSimRate();
    physSteps_ = std::max(1, (int)std::ceil((double)BUFFER_SIZE * simRate_ / SAMPLE_RATE));

    // Initialise shared vertex data to the flat membrane
    vertexData_ = membrane_.getCurrentGrid();

    alive_.store(true);
    thread_ = std::thread(&PhysicsThread::run, this);
}

void PhysicsThread::stop() {
    alive_.store(false);
    strikeCV_.notify_all();
    if (thread_.joinable())
        thread_.join();
}

void PhysicsThread::enqueueStrike(const StrikeDefs& strike) {
    {
        std::lock_guard<std::mutex> lock(strikeMutex_);
        strikeQueue_.push(strike);
    }
    strikeCV_.notify_one();
}

std::vector<float> PhysicsThread::getLatestVertexData() {
    std::lock_guard<std::mutex> lock(vertexMutex_);
    return vertexData_;
}

void PhysicsThread::run() {
    while (alive_.load()) {
        // Block when idle; drain any queued strikes (supports mid-sim retrigger).
        {
            std::unique_lock<std::mutex> lock(strikeMutex_);
            if (!simRunning_.load() && strikeQueue_.empty()) {
                strikeCV_.wait(lock, [this] {
                    return !strikeQueue_.empty() || !alive_.load();
                });
            }
            while (!strikeQueue_.empty()) {
                StrikeDefs s = strikeQueue_.front();
                strikeQueue_.pop();
                lock.unlock();
                membrane_.setInitialCondition(&s);
                simRunning_.store(true);
                dB_.store(0.0f, std::memory_order_relaxed);
                lock.lock();
            }
        }

        if (!alive_.load() || !simRunning_.load()) continue;

        // --- Physics step ---
        membrane_.Simulate(physSteps_);

        // --- Resample physics buffer to audio sample rate (physics thread) ---
        auto& physBuf = membrane_.getPhysicsBuffer();
        std::vector<float> resampled = dsp_.sampleInterp(
            physBuf.data(), (int)physBuf.size(), simRate_, SAMPLE_RATE);

        // --- dB level + decay stop ---
        float dB = dsp_.calculateDecibleLevel(resampled);
        dB_.store(dB, std::memory_order_relaxed);

        // --- Update shared vertex data ---
        {
            std::lock_guard<std::mutex> vlock(vertexMutex_);
            vertexData_ = membrane_.getCurrentGrid();
        }

        if (dB <= -100.0f) {
            simRunning_.store(false);
            // Reset vertex to flat membrane
            std::lock_guard<std::mutex> vlock(vertexMutex_);
            std::fill(vertexData_.begin(), vertexData_.end(), 0.0f);
            continue;
        }

        // --- Push resampled packet to audio ring buffer ---
        if (runAudio_.load(std::memory_order_relaxed)) {
            AudioPacket pkt;
            pkt.count = std::min(resampled.size(), MAX_PACKET_SAMPLES);
            std::copy(resampled.begin(), resampled.begin() + (ptrdiff_t)pkt.count, pkt.samples);

            // Back-pressure: wait if the ring buffer is full
            while (!audio_.pushPacket(pkt) && alive_.load()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
    }
}
