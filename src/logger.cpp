#include "logger.h"
#include <time.h>
#include <iostream>

Logger &Logger::GetInstance()
{
    static Logger logger;
    return logger;
}

Logger::Logger()
{
    // activate log-writing dedicated thread
    std::thread writeLogTask([&]()
                             {
        for(;;)
        {
            // acquire current time data, and write info into corresponding log file
            time_t now = time(nullptr);
            tm *now_time = localtime(&now);
            char file_name[128];
            sprintf(file_name, "%d-%d-%d-log.txt", now_time->tm_year+1900, now_time->tm_mon+1, now_time->tm_mday);

            FILE *pf = fopen(file_name, "a+");
            if(pf == nullptr)
            {
                std::cout << "logger file: " << file_name << " open error!" << std::endl;
                exit(EXIT_FAILURE);
            }

            std::string msg = m_lock_queue.pop();

            char time_buf[128] = {0};
            sprintf(time_buf, "%d:%d:%d =>[%s]", now_time->tm_hour, now_time->tm_min, now_time->tm_sec, (m_log_level == INFO ? "info" : "error"));
            msg.insert(0, time_buf);
            msg.append("\n");

            fputs(msg.c_str(), pf);
            fclose(pf);

        } });
    // detach thread
    writeLogTask.detach();
}

// set log level
void Logger::SetLogLevel(LogLevel level)
{
    m_log_level = level;
}

// push log info into lockqueue
void Logger::Log(std::string msg)
{
    m_lock_queue.push(msg);
}
