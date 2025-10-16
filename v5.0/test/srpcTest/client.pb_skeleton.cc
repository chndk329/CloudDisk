#include "user.srpc.h"
#include "workflow/WFFacilities.h"

using namespace srpc;

#include <iostream>

static WFFacilities::WaitGroup wait_group(1);

void sig_handler(int signo)
{
	wait_group.done();
}

static void signup_done(RespSignup *response, srpc::RPCContext *context)
{
    using std::cout;
    using std::endl;
    using std::string;

    // 当执行该函数时，客户端就已经收到了RespSignup的数据了，这是个回调函数
    int code = response->code();
    string msg = response->msg();

    cout << "code = " << code << endl;
    cout << "msg = " << msg << endl;
}

int main()
{
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	const char *ip = "127.0.0.1";
	unsigned short port = 1412;

    // UserService 变成了一个命名空间
	UserService::SRPCClient client(ip, port);

	// example for RPC method call
	ReqSignup signup_req;
	//signup_req.set_message("Hello, srpc!");
    
    // 组装一个ReqSignup消息
    signup_req.set_username("Tvux");
    signup_req.set_password("123");

    // 客户端发起一个RPC请求，类似于函数调用
	client.Signup(&signup_req, signup_done);

	wait_group.wait();
	google::protobuf::ShutdownProtobufLibrary();
	return 0;
}
