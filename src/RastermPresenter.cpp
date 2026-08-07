#include "RastermPresenter.h"

#include <cstring>

namespace sn
{
rastermPresenter::~rastermPresenter()
{
    stop();
}

bool rastermPresenter::start(const int width, const int height)
{
    stop();
    if (width <= 0 || height <= 0 || !m_engine.initialize({
            .quality = rasterm::QualityProfile::Realtime,
            .useAlternateScreen = true,
            .preserveCursor = true,
            .enableDirtyRegions = false,
        }))
    {
        return false;
    }

    m_width = width;
    m_height = height;
    const auto frameBytes = static_cast<std::size_t>(width) * height * 4;
    m_pendingFrame.assign(frameBytes, 0);
    m_renderFrame.assign(frameBytes, 0);
    m_pendingGeneration = 0;
    m_stopping = false;
    m_running = true;
    m_engine.clear();
    m_thread = std::thread(&rastermPresenter::presentLoop, this);
    return true;
}

void rastermPresenter::stop() noexcept
{
    {
        std::lock_guard lock(m_mutex);
        if (!m_running)
        {
            m_engine.shutdown();
            return;
        }
        m_stopping = true;
    }
    m_frameReady.notify_one();
    if (m_thread.joinable())
        m_thread.join();

    m_running = false;
    m_engine.shutdown();
}

void rastermPresenter::submit(const std::uint8_t* rgba, const std::ptrdiff_t stride)
{
    if (rgba == nullptr || stride < static_cast<std::ptrdiff_t>(m_width) * 4)
        return;

    {
        std::lock_guard lock(m_mutex);
        if (!m_running || m_stopping)
            return;

        const auto rowBytes = static_cast<std::size_t>(m_width) * 4;
        for (int y = 0; y < m_height; ++y)
        {
            std::memcpy(m_pendingFrame.data() + static_cast<std::size_t>(y) * rowBytes,
                        rgba + static_cast<std::ptrdiff_t>(y) * stride,
                        rowBytes);
        }
        ++m_pendingGeneration;
    }
    m_frameReady.notify_one();
}

void rastermPresenter::presentLoop()
{
    std::uint64_t renderedGeneration = 0;
    while (true)
    {
        {
            std::unique_lock lock(m_mutex);
            m_frameReady.wait(lock, [&] {
                return m_stopping || m_pendingGeneration != renderedGeneration;
            });
            if (m_stopping)
                return;

            m_renderFrame.swap(m_pendingFrame);
            renderedGeneration = m_pendingGeneration;
        }

        m_engine.renderFrame(m_renderFrame.data(),
                             m_width,
                             m_height,
                             static_cast<std::ptrdiff_t>(m_width) * 4,
                             rasterm::PixelFormat::RGBA32);
    }
}
}