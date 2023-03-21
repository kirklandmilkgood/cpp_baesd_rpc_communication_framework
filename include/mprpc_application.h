#pragma once
#include "mprpc_config.h"
#include "mprpc_controller.h"
#include "mprpc_controller.h"

// class responsible for initialization manipulation
class MprpcApplication
{
public:
    static void Init(int argc, char **argv);
    static MprpcApplication &GetInstance(); // singleton pattern
    static MprpcConfig &GetConfig();

private:
    static MprpcConfig m_config;

    MprpcApplication(){};
    MprpcApplication(const MprpcApplication &) = delete;
    MprpcApplication(MprpcApplication &&) = delete;
};