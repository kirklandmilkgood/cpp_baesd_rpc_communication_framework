#pragma once

#include <semaphore.h>
#include <zookeeper/zookeeper.h>
#include <string>

// class of encapsulated client side
class ZkClient
{
public:
    ZkClient();
    ~ZkClient();
    // activate zkclient and connect to zkserver
    void Start();
    // create service node on zkserver according to designated path
    void Create(const char *path, const char *data, int data_length, int state = 0);
    // get data from designated path or value of node
    std::string GetData(const char *path);

private:
    zhandle_t *m_zhandle; // a handle is needed to invoke any zk function
};