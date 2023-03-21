#include <iostream>
#include <string>
#include "../friend.pb.h"
#include "../../include/mprpc_application.h"
#include "../../include/rpcprovider.h"
#include <vector>
#include "../../include/logger.h"

class FriendService : public example_service::FriendServiceRpc
{
public:
    std::vector<std::string> GetfriendsList(uint32_t user_id)
    {
        std::cout << "doing GetFriendList service..., user_id: " << user_id << std::endl;
        std::vector<std::string> friends_list;
        friends_list.push_back("Maison");
        friends_list.push_back("Gratian");
        friends_list.push_back("Francine");
        return friends_list;
    }

    // override
    void GetfriendsList(google::protobuf::RpcController *controller,
                        const ::example_service::GetfriendsListRequest *request,
                        ::example_service::GetfriendsListResponse *response,
                        ::google::protobuf::Closure *done)
    {
        uint32_t user_id = request->user_id();
        std::vector<std::string> friends_list = GetfriendsList(user_id);
        response->mutable_result()->set_error_code(0);
        response->mutable_result()->set_error_msg("");
        for (auto &name : friends_list)
        {
            auto *p = response->add_friend_();
            *p = name;
        }
        done->Run();
    }
};

int main(int argc, char **argv)
{
    LOG_ERR("Test LOG_ERR");
    LOG_INFO("friendservice starting...");

    // rpc framework initialization
    RpcProvider provider;
    provider.NotifyService(new FriendService());

    // activate a rpc node
    provider.Run(); // current progress blocking
    return 0;
}