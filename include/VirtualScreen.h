#ifndef VIRTUALSCREEN_H
#define VIRTUALSCREEN_H

#include <cstddef>
#include <cstdint>
#include <vector>

namespace sn
{
struct Color
{
    std::uint8_t red = 0;
    std::uint8_t green = 0;
    std::uint8_t blue = 0;
    std::uint8_t alpha = 255;

    constexpr Color() = default;
    constexpr explicit Color(const std::uint32_t rgba) :
        red(static_cast<std::uint8_t>(rgba >> 24)),
        green(static_cast<std::uint8_t>(rgba >> 16)),
        blue(static_cast<std::uint8_t>(rgba >> 8)),
        alpha(static_cast<std::uint8_t>(rgba))
    {
    }
};
static_assert(sizeof(Color) == 4, "rasterm requires tightly packed RGBA pixels");

class VirtualScreen
{
public:
    void create(unsigned int width, unsigned int height, Color color);
    void setPixel(std::size_t x, std::size_t y, Color color) noexcept
    {
        if (x < m_width && y < m_height)
            m_pixels[y * m_width + x] = color;
    }
    void finishFrame() noexcept { ++m_frameNumber; }

    [[nodiscard]] const std::uint8_t* data() const noexcept;
    [[nodiscard]] int width() const noexcept { return static_cast<int>(m_width); }
    [[nodiscard]] int height() const noexcept { return static_cast<int>(m_height); }
    [[nodiscard]] std::ptrdiff_t stride() const noexcept;
    [[nodiscard]] std::uint64_t frameNumber() const noexcept { return m_frameNumber; }

private:
    unsigned int m_width = 0;
    unsigned int m_height = 0;
    std::uint64_t m_frameNumber = 0;
    std::vector<Color> m_pixels;
};
};
#endif // VIRTUALSCREEN_H