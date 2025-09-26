#pragma once
#include "Manager.h"
#include <filesystem>
#include <string>

namespace nest
{
    const enum LogType : uint8_t
    {
        kLog, // will log message in green
        kError, // will log error and print to console in red
        kPrint // will print to console in white, will not log
    };

    // Log manager will create a directory in ExeAndTemps (will create that folder if it doesnt exist)
    // you dont ever need to actually construct this class but you do need to manually initialize and close the log file
    // simply call InitLogManager and then CloseLog when done to avoid losing data
    class LogManager : public Manager
    {
    public:
        SET_HASHED_ID(LogManager)
        // default values set in cpp
        static std::string m_kLogFolderName; // Log folder name
        static std::string m_kLogFileName; // The current day will automatically be added to the front of this text
        static bool m_kLogByMonth; // set to false to save by days instead of months
        static std::filesystem::path m_kLogPath; // The path to save the logs at, remains empty until set
        static std::ofstream m_logFile;
        static inline const std::filesystem::path m_kOriginalPath = std::filesystem::current_path();
    public:
        LogManager() = default;
        LogManager(const LogManager& copy) = delete;
        LogManager& operator=(const LogManager& copy) = delete;
        // this isn't supposed to be called directly, it is wrapped with a macro, use _LOG(msg) and _LOG(LogType::kError, msg)
        static void InitLogManager();
        static void Log(const std::string& message, const char* functionName, const char* fileName, unsigned int lineNumber, LogType type);
        static void OpenLog();
        static void CloseLog();
    private:
        static void CreateLogDirectory(); // Create a directory based on the current system clock
        static void SetConfigsFromFile(); // set the configurations from the config file, or create a new config file if not found
        static void DisplayToConsole(const std::string& message, const char* functionName, const char* fileName, unsigned int lineNumber, LogType error);
        static void ResetPath();
    };
}