#ifndef CONTROLLER_H
#define CONTROLLER_H
#include <cstdint>
#include <string>
#include <vector>

namespace sn
{
using Byte = std::uint8_t;
using Key = int;

[[nodiscard]] bool keyPressed(Key key) noexcept;
[[nodiscard]] Key keyCodeFromName(const std::string& name) noexcept;

class Controller
{
public:
    Controller();
    enum Buttons
    {
        A,
        B,
        Select,
        Start,
        Up,
        Down,
        Left,
        Right,
        TotalButtons,
    };

    void strobe(Byte b);
    Byte read();
    void setKeyBindings(const std::vector<Key>& keys);

private:
    bool                           m_strobe;
    unsigned int                   m_keyStates;

    std::vector<Key>               m_keyBindings;
};
}

#endif // CONTROLLER_H
