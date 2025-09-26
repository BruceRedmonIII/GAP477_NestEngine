#pragma once
#include "../Managers/LogManager.h"
#include <filesystem>
#include <string>

namespace std
{
    [[nodiscard]] inline string to_string(const char* val)
    {
        return std::string(val);
    }

    [[nodiscard]] inline string to_string(std::filesystem::path path)
    {
        return std::string(path.string());
    }

    [[nodiscard]] inline const string& to_string(const std::string& str)
    {
        return str;
    }

}

template <class... Args>
std::string GenerateString(Args&&... args)
{
    std::string result = (... + std::to_string(std::forward<Args>(args)));
    return result;
}

namespace nest
{
    template <class... Args>
    void VariadicLog(nest::LogType type, Args&&... args)
    {
        nest::LogManager::Log(GenerateString(args...), __FUNCTION__, __FILE__, __LINE__, type);
    };

    [[maybe_unused]] static std::string FindNextWord(size_t& index, const char* string, char delim)
    {
        constexpr char end{};
        std::string wordValue{};
        while (string[index] != delim && string[index] != end) // go to end of word
        {
            wordValue += string[index];
            ++index;
        }
        while (string[index] == delim && string[index] != end) // go to start of next word
        {
            ++index;
        }
        return wordValue;
    }
}