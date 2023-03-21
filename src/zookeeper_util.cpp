#include "zookeeper_util.h"
#include "mprpc_application.h"
#include <semaphore.h>
#include <iostream>

static char data_buf[64];
// global watcher observator, notification to zkclient from zkserver
void global_watcher(zhandle_t *zh, int type, int state, const char *path, void *watcherCtx)
{
    if (type == ZOO_SESSION_EVENT)
    {
        if (state == ZOO_CONNECTED_STATE) // zkclient connect to zkserver successfully
        {
            sem_t *sem = (sem_t *)zoo_get_context(zh);
            sem_post(sem);
        }
    }
}

void QueryServed_data_completion(int rc, const char *value, int value_len, const Stat *stat, const void *data)
{
    std::cout << "data: " << value << "data length: " << value_len << std::endl;
    strncpy(data_buf, value, sizeof(data_buf) - 1);
    data_buf[sizeof(data_buf) - 1] = '\0';
}

ZkClient::ZkClient() : m_zhandle(nullptr)
{
}

ZkClient::~ZkClient()
{
    if (m_zhandle != nullptr)
    {
        zookeeper_close(m_zhandle); // close m_zhandle, release resource
    }
}

// connect to zkserver
void ZkClient::Start()
{
    std::string host = MprpcApplication::GetInstance().GetConfig().Load("zookeeperip");
    std::string port = MprpcApplication::GetInstance().GetConfig().Load("zookeeperport");
    std::string connstr = host + ":" + port;
    m_zhandle = zookeeper_init(connstr.c_str(), global_watcher, 30000, nullptr, nullptr, 0);
    if (m_zhandle == nullptr)
    {
        std::cout << "zookeeper_init error!" << std::endl;
        exit(EXIT_FAILURE);
    }

    sem_t sem;
    sem_init(&sem, 0, 0);
    zoo_set_context(m_zhandle, &sem);
    sem_wait(&sem); // block here, until the semaphore is not 0
    std::cout << "zookeeper_init success!" << std::endl;
}

void ZkClient::Create(const char *path, const char *data, int data_length, int state)
{
    char path_buffer[128];
    int bufferlen = sizeof(path_buffer);
    int flag;
    flag = zoo_aexists(m_zhandle, path, 0, nullptr, nullptr);
    if (ZNONODE == flag)
    {
        if (flag == ZOK)
        {
            std::cout << "znode create success... path: " << path << std::endl;
        }
        else
        {
            std::cout << "flag: " << flag << std::endl;
            std::cout << "znode create error: " << path << std::endl;
            exit(EXIT_FAILURE);
        }
    }
}

// acquire data of znode according to designated path
std::string ZkClient::GetData(const char *path)
{
    int flag;
    bzero(data_buf, sizeof(data_buf));
    flag = zoo_aget(m_zhandle, path, 0, QueryServed_data_completion, nullptr);
    if (flag != ZOK)
    {
        std::cout << "get znode error... path: " << path << std::endl;
        return "";
    }
    else
    {
        return data_buf;
    }
}
