#pragma once
#include <google/protobuf/service.h>
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpConnection.h>
#include <string>
#include <functional>
#include <google/protobuf/descriptor.h>
#include <unordered_map>

// class responsible for rpc service launching
class RpcProvider
{
public:
    // api used to launch rpc methods
    void NotifyService(google::protobuf::Service *service);
    // activate rpc service nodes, providing remote rpc service
    void Run();

private:
    // eventloop
    muduo::net::EventLoop m_eventLoop;
    // methods and service info
    struct ServiceInfo
    {
        google::protobuf::Service *m_service;
        std::unordered_map<std::string, const google::protobuf::MethodDescriptor *> m_methodMap;
    };
    // save all service and its info that has been registered
    std::unordered_map<std::string, ServiceInfo> m_serviceMap;

    // callback function for new socket connection
    void OnConnection(const muduo::net::TcpConnectionPtr &);
    // callback function for read/write events from connected users
    void OnMessage(const muduo::net::TcpConnectionPtr &, muduo::net::Buffer *, muduo::Timestamp);
    // callback function for Closure, used to serialize rpc response and send it to client side
    void SendRpcResponse(const muduo::net::TcpConnectionPtr &, google::protobuf::Message *);
};
