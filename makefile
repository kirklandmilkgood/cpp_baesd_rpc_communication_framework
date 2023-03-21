CXX ?= g++

DEBUG ?= 1;
ifeq ($(DEBUG), 1)
	CXXFLAGS += -g
else
	CXXFLAGS += -O2

endif

rpc_callee: ./example/callee/friendservice.cpp ./example/friend.pb.cc ./src/logger.cpp ./src/mprpc_application.cpp ./src/mprpc_channel.cpp ./src/mprpc_config.cpp ./src/mprpc_controller.cpp \
	./src/rpcheader.pb.cc ./src/rpcprovider.cpp ./src/zookeeper_util.cpp
	$(CXX) -o rpc_callee $^ $(CXXFLAGS) -I$(shell pwd)/include -I$(shell pwd)/example -L/usr/lib/muduo -lprotobuf -lpthread -lmuduo_net -lmuduo_base -lzookeeper_mt

clean:
	rm -r rpc_callee