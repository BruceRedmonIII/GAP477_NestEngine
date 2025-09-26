#include "LogManager.h"
#include <cassert>
#include <iostream>
#include <fstream>
#include <chrono>
#include <filesystem>
#include <windows.h>
// default values in-case config file doesn't exist
std::filesystem::path nest::LogManager::m_kLogPath{};
std::string nest::LogManager::m_kLogFileName = "LogFile.txt";
std::string nest::LogManager::m_kLogFolderName = "Logs";
bool nest::LogManager::m_kLogByMonth = true;
std::ofstream nest::LogManager::m_logFile{};

void nest::LogManager::InitLogManager()
{
    SetConfigsFromFile();
    CreateLogDirectory();
    OpenLog();
}

//-----------------------------------------------------------------------------------------------
// Logs a message to the console and the logfile
//-----------------------------------------------------------------------------------------------
void nest::LogManager::Log(const std::string& message, const char* functionName, const char* fileName, unsigned int lineNumber, LogType type)
{
    if (type == LogType::kPrint)
    {
#if _LOG_TO_TERMINAL == 1
        DisplayToConsole(message, functionName, fileName, lineNumber, type);
#endif
        return;
    }

    const auto now = std::chrono::system_clock::now();
    m_logFile << now;
    if (type == LogType::kError)
    {
        m_logFile << " ERROR: ";
    }
    m_logFile << " Function name | " << functionName << " | FileName -| " << fileName << "| LineNumber -|" << lineNumber << "|: " << message << std::endl;

#if _LOG_TO_TERMINAL == 1
    DisplayToConsole(message, functionName, fileName, lineNumber, type);
#endif
}

//-----------------------------------------------------------------------------------------------
// Opens the log file
//-----------------------------------------------------------------------------------------------
void nest::LogManager::OpenLog()
{
    if (!m_logFile.is_open()) // check to see if the log file has already been opened
    {
        std::filesystem::current_path(m_kLogPath);
        m_logFile.open(m_kLogFileName, std::fstream::app);
    }
    else
        return; // already open

    if (!m_logFile) // failed to open an existing file
    {
        std::cerr << "Log file not found! creating new LogFile\n";
        m_logFile.open(m_kLogFileName);
    }

    if (!m_logFile) // failed to create a new file
    {
        std::cerr << "Failed to create log file!\n";
    }
    ResetPath();
}

//-----------------------------------------------------------------------------------------------
// Closes the log file
//-----------------------------------------------------------------------------------------------
void nest::LogManager::CloseLog()
{
    m_logFile.close(); // always close 
}

//-----------------------------------------------------------------------------------------------
// This will create a log directory based on the current system clock,
// if you have enabled save by month,
// it will create the current months folder and will add any NEW log data into that month
//-----------------------------------------------------------------------------------------------
void nest::LogManager::CreateLogDirectory()
{
    std::error_code errorCode{};

    if (m_kLogPath.empty())
    {
        const std::filesystem::directory_entry dEntry{ std::filesystem::current_path().parent_path() };
        std::filesystem::current_path(dEntry);
        const auto now = std::chrono::system_clock::now();
        // I could static cast the entire year month and day at once,
        // but I'd lose the ability to add underscores and also switch the format around.
        const auto ymd = std::chrono::year_month_day{ std::chrono::floor<std::chrono::days>(now) };
        const auto month = static_cast<unsigned>(ymd.month());
        const auto day = static_cast<unsigned>(ymd.day());
        const auto year = static_cast<int>(ymd.year());
        std::string ymdFolderString{};
        if (m_kLogByMonth) // if log by month is selected, change the file path name to be days
        {
            ymdFolderString = std::to_string(month) + "_" + std::to_string(year);
        }
        else // else leave the filepath as LogFile and make the folder show the day
        {
            ymdFolderString = std::to_string(month) + "_" + std::to_string(day) + "_" + std::to_string(year);
        }

        m_kLogFileName = "Day" + std::to_string(day) + "_" + std::to_string(year) + m_kLogFileName; // adds day to the log file name
        // if the current path exists, don't create a new directory
        if (!std::filesystem::exists(std::filesystem::current_path().concat("\\bin\\" + m_kLogFolderName + "\\" + ymdFolderString)))
        {
            std::filesystem::create_directories("bin\\" + m_kLogFolderName + "\\" + ymdFolderString, errorCode);
            if (errorCode)
            {
                std::cerr << errorCode << std::endl;
            }
            std::filesystem::permissions("bin\\" + m_kLogFolderName + "\\" + ymdFolderString, std::filesystem::perms::all);
        }
        // piece together the path and set it as the global path variable
        std::filesystem::path filePathFinal = dEntry.path();
        filePathFinal.concat("\\bin\\" + m_kLogFolderName + "\\" + ymdFolderString + "\\");
        m_kLogPath = filePathFinal;
    }
}

//-----------------------------------------------------------------------------------------------
// Sets all the configs from the config file, or creates a config file with the default settings
//-----------------------------------------------------------------------------------------------
void nest::LogManager::SetConfigsFromFile()
{
    std::ifstream configFile{};
    std::string line{};
    std::string configSetting{};

    configFile.open("LogConfig.txt");
    if (configFile.is_open())
    {
        while (std::getline(configFile, line))
        {
            std::stringstream streamLine(line);
            std::getline(streamLine, line, '=');
            std::getline(streamLine, configSetting);
            if (line == "SaveByMonth")
            {
                if (configSetting == "true")
                {
                    m_kLogByMonth = true;
                }
                else
                    m_kLogByMonth = false;
            }
            if (line == "LogFolderName")
            {
                m_kLogFolderName = configSetting;
            }
            if (line == "LogFileSuffix")
            {
                m_kLogFileName = configSetting;
            }

            // assert that it didn't copy an empty string
            assert(!m_kLogFileName.empty());
            assert(!m_kLogFolderName.empty());
        }
    }
    else // create a new config file
    {
        std::ofstream newConfigFile;
        newConfigFile.open("LogConfig.txt");
        newConfigFile << "SaveByMonth=true" << std::endl;
        newConfigFile << "LogFolderName=Logs" << std::endl;
        newConfigFile << "LogFileSuffix=LogFile.txt" << std::endl;
        newConfigFile.close();
    }
    configFile.close();
}

void nest::LogManager::DisplayToConsole(const std::string& message, const char* functionName, const char* fileName, unsigned int lineNumber, LogType type)
{
    if (type == LogType::kPrint) // returns instantly since its just a print function
    {
        std::cout << message << std::endl;
        return;
    }

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (type == LogType::kLog)
    {
        SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    }
    else if (type == LogType::kError)
    {
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY); 
        std::cerr << "ERROR: ";
    }
    std::cerr << " Function name | " << functionName << " | FileName -| " << fileName << "| LineNumber -|" << lineNumber << "|: " << message << std::endl;
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}

void nest::LogManager::ResetPath()
{
    std::filesystem::current_path(m_kOriginalPath);
}
