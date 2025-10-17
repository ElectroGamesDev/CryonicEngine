#pragma once

#include <string>
#include <vector>
#include <utility>

class ConsoleLogger
{
public:
    enum class LogType {
        INFO,
        WARNING,
        ERROR_
    };

    struct LogEntry {
        std::string message;
        LogType type;
    };

    static void InfoLog(const std::string& message, bool devMessage = true);
    static void WarningLog(const std::string& message, bool devMessage = true);
    static void ErrorLog(const std::string& message, bool devMessage = true);

    // GetLogs() has been replaced by GetLog() and GetLogCount() to fix issues with the max log cap. Using this function will return logs in the incorrect order if the log count passes the max (1000 by default).
    static const std::vector<LogEntry>& GetLogs();
    static int GetLogCount();
    static const LogEntry& GetLog(int index);

    static bool showDebugMessages;

private:
    static int logCount;
    static int logStart;
    static int maxLogs;
    static std::vector<LogEntry> logs;

    static void PushLog(const std::string& message, LogType type);
};
