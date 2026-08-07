#include "Emulator.h"
#include "APU/Constants.h"
#include "Log.h"

#include <chrono>
#include <thread>

#include <windows.h>

namespace sn
{
using Clock = std::chrono::steady_clock;

namespace
{
bool pressedOnce(const Key key, bool& previous) noexcept
{
    const bool current = keyPressed(key);
    const bool pressed = current && !previous;
    previous = current;
    return pressed;
}
}

Emulator::Emulator()
  : m_cpu(m_bus)
  , m_audioPlayer(static_cast<int>(1.0 / apu_clock_period_s.count()))
  , m_ppu(m_pictureBus, m_emulatorScreen)
  , m_apu(m_audioPlayer, m_cpu.createIRQHandler(), [&](Address addr) { return DMCDMA(addr); })
  , m_bus(m_ppu, m_apu, m_controller1, m_controller2, [&](Byte b) { OAMDMA(b); })
  , m_lastWakeup()
{
    m_ppu.setInterruptCallback([&]() { m_cpu.nmiInterrupt(); });
}

void Emulator::run(std::string rom_path)
{
    if (!m_cartridge.loadFromFile(rom_path))
        return;

    m_mapper = Mapper::createMapper(static_cast<Mapper::Type>(m_cartridge.getMapper()),
                                    m_cartridge,
                                    m_cpu.createIRQHandler(),
                                    [&]() { m_pictureBus.updateMirroring(); });
    if (!m_mapper)
    {
        LOG(Error) << "Creating Mapper failed. Probably unsupported." << std::endl;
        return;
    }

    if (!m_bus.setMapper(m_mapper.get()) || !m_pictureBus.setMapper(m_mapper.get()))
    {
        return;
    }

    m_cpu.reset();
    m_ppu.reset();

    m_emulatorScreen.create(NESVideoWidth, NESVideoHeight, Color(0xffffffff));
    if (!m_presenter.start(NESVideoWidth, NESVideoHeight))
    {
        LOG(Error) << "rasterm failed to initialize Windows terminal output." << std::endl;
        return;
    }

    m_lastWakeup  = Clock::now();
    m_elapsedTime = m_lastWakeup - m_lastWakeup;

    m_audioPlayer.start();

    bool pause = false;
    bool escapeWasPressed = false;
    bool f2WasPressed = false;
    bool f3WasPressed = false;
    bool f4WasPressed = false;
    bool f5WasPressed = false;
    std::uint64_t renderedFrame = 0;
    while (true)
    {
        if (pressedOnce(VK_ESCAPE, escapeWasPressed))
        {
            return;
        }
        if (pressedOnce(VK_F2, f2WasPressed))
        {
            pause = !pause;
            if (!pause)
            {
                m_lastWakeup = Clock::now();
                LOG(Info) << "Unpaused." << std::endl;
            }
            else
            {
                LOG(Info) << "Paused." << std::endl;
            }
        }
        if (pressedOnce(VK_F4, f4WasPressed))
            Log::get().setLevel(Info);
        if (pressedOnce(VK_F5, f5WasPressed))
            Log::get().setLevel(InfoVerbose);

        if (pause)
        {
            if (pressedOnce(VK_F3, f3WasPressed))
            {
                for (int i = 0; i < 29781; ++i)
                {
                    m_ppu.step(); m_ppu.step(); m_ppu.step();
                    m_cpu.step();
                    m_apu.step();
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }

        const auto now = Clock::now();
        m_elapsedTime += now - m_lastWakeup;
        m_lastWakeup = now;
        while (m_elapsedTime > cpu_clock_period_ns)
        {
            m_ppu.step(); m_ppu.step(); m_ppu.step();
            m_cpu.step();
            m_apu.step();
            m_elapsedTime -= cpu_clock_period_ns;
        }

        if (m_emulatorScreen.frameNumber() != renderedFrame)
        {
            renderedFrame = m_emulatorScreen.frameNumber();
            m_presenter.submit(m_emulatorScreen.data(), m_emulatorScreen.stride());
        }
        else
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void Emulator::OAMDMA(Byte page)
{
    m_cpu.skipOAMDMACycles();
    auto page_ptr = m_bus.getPagePtr(page);
    if (page_ptr != nullptr)
    {
        m_ppu.doDMA(page_ptr);
    }
    else
    {
        LOG(Error) << "Can't get pageptr for DMA" << std::endl;
    }
}

Byte Emulator::DMCDMA(Address addr)
{
    m_cpu.skipDMCDMACycles();
    return m_bus.read(addr);
};

void Emulator::setKeys(const std::vector<Key>& p1, const std::vector<Key>& p2)
{
    m_controller1.setKeyBindings(p1);
    m_controller2.setKeyBindings(p2);
}

void Emulator::muteAudio()
{
    m_audioPlayer.mute();
}

}