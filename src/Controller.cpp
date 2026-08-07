#include "Controller.h"

#include <windows.h>

#include <cctype>
#include <unordered_map>

namespace sn
{
Controller::Controller()
  : m_strobe(false)
  , m_keyStates(0)
  , m_keyBindings(TotalButtons)
{
}

bool keyPressed(const Key key) noexcept
{
    return key > 0 && (GetAsyncKeyState(key) & 0x8000) != 0;
}

Key keyCodeFromName(const std::string& name) noexcept
{
    if (name.size() == 1 && std::isalnum(static_cast<unsigned char>(name[0]))) {
        return std::toupper(static_cast<unsigned char>(name[0]));
    }
    if (name.size() == 4 && name.starts_with("Num") && std::isdigit(static_cast<unsigned char>(name[3]))) {
        return name[3];
    }
    if (name.size() == 7 && name.starts_with("Numpad") && std::isdigit(static_cast<unsigned char>(name[6]))) {
        return VK_NUMPAD0 + name[6] - '0';
    }
    if (name.size() >= 2 && name[0] == 'F') {
        try {
            const int number = std::stoi(name.substr(1));
            if (number >= 1 && number <= 24) {
                return VK_F1 + number - 1;
            }
        }
        catch (...) {
        }
    }

    static const std::unordered_map<std::string, Key> namedKeys {
        { "Escape", VK_ESCAPE }, { "LControl", VK_LCONTROL }, { "LShift", VK_LSHIFT },
        { "LAlt", VK_LMENU }, { "LSystem", VK_LWIN }, { "RControl", VK_RCONTROL },
        { "RShift", VK_RSHIFT }, { "RAlt", VK_RMENU }, { "RSystem", VK_RWIN },
        { "Menu", VK_APPS }, { "LBracket", VK_OEM_4 }, { "RBracket", VK_OEM_6 },
        { "SemiColon", VK_OEM_1 }, { "Comma", VK_OEM_COMMA }, { "Period", VK_OEM_PERIOD },
        { "Quote", VK_OEM_7 }, { "Slash", VK_OEM_2 }, { "BackSlash", VK_OEM_5 },
        { "Tilde", VK_OEM_3 }, { "Equal", VK_OEM_PLUS }, { "Dash", VK_OEM_MINUS },
        { "Space", VK_SPACE }, { "Return", VK_RETURN }, { "BackSpace", VK_BACK },
        { "Tab", VK_TAB }, { "PageUp", VK_PRIOR }, { "PageDown", VK_NEXT },
        { "End", VK_END }, { "Home", VK_HOME }, { "Insert", VK_INSERT },
        { "Delete", VK_DELETE }, { "Add", VK_ADD }, { "Subtract", VK_SUBTRACT },
        { "Multiply", VK_MULTIPLY }, { "Divide", VK_DIVIDE }, { "Left", VK_LEFT },
        { "Right", VK_RIGHT }, { "Up", VK_UP }, { "Down", VK_DOWN }, { "Pause", VK_PAUSE },
    };
    const auto key = namedKeys.find(name);
    return key == namedKeys.end() ? 0 : key->second;
}

void Controller::setKeyBindings(const std::vector<Key>& keys)
{
    m_keyBindings = keys;
}

void Controller::strobe(Byte b)
{
    m_strobe = (b & 1);
    if (!m_strobe)
    {
        m_keyStates = 0;
        int shift   = 0;
        for (int button = A; button < TotalButtons; ++button)
        {
            m_keyStates |= (keyPressed(m_keyBindings[static_cast<Buttons>(button)]) << shift);
            ++shift;
        }
    }
}

Byte Controller::read()
{
    Byte ret;
    if (m_strobe)
        ret = keyPressed(m_keyBindings[A]);
    else
    {
        ret           = (m_keyStates & 1);
        m_keyStates >>= 1;
    }
    return ret | 0x40;
}

}