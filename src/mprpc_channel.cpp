#include "mprpc_channel.h"
#include <string>
#include "rpcheader.pb.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include "mprpc_application.h"
#include "zookeeper_util.h"

// agent rpc-methods-calling, data serialization, data-sending
void MprpcChannel::CallMethod(const google::protobuf::MethodDescriptor *method,
                              google::protobuf::RpcController *controller,
                              const google::protobuf::Message *request,
                              google::protobuf::Message *response,
                              google::protobuf::Closure *done)
{
    const google::protobuf::ServiceDescriptor *sd = method->service();

    std::string service_name = sd->name();    // service name
    std::string method_name = method->name(); // method name

    // acquire length of serialized arguments: args_size
    uint32_t args_size = 0;
    std::string args_str;
    if (request->SerializeToString(&args_str))
    {
        args_size = args_str.size();
    }
    else
    {
        controller->SetFailed("serialize request error!");
        return;
    }

    // define rpc request header
    mprpc::RpcHeader rpcHeader;
    rpcHeader.set_service_name(service_name);
    rpcHeader.set_method_name(method_name);
    rpcHeader.set_args_size(args_size);
    uint32_t header_size = 0;
    std::string rpc_header_str;
    if (rpcHeader.SerializeToString(&rpc_header_str))
    {
        header_size = rpc_header_str.size();
    }
    else
    {
        controller->SetFailed("serialize rpc gheader error!");
        return;
    }

    // organize rpc request strings that are about to be sent
    std::string send_rpc_str;
    send_rpc_str.insert(0, std::string((char *)&header_size, 4)); // header size
    send_rpc_str += rpc_header_str;                               // rpc header
    send_rpc_str += args_str;                                     // args

    // print rpc header info
    std::cout << "==========================" << std::endl;
    std::cout << "header_size: " << header_size << std::endl;
    std::cout << "rpc_header_str: " << rpc_header_str << std::endl;
    std::cout << "service_name: " << service_name << std::endl;
    std::cout << "method name: " << method_name << std::endl;
    std::cout << "args_str: " << args_str << std::endl;
    std::cout << "==========================" << std::endl;

    // tcp socket programming
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientfd == -1)
    {
        char error_txt[512] = {0};
        sprintf(error_txt, "create socket error:%d", errno);
        controller->SetFailed(error_txt);
        return;
    }

    // rpc caller want to call rpc method, then query the host info of the service on zk nodes first
    ZkClient zkCli;
    zkCli.Start();
    //  ex: /UserServiceRpc/Login
    std::string method_path = "/" + service_name + "/" + method_name;
    //  ex: 127.0.0.1:8000
    std::string host_data = zkCli.GetData(method_path.c_str());
    if (host_data == "")
    {
        controller->SetFailed(method_path + "is not exist!");
        return;
    }
    int idx = host_data.find(":");
    if (idx == -1)
    {
        controller->SetFailed(method_path + " address is invaild!");
        return;
    }
    std::string ip = host_data.substr(0, idx);
    uint16_t port = atoi(host_data.substr(idx + 1, host_data.size() - idx).c_str());

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());

    // connect to zk service nodes
    if (connect(clientfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        close(clientfd);
        char error_txt[512] = {0};
        sprintf(error_txt, "connect error! errno:%d", errno);
        controller->SetFailed(error_txt);
        return;
    }

    // send rpc request
    if (send(clientfd, send_rpc_str.c_str(), send_rpc_str.size(), 0) == -1)
    {
        close(clientfd);
        char error_txt[512] = {0};
        sprintf(error_txt, "send error! errno:%d", errno);
        controller->SetFailed(error_txt);
        return;
    }

    // receive rpc response
    char recv_buf[1024] = {0};
    int recv_size = 0;
    if ((recv_size = recv(clientfd, recv_buf, 1024, 0)) == -1)
    {
        close(clientfd);
        char error_txt[512] = {0};
        sprintf(error_txt, "recv error! errno:%d", errno);
        return;
    }

    // de-serialize rpc response
    if (!response->ParseFromArray(recv_buf, recv_size))
    {
        close(clientfd);
        char error_txt[512] = {0};
        sprintf(error_txt, "parse error! response_str:%s", recv_buf);
        controller->SetFailed(error_txt);
        return;
    }

    close(clientfd);
}
