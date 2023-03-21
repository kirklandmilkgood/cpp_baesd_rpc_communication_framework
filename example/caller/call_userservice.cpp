#include <iostream>
#include "../../include/mprpc_application.h"
#include "../../include/mprpc_channel.h"
#include "../user.pb.h"

int main(int argc, char **argv)
{
    // framework initialization
    MprpcApplication::Init(argc, argv);

    // remotely call rpc method -- Login
    example_service::UserServiceRpc_Stub stub(new MprpcChannel());
    // request arguments of rpc method
    example_service::LoginRequest request;
    request.set_name("Lathan");
    request.set_password("admin");
    // rpc response
    example_service::LoginResponse response;
    // rpc method calling
    stub.Login(nullptr, &request, &response, nullptr); // agented by RpcChannel::callMethod

    if (response.result().error_code() == 0)
    {
        std::cout << "rpc login response success: " << response.success() << std::endl;
    }
    else
    {
        std::cout << "rpc login response error: " << response.result().error_msg() << std::endl;
    }

    // rpc method remote calling -- Register
    example_service::RegisterRequest req;
    req.set_id(99);
    req.set_name("Brynlee");
    req.set_password("123456");
    example_service::RegisterResponse rsp;

    stub.Register(nullptr, &req, &rsp, nullptr);

    if (rsp.result().error_code() == 0)
    {
        std::cout << "rpc register response success: " << rsp.success() << std::endl;
    }
    else
    {
        std::cout << "rpc register response error: " << rsp.result().error_msg() << std::endl;
    }
    return 0;
}
