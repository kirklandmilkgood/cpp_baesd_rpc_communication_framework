#pragma once
#include <queue>
#include <thread>
#include <mutex>              // pthread_mutex_t
#include <condition_variable> // p_thread_condition_t

// queue for async log-writting
template <typename T>
class LockQueue
{
public:
    // there are multiple worker threads can write log
    void push(const T &data)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(data);
        m_cond_var.notify_one();
    }

    // one thread acquire task from queue and write log at a time
    T pop()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (m_queue.empty())
        {
            // queue is empty, thread enter waiting state
            m_cond_var.wait(lock);
        }

        T data = m_queue.front();
        m_queue.pop();
        return data;
    }

private:
    std::queue<T> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_cond_var;
};