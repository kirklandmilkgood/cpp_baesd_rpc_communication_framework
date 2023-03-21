#pragma once

#include <unordered_map>
#include <string>

// config arguments: rpc_server_ip, rpc_server_port, zookeeper_ip, zookeeper_port
// read config file
class MprpcConfig
{
public:
    // load and parse config file
    void LoadConfigFile(const char *config_file);
    // load and query config entry
    std::string Load(const std::string &key);

private:
    std::unordered_map<std::string, std::string> m_config_map;
    // remove blanks in the string
    void Trim(std::string &src_buf);
};