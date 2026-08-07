#ifndef EMULATOR_H
#define EMULATOR_H
#include <chrono>
#include <memory>
#include <string>
#include <vector>

#include "APU/APU.h"
#include "AudioPlayer.h"
#include "CPU.h"
#include "Controller.h"
#include "MainBus.h"
#include "PPU.h"
#include "PictureBus.h"
#include "RastermPresenter.h"

namespace sn
{
using TimePoint          = std::chrono::steady_clock::time_point;
using Duration           = std::chrono::steady_clock::duration;

const int NESVideoWidth  = ScanlineVisibleDots;
const int NESVideoHeight = VisibleScanlines;

class Emulator
{
public:
    Emulator();
    void run(std::string rom_path);
    void setKeys(const std::vector<Key>& p1, const std::vector<Key>& p2);
    void muteAudio();

private:
    void                    OAMDMA(Byte page);
    Byte                    DMCDMA(Address addr);

    CPU                     m_cpu;

    AudioPlayer             m_audioPlayer;

    PictureBus              m_pictureBus;
    PPU                     m_ppu;
    APU                     m_apu;
    Cartridge               m_cartridge;
    std::unique_ptr<Mapper> m_mapper;

    Controller              m_controller1, m_controller2;

    MainBus                 m_bus;

    VirtualScreen           m_emulatorScreen;
    rastermPresenter        m_presenter;

    TimePoint               m_lastWakeup;

    Duration                m_elapsedTime;
};
}
#endif // EMULATOR_H