#include "PhysicsThread.hpp"
#include "CircularMembrane.hpp"
#include <chrono>

void PhysicsThread::start(Params params) { 
    running_.store(true);
    thread_ = std::thread(&PhysicsThread::run, this, params);
}

void PhysicsThread::stop() {
    running_.store(false);
    strikeCV_.notify_all();
    if (thread_.joinable()) thread_.join();
}

void PhysicsThread::pushStrike(const StrikeDefs& strike) {
    std::lock_guard<std::mutex> lock(strikeMtx_);
    strikeQueue_.push(strike);
    strikeCV_.notify_one();
}

bool PhysicsThread::tryGetGrid(std::vector<float>& out, int timeoutMs) {
    std::unique_lock<std::mutex> lock(gridMtx_);
    bool newData = gridCV_.wait_for(lock, std::chrono::milliseconds(timeoutMs),
                                    [this]{ return gridDirty_; });
    if (newData) {
        out = latestGrid_;
        gridDirty_ = false;
    }
    return newData;
}

void PhysicsThread::run(Params params) {
    membrane_.init(params.timbre.radius, params.timbre.damping, params.timbre.tension,
                  params.timbre.material_density, params.grid.grid_r, params.grid.grid_th);
    shell_.init(params.timbre.radius, params.timbre.shell_length, membrane_.getTimeStep(), membrane_.getSpeed(),params.grid.grid_r, params.grid.grid_th, params.grid.grid_z);
    
    int physBufferSize = (int)((params.audio.bufferSize / params.audio.sampleRate) * membrane_.getSimRate());
    std::vector<float> physBuf(physBufferSize, 0.0f);
    while (running_) {
        StrikeDefs strike;
        {
            std::unique_lock<std::mutex> lock(strikeMtx_);
            strikeCV_.wait(lock, [this]{ return !strikeQueue_.empty() || !running_; });
            if (!running_) break;
            strike = strikeQueue_.front();
            strikeQueue_.pop();
            membrane_.setInitialCondition(&strike);
        }

        while (running_) {
            //check for more events coming in
            {
                std::lock_guard<std::mutex> lock(strikeMtx_);
                if (!strikeQueue_.empty()) {
                    break;
                }
            }

            std::vector<float>& membraneVelocity = membrane_.getVelocityField();
            shell_.setVelocityField(membraneVelocity);
            shell_.Simulate(physBufferSize);
            membrane_.setPressure(shell_.getMembraneBoundaryPressure());
            membrane_.Simulate(physBufferSize, physBuf);

            // Send GUI data first — don't let a full audio ring buffer delay the visual update.
            {
                std::lock_guard<std::mutex> gridLock(gridMtx_);
                latestGrid_ = membrane_.getCurrentGrid();
                gridDirty_ = true;
                gridCV_.notify_one();
            }

            // Send audio data (may block briefly if the ring buffer is full).
            sendAudioChunk(physBuf, params.audio.bufferSize, params.audio.sampleRate);

            if (!membrane_.isActive()){
                membrane_.resetStateVectors();
                shell_.resetStateVectors();
                std::cout << "completed decay" << std::endl;
                break;
            }
        }
    }
}

void PhysicsThread::sendAudioChunk(std::vector<float>& chunk, int bufferSize, float sampleRate) {
    std::vector<float> audioBuf = dspToolbox_.sampleInterp(chunk.data(), chunk.size(), bufferSize, membrane_.getSimRate(), sampleRate);
    //audioBuf = dspToolbox_.normalizeAudio(audioBuf);
    audioBuf = dspToolbox_.applyGain(audioBuf, 7.0f);
    audio_.pushChunk(audioBuf.data(), audioBuf.size());
}
