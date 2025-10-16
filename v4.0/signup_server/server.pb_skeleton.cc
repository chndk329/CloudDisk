#include "../include/user.srpc.h"
#include "../include/EncryptPassword.h" // 密码加密
#include "../include/genPhone.h" // 生成随机手机号
#include "../include/Mylogger.h" // 日志
#include "../include/Configuration.h" // 读取配置文件
Configuration& conf = Configuration::getInstance();

#include <workflow/MySQLResult.h>
#include <workflow/WFFacilities.h>

using namespace srpc;

static WFFacilities::WaitGroup wait_group(1);

void sig_handler(int signo)
{
	wait_group.done();
}

class UserServiceServiceImpl : public UserService::Service
{
public:

	void Signup(ReqSignup *request, RespSignup *response, srpc::RPCContext *ctx) override
	{
		// TODO: fill server logic here
        
        // 代理模式要实现该Signup函数
        // 当该函数执行时，服务器已经收到了ReqSignup数据了
        // 处理逻辑...
        // 接下来只需要填充RespSignup就可以了，由框架发回给客户端
        
        // 1. 解析请求
        std::string username = request->username();
        std::string password = request->password();

        // 2. 密码加密
        string salt = generateSalt();
        string encrypted_pwd = encryptPassword(password, salt);

        // 3. 生成随机手机号和邮箱
        string phone = generatePhone();
        string email = phone + "@qq.com";

        // 4，写入MySQL
        string sql = "INSERT INTO cloud_disk.tbl_user(user_name, salt, user_pwd, email, phone) VALUES('";
        sql += username + "', '" + salt + "', '" + encrypted_pwd + "', '" + email + "', '" + phone + "')";
        LogInfo(sql.c_str());

        const string mysql_url = conf["mysql_url"];
        auto mysql_task = WFTaskFactory::create_mysql_task(mysql_url, 1, [response, ctx](WFMySQLTask* mysql_task){
            // 对MySQL任务进行状态检测
            int state = mysql_task->get_state();
            int error = mysql_task->get_error();
            if (state != WFT_STATE_SUCCESS) {
                string error_str = WFGlobal::get_error_string(state, error);
                LogError("%s", error_str.c_str());

                response->set_code(-1);
                response->set_msg("Signup Failed");
                return;
            }

            // 对SQL进行语法检测
            protocol::MySQLResponse* mysql_resp = mysql_task->get_resp();
            if (mysql_resp->get_packet_type() == MYSQL_PACKET_ERROR) {
                int error_code = mysql_resp->get_error_code();
                string error_msg = mysql_resp->get_error_msg();
                LogError("ERROR %d: %s", error_code, error_msg.c_str());

                response->set_code(-1);
                response->set_msg("Signup Failed");
                return;
            }

            protocol::MySQLResultCursor cursor(mysql_resp);
            if (cursor.get_cursor_status() == MYSQL_STATUS_OK) {
                // 写操作正常执行
                int affected_rows = cursor.get_affected_rows();
                string suffix = affected_rows > 1 ? " rows affected" : " row affected";
                string res = std::to_string(affected_rows) + suffix;
                LogInfo("Query OK, %s", res.c_str());

                response->set_code(0);
                response->set_msg("OK");
            }
            else {
                // 简单返回错误信息
                response->set_code(-1);
                response->set_msg("Signup Failed");
            }
        });

        mysql_task->get_req()->set_query(sql);
        ctx->get_series()->push_back(mysql_task);
    }
};

int main()
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    unsigned short port = 1412;
    SRPCServer server;

    UserServiceServiceImpl userservice_impl;
    server.add_service(&userservice_impl);

    server.start(port);
    wait_group.wait();
    server.stop();
    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}
