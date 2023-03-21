#include <iostream>
#include <string>
#include "../user.pb.h"
#include "../../include/mprpc_application.h"
#include "../../include/rpcprovider.h"

// Userservice is a local service with 2 local methods:Login, Register
class UserService : public example_service::UserServiceRpc
{
public:
    bool Login(std::string name, std::string password)
    {
        std::cout << "doing local service: Login..." << std::endl;
        std::cout << "name: " << name << "password: " << password << std::endl;
        return false;
    }

    bool Register(uint32_t id, std::string name, std::string password)
    {
        std::cout << "doing local service: Register..." << std::endl;
        std::cout << "id: " << id << "name: " << name << "password: " << password << std::endl;
        return true;
    }
    // override virtual functions in UserServiceRpc that will be directly called by the rpc framework
    // caller -> Login(LoginRequest) -> muduo -> callee
    // callee -> Login(LoginRequest) -> override functions

    void Login(::google::protobuf::RpcController *controller,
               const ::example_service::LoginRequest *request,
               ::example_service::LoginResponse *response,
               ::google::protobuf::Closure *done)
    {
        std::string name = request->name();
        std::string password = request->password();

        // local business
        bool login_result = Login(name, password);
        // write response, including error code, error msg, return value...
        example_service::ResultCode *code = response->mutable_result();
        code->set_error_code(0);
        code->set_error_msg("");
        response->set_success(login_result);

        // callback manipulation: data serialization, data sending...
        done->Run();
    }

    void Register(::google::protobuf::RpcController *controller,
                  const ::example_service::RegisterRequest *request,
                  ::example_service::RegisterResponse *response,
                  ::google::protobuf::Closure *done)
    {
        uint32_t id = request->id();
        std::string name = request->name();
        std::string password = request->password();

        bool ret = Register(id, name, password);
        // write response
        response->mutable_result()->set_error_code(0);
        response->mutable_result()->set_error_msg("");
        response->set_success(ret);

        // callback
        done->Run();
    }
};

int main(int argc, char **argv)
{
    // rpc framework initialization
    MprpcApplication::Init(argc, argv);

    // provider launch UserService on the rpc node
    RpcProvider provider;
    provider.NotifyService(new UserService());

    // activate a rpc node, after calling Run(), this progress gets into blocking state, waiting for rpc remote calling
    provider.Run();
    return 0;
}