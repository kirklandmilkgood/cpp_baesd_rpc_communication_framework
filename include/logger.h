#pragma once
#include "lockqueue.h"
#include <string>

#define LOG_INFO(log_msg_format, ...)                     \
    do                                                    \
    {                                                     \
        Logger &logger = Logger::GetInstance();           \
        logger.SetLogLevel(INFO);                         \
        char c[1024] = {0};                               \
        snprintf(c, 1024, log_msg_format, ##__VA_ARGS__); \
        logger.Log(c);                                    \
    } while (0)

#define LOG_ERR(log_msg_format, ...)                      \
    do                                                    \
    {                                                     \
        Logger &logger = Logger::GetInstance();           \
        logger.SetLogLevel(ERROR);                        \
        char c[1024] = {0};                               \
        snprintf(c, 1024, log_msg_format, ##__VA_ARGS__); \
        logger.Log(c);                                    \
    } while (0)

// define log level
enum LogLevel
{
    INFO,  // normal info
    ERROR, // error info
};

// log system provided by mprpc framework
class Logger
{
public:
    // singleton pattern of log
    static Logger &GetInstance();
    // set log level
    void SetLogLevel(LogLevel level);
    // write log
    void Log(std::string msg);

private:
    int m_log_level;                     // log level
    LockQueue<std::string> m_lock_queue; // log-writting task queue

    Logger();
    Logger(const Logger &) = delete; // singleton
    Logger(Logger &&) = delete;      // singleton
};