#pragma once

#include <cstddef>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <rasterm/Engine.hpp>
#include <thread>
#include <vector>

namespace sn
{
class rastermPresenter
{
public:
    rastermPresenter() = default;
    ~rastermPresenter();

    rastermPresenter(const rastermPresenter&)             = delete;
    rastermPresenter&  operator=(const rastermPresenter&) = delete;

    [[nodiscard]] bool start(int width, int height);
    void stop() noexcept;
    void submit(const std::uint8_t* rgba, std::ptrdiff_t stride);

private:
    void presentLoop();

    rasterm::Engine m_engine;
    std::thread m_thread;
    std::mutex m_mutex;
    std::condition_variable m_frameReady;
    std::vector<std::uint8_t> m_pendingFrame;
    std::vector<std::uint8_t> m_renderFrame;
    std::uint64_t m_pendingGeneration = 0;
    int m_width = 0;
    int m_height = 0;
    bool m_running = false;
    bool m_stopping = false;
};
}