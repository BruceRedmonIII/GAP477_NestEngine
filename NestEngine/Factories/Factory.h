#pragma once
#include "../Configuration/LogMacro.hint"
#include "../Math/MathAlias.h"
#include "../SDLWrappers/Color.h"
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
namespace nest
{
    class XMLNode;
    class GameObject;
    enum class Result
    {
        kFailedToFind,
        kFailedToCreate,
        kSucess,
        kNone
    };

    class Factory
    {
    public:
        SET_HASHED_ID(Factory)
        virtual ~Factory() = default;
        virtual Result CreateComponent(HashedId componentId, nest::GameObject* owner, const nest::XMLNode& dataFile);
        static Color CreateColorFromString(const char* color)
        {
            Color newColor{ 0, 0, 0, 0 };
            size_t pos = 0;
            try
            {
                newColor.r = static_cast<uint8_t>(std::stoi(FindNextWord(pos, color, ' ')));
                newColor.g = static_cast<uint8_t>(std::stoi(FindNextWord(pos, color, ' ')));
                newColor.b = static_cast<uint8_t>(std::stoi(FindNextWord(pos, color, ' ')));
                newColor.a = static_cast<uint8_t>(std::stoi(FindNextWord(pos, color, ' ')));
            }
            catch (std::invalid_argument const& ex)
            {
                std::cout << "std::invalid_argument::what(): " << ex.what() << '\n';
            }
            catch (std::out_of_range const& ex)
            {
                std::cout << "std::out_of_range::what(): " << ex.what() << '\n';
            }
            return newColor;
        }
        static Vec4 CreateRectFromString(const char* rect)
        {
            Vec4 newRect{ 0, 0, 0, 0 };
            size_t pos = 0;
            try
            {
                newRect.x = std::stof(FindNextWord(pos, rect, ' '));
                newRect.y = std::stof(FindNextWord(pos, rect, ' '));
                newRect.z = std::stof(FindNextWord(pos, rect, ' '));
                newRect.w = std::stof(FindNextWord(pos, rect, ' '));
            }
            catch (std::invalid_argument const& ex)
            {
                std::cout << "std::invalid_argument::what(): " << ex.what() << '\n';
            }
            catch (std::out_of_range const& ex)
            {
                std::cout << "std::out_of_range::what(): " << ex.what() << '\n';
            }
            return newRect;
        }
        static Vec4 CreateFloatRectFromString(const char* rect)
        {
            Vec4 newRect{ 0, 0, 0, 0 };
            size_t pos = 0;
            try
            {
                newRect.x = std::stof(FindNextWord(pos, rect, ' '));
                newRect.y = std::stof(FindNextWord(pos, rect, ' '));
                newRect.z = std::stof(FindNextWord(pos, rect, ' '));
                newRect.w = std::stof(FindNextWord(pos, rect, ' '));
            }
            catch (std::invalid_argument const& ex)
            {
                std::cout << "std::invalid_argument::what(): " << ex.what() << '\n';
            }
            catch (std::out_of_range const& ex)
            {
                std::cout << "std::out_of_range::what(): " << ex.what() << '\n';
            }
            return newRect;
        }
    };
}