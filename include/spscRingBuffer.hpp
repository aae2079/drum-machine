#ifndef SPSC_RING_BUFFER_HPP
#define SPSC_RING_BUFFER_HPP

#include <array>
#include <atomic>

// Single-producer / single-consumer lock-free ring buffer.
// push() is called from the producer thread only.
// pop()  is called from the consumer thread only.
template<typename T, size_t N>
class SPSCRingBuffer {
    static_assert(N >= 2, "capacity must be >= 2");
    std::array<T, N> buf_;
    alignas(64) std::atomic<size_t> head_{0}; // written by producer
    alignas(64) std::atomic<size_t> tail_{0}; // written by consumer

public:
    // Returns false when full (producer thread).
    bool push(const T& item) {
        size_t h    = head_.load(std::memory_order_relaxed);
        size_t next = (h + 1) % N;
        if (next == tail_.load(std::memory_order_acquire))
            return false;
        buf_[h] = item;
        head_.store(next, std::memory_order_release);
        return true;
    }

    // Returns false when empty (consumer thread).
    bool pop(T& item) {
        size_t t = tail_.load(std::memory_order_relaxed);
        if (t == head_.load(std::memory_order_acquire))
            return false;
        item = buf_[t];
        tail_.store((t + 1) % N, std::memory_order_release);
        return true;
    }

    bool empty() const {
        return tail_.load(std::memory_order_acquire) ==
               head_.load(std::memory_order_acquire);
    }
};

#endif // SPSC_RING_BUFFER_HPP
