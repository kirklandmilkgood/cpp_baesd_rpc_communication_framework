#include "mprpc_config.h"
#include <iostream>
#include <string>

// parse and load config file
void MprpcConfig::LoadConfigFile(const char *config_file)
{
    FILE *pf = fopen(config_file, "r");
    if (pf == nullptr)
    {
        std::cout << config_file << "is not exist!" << std::endl;
        exit(EXIT_FAILURE);
    }

    // parse config entries: 1. ignore comments 2. remove blanks 3. parse entries properly
    // read config file line by line until reach the end of file (eof)
    while (!feof(pf))
    {
        char buf[512] = {0};
        fgets(buf, 512, pf);

        // remove redundant blanks
        std::string read_buf(buf);
        Trim(read_buf);

        // ignore comments
        if (read_buf[0] == '#' || read_buf.empty())
        {
            continue;
        }

        // parse config entries
        int idx = read_buf.find('=');
        if (idx == -1)
        {
            // config entry is illegal
            continue;
        }

        std::string key;
        std::string value;
        key = read_buf.substr(0, idx);
        Trim(key); // remove blanks on both sides of entries
        // rpcserverip = 127.0.0.1\n, remove \n
        int end_idx = read_buf.find('\n', idx);
        value = read_buf.substr(idx + 1, end_idx - idx - 1);
        Trim(value); // remove blanks on both sides of entries
        m_config_map.insert({key, value});
    }
    fclose(pf);
}

// load info of config entries
std::string MprpcConfig::Load(const std::string &key)
{
    auto it = m_config_map.find(key);
    if (it == m_config_map.end())
    {
        return "";
    }
    return it->second;
}

// remove blanks
void MprpcConfig::Trim(std::string &src_buf)
{
    int idx = src_buf.find_first_not_of(' ');
    if (idx != -1)
    {
        // in this case, there are redundant blanks in first half of the string
        src_buf = src_buf.substr(idx, src_buf.size() - idx);
    }
    // remove redundant blanks in last blanks of the string
    idx = src_buf.find_last_not_of(' ');
    if (idx != -1)
    {
        // in this case, there are redundant blanks in the last half of the string
        src_buf = src_buf.substr(0, idx + 1);
    }
}