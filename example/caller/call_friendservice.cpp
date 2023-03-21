#include <iostream>
#include "../../include/mprpc_application.h"
#include "../../include/mprpc_channel.h"
#include "../friend.pb.h"

int main(int argc, char **argv)
{
    MprpcApplication::Init(argc, argv);

    example_service::FriendServiceRpc_Stub stub(new MprpcChannel());
    example_service::GetfriendsListRequest request;
    request.set_user_id(999);
    example_service::GetfriendsListResponse response;
    MprpcController controller;
    stub.GetfriendsList(&controller, &request, &response, nullptr);

    if (controller.Failed())
    {
        std::cout << controller.ErrorText() << std::endl;
    }
    else
    {
        if (response.result().error_code() == 0)
        {
            std::cout << "rpc GetFriendList response success: " << std::endl;
            int size = response.friend__size();
            for (int i = 0; i < size; i++)
            {
                std::cout << "index: " << (i + 1) << "name: " << response.friend_(i) << std::endl;
            }
        }
        else
        {
            std::cout << "rpc GetFriendsList response error: " << response.result().error_msg() << std::endl;
        }
    }
    return 0;
}