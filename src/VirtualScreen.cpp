#include "VirtualScreen.h"

namespace sn
{
void VirtualScreen::create(const unsigned int width, const unsigned int height, const Color color)
{
    m_width = width;
    m_height = height;
    m_frameNumber = 0;
    m_pixels.assign(static_cast<std::size_t>(width) * height, color);
}

const std::uint8_t* VirtualScreen::data() const noexcept
{
    return reinterpret_cast<const std::uint8_t*>(m_pixels.data());
}

std::ptrdiff_t VirtualScreen::stride() const noexcept
{
    return static_cast<std::ptrdiff_t>(m_width * sizeof(Color));
}
}