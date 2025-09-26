#pragma once
#include <cstdint>
struct SDL_Color;
namespace nest
{
    struct Color
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
        Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
        Color() = default;
        SDL_Color GetSDLColor() const;

        // mainly used for widgets to determine if we need to color mod
        bool IsColorSet() { return (r + g + b + a > 0); } // if this is false, then the color has not been set
    };
}