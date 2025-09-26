#include "Color.h"
#include <SDL.h>
nest::Color::Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
    : r(r)
    , g(g)
    , b(b)
    , a(a)
{}

SDL_Color nest::Color::GetSDLColor() const
{
    return SDL_Color(r, g, b, a);
}
