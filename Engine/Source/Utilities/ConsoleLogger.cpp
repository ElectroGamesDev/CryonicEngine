#include "Utilities/ConsoleLogger.h"
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

int ConsoleLogger::maxLogs = 1000; // Todo: Make this configurable in settings
bool ConsoleLogger::showDebugMessages = true;
std::vector<ConsoleLogger::LogEntry> ConsoleLogger::logs(maxLogs);
int ConsoleLogger::logCount = 0;
int ConsoleLogger::logStart = 0;

void ConsoleLogger::InfoLog(const std::string& message, bool devMessage)
{
    if (devMessage && !showDebugMessages)
        return;

    std::cout << "[INFO] " << message << std::endl;
    PushLog(message, LogType::INFO);
}

void ConsoleLogger::WarningLog(const std::string& message, bool devMessage)
{
    if (devMessage && !showDebugMessages)
        return;

#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, 14); // Set color to yellow
#endif
    std::cout << "[WARNING] " << message << std::endl;
#ifdef _WIN32
    SetConsoleTextAttribute(hConsole, 15); // Reset color
#endif

    PushLog(message, LogType::WARNING);
}

void ConsoleLogger::ErrorLog(const std::string& message, bool devMessage)
{
    if (devMessage && !showDebugMessages)
        return;

#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, 12); // Set color to red
#endif
    std::cerr << "[ERROR] " << message << std::endl;
#ifdef _WIN32
    SetConsoleTextAttribute(hConsole, 15); // Reset color
#endif

    PushLog(message, LogType::ERROR_);
}


void ConsoleLogger::PushLog(const std::string& message, LogType type)
{
    int index = (logStart + logCount) % maxLogs;

    logs[index] = { message, type };

    if (logCount < maxLogs)
        ++logCount;
    else
        logStart = (logStart + 1) % maxLogs;
}

const std::vector<ConsoleLogger::LogEntry>& ConsoleLogger::GetLogs()
{
    return logs;
}

int ConsoleLogger::GetLogCount()
{
    return logCount;
}

const ConsoleLogger::LogEntry& ConsoleLogger::GetLog(int index)
{
    return logs[(logStart + index) % maxLogs];
}