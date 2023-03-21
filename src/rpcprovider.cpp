#include "rpcprovider.h"
#include "mprpc_application.h"
#include "rpcheader.pb.h"
#include "logger.h"
#include "zookeeper_util.h"

// service_name => service description
// => service* service object
// method_name => method object

// api for launching rpc service
void RpcProvider::NotifyService(google::protobuf::Service *service)
{
    ServiceInfo service_info;

    // acquire description info of service object
    const google::protobuf::ServiceDescriptor *pserviceDescriptor = service->GetDescriptor();
    // acquire service name
    std::string service_name = pserviceDescriptor->name();
    // acquire num of methods provided by service object
    int method_num = pserviceDescriptor->method_count();

    LOG_INFO("service_name:%s", service_name.c_str());
    for (int i = 0; i < method_num; i++)
    {
        // acquire descriptor of method designated by index
        const google::protobuf::MethodDescriptor *pmethodDescriptor = pserviceDescriptor->method(i);
        std::string method_name = pmethodDescriptor->name();
        service_info.m_methodMap.insert({method_name, pmethodDescriptor});

        LOG_INFO("method_name:%s", method_name.c_str());
    }
    service_info.m_service = service;
    m_serviceMap.insert({service_name, service_info});
}

// activate rpc service nodes, starting providing rpc remote service
void RpcProvider::Run()
{
    // load and read config file to get rpcserver info
    std::string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    uint16_t port = atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());
    muduo::net::InetAddress address(ip, port);

    // create TcpServer object
    muduo::net::TcpServer server(&m_eventLoop, address, "RpcProvider");

    // bind connection callback func and read/write events callback func, seperate business module from net module
    server.setConnectionCallback(std::bind(&RpcProvider::OnConnection, this, std::placeholders::_1));
    server.setMessageCallback(std::bind(&RpcProvider::OnMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    // set thread number for muduo
    server.setThreadNum(5);

    // register all service on current rpc node to zk server, then rpc client can find service from zk server
    // session timeout: 30s
    ZkClient zkCli;
    zkCli.Start();
    // service_name is a permanent node whereas service_name is a temporal node
    for (auto &sp : m_serviceMap)
    {
        //  /service_name  /UserServiceRpc
        std::string service_path = "/" + sp.first;
        zkCli.Create(service_path.c_str(), nullptr, 0);
        for (auto &mp : sp.second.m_methodMap)
        {
            //  /service_name/method_name  /UserServiceRpc/Login
            // => store the info regarding the host ip and port number which current rpc service node works on
            std::string method_path = service_path + "/" + mp.first;
            char method_path_data[128] = {0};
            sprintf(method_path_data, "%s:%d", ip.c_str(), port);
            // ZOO_EPHEMERAL indicates that this zknode is a temporal node
            zkCli.Create(method_path.c_str(), method_path_data, strlen(method_path_data), ZOO_EPHEMERAL);
        }
    }

    // rpc server is about to work, print info
    std::cout << "RpcProvider start service at ip: " << ip << " port: " << std::endl;

    // activate net service
    server.start();
    m_eventLoop.loop();
}

// callback func for new socket connection
void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr &conn)
{
    if (!conn->connected())
    {
        // connection with rpc client shutdown
        conn->shutdown();
    }
}

// callback func for read/write events
void RpcProvider::OnMessage(const muduo::net::TcpConnectionPtr &conn, muduo::net::Buffer *buffer, muduo::Timestamp)
{
    // data stream received from a rpc calling
    std::string recv_buf = buffer->retrieveAllAsString();

    // get the first 4 chars from data stream
    uint32_t header_size = 0;
    recv_buf.copy((char *)&header_size, 4, 0);

    // de-serialize data stream to acquire detailed info accordng to header_size
    std::string rpc_header_str = recv_buf.substr(4, header_size);
    mprpc::RpcHeader rpcHeader;
    std::string service_name;
    std::string method_name;
    uint32_t args_size;
    if (rpcHeader.ParseFromString(rpc_header_str))
    {
        // de-serialization success
        service_name = rpcHeader.service_name();
        method_name = rpcHeader.method_name();
        args_size = rpcHeader.args_size();
    }
    else
    {
        // de-serialization fail
        std::cout << "rpc_header_str: " << rpc_header_str << "parse error !" << std::endl;
        return;
    }

    // acquire data stream of rpc method arguments
    std::string args_str = recv_buf.substr(4 + header_size, args_size);

    // print info
    std::cout << "=========================================" << std::endl;
    std::cout << "header_size: " << header_size << std::endl;
    std::cout << "rpc_header_str: " << rpc_header_str << std::endl;
    std::cout << "rservice_name: " << service_name << std::endl;
    std::cout << "method_name: " << method_name << std::endl;
    std::cout << "args_str: " << args_str << std::endl;
    std::cout << "=========================================" << std::endl;

    // acquire service object and method object
    auto it = m_serviceMap.find(service_name);
    if (it == m_serviceMap.end())
    {
        std::cout << service_name << " is not exist!" << std::endl;
        return;
    }
    auto mit = it->second.m_methodMap.find(method_name);
    if (mit == it->second.m_methodMap.end())
    {
        std::cout << service_name << ":" << method_name << " is not exist!" << std::endl;
        return;
    }

    google::protobuf::Service *service = it->second.m_service;      // acquire service object
    const google::protobuf::MethodDescriptor *method = mit->second; // acquire method object

    // produce request and response arguments
    google::protobuf::Message *request = service->GetRequestPrototype(method).New();
    if (!request->ParseFromString(args_str))
    {
        std::cout << "request parse error, content: " << args_str << std::endl;
        return;
    }
    google::protobuf::Message *response = service->GetResponsePrototype(method).New();

    // bind a Closure callback func to the below method
    google::protobuf::Closure *done = google::protobuf::NewCallback<RpcProvider, const muduo::net::TcpConnectionPtr &,
                                                                    google::protobuf::Message *>(this, &RpcProvider::SendRpcResponse, conn, response);
    // call method launched by current rpc node according to the rpc request
    service->CallMethod(method, nullptr, request, response, done);
}

// Closure callback
void RpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr &conn, google::protobuf::Message *response)
{
    std::string response_str;
    if (response->SerializeToString(&response_str)) // serialization the response
    {
        conn->send(response_str);
    }
    else
    {
        std::cout << "serialize response error!" << std::endl;
    }
    conn->shutdown(); // simulate http non-persisent connection, connection shutdown by rpcprovider
}
