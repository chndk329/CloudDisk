#include "../include/CloudDiskServer.h"
#include "../include/Token.h"
#include "../include/genPhone.h"
#include "../include/Hash.h"

#include <workflow/MySQLResult.h>
#include <wfrest/json.hpp>
#include <sys/stat.h> // for mkdir
#include <fstream> // for ofstream

using std::string;
using std::map;
using std::vector;
using std::ofstream;

// 读取配置文件
#include "../include/Configuration.h"
Configuration& conf = Configuration::getInstance();

// -llog4cpp -lpthread
#include "../include/Mylogger.h"

// 密码加密
#include "../include/EncryptPassword.h"

CloudDiskServer::CloudDiskServer()
    : http_server()
      , wait_group(1)
/* v2.0 */
      /* , oss_uploader() */
/* v2.0 */
/* v3.0 */
      , publisher()
/* v3.0 */
{}

void CloudDiskServer::start(unsigned short port) {
    // 当客户端访问服务器时，希望看到客户端请求的信息记录 - track()
    // track() 的返回值是HttpServer的引用
    if (http_server.track().start(port) == 0) {
        LogInfo("CloudDiskServer start success!");
        loadModules(); // 注册接口
        // 当服务器启动时，希望看到已经部署好的接口信息 - POST、GET -> list_routes()
        http_server.list_routes();
        wait_group.wait();
    }
    else {
        LogError("CloudDiskServer start failed!");
    }
}

// 注册接口
void CloudDiskServer::loadModules() {
    loadStaticResources();
    loadSignUpModule();
    loadSignInModule();
    loadUserInfoModule();
    loadUserFileListModule();
    loadFileUploadModule();
    loadFileDownloadModule();
}

// 加载静态资源
void CloudDiskServer::loadStaticResources() {
    http_server.GET("/user/signup", [](const HttpReq*, HttpResp* resp) {
        resp->File(conf["signup_html"]);
    });

    http_server.GET("/static/view/signin.html", [](const HttpReq*, HttpResp* resp) {
        resp->File(conf["signin_html"]);
    });

    http_server.GET("/static/view/home.html", [](const HttpReq*, HttpResp* resp) {
        resp->File(conf["home_html"]);
    });

    http_server.GET("/static/js/auth.js", [](const HttpReq*, HttpResp* resp) {
        resp->File(conf["auth_js"]);
    });

    http_server.GET("/static/img/avatar.jpeg", [](const HttpReq*, HttpResp* resp) {
        resp->File(conf["avatar_jpeg"]);
    });

    http_server.GET("/file/upload", [](const HttpReq*, HttpResp* resp) {
        resp->File(conf["index_html"]);
    });

    http_server.GET("/file/upload/success", [](const HttpReq*, HttpResp* resp) {
        resp->File(conf["upload_success_html"]);
    });

}

// 注册
void CloudDiskServer::loadSignUpModule() {
    http_server.POST("/user/signup", [](const HttpReq* req, HttpResp* resp, SeriesWork* series) {
        // 怎么知道这个返回的是一个x-www-urlencoded
        // 直接使用wireshark抓包
        // APPLICATION_URLENCODED 这个宏定义在 HttpDef.h
        if (req->content_type() == APPLICATION_URLENCODED) {
            // 1. 解析请求
            map<string, string>& form_kv = req->form_kv(); 
            string username = form_kv["username"];
            string password = form_kv["password"];

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
            auto mysql_task = WFTaskFactory::create_mysql_task(mysql_url, 1, [resp](WFMySQLTask* mysql_task){
                // 对MySQL任务进行状态检测
                int state = mysql_task->get_state();
                int error = mysql_task->get_error();
                if (state != WFT_STATE_SUCCESS) {
                    string error_str = WFGlobal::get_error_string(state, error);
                    LogError("%s", error_str.c_str());
                    // ErrorCode 在 ErrorCode.h
                    resp->Error(error, error_str); // 返回一个json
                    return;
                }

                // 对SQL进行语法检测
                protocol::MySQLResponse* mysql_resp = mysql_task->get_resp();
                if (mysql_resp->get_packet_type() == MYSQL_PACKET_ERROR) {
                    int error_code = mysql_resp->get_error_code();
                    string error_msg = mysql_resp->get_error_msg();
                    LogError("ERROR %d: %s", error_code, error_msg.c_str());
                    resp->Error(error_code, error_msg);
                    return;
                }

                protocol::MySQLResultCursor cursor(mysql_resp);
                if (cursor.get_cursor_status() == MYSQL_STATUS_OK) {
                    // 写操作正常执行
                    int affected_rows = cursor.get_affected_rows();
                    string suffix = affected_rows > 1 ? " rows affected" : " row affected";
                    string res = std::to_string(affected_rows) + suffix;
                    LogInfo("Query OK, %s", res.c_str());
                }
                else {
                    // 简单返回错误信息
                    resp->String("FAILED");
                }
            });
            mysql_task->get_req()->set_query(sql);
            series->push_back(mysql_task);

            // 5. 响应
            // 成功必须返回SUCCESS，前端页面已经写好了
            resp->String("SUCCESS");
        }
        else {
            resp->String("FAILED");
        }
    });
}

// 登录 
void CloudDiskServer::loadSignInModule() {
    http_server.POST("/user/signin", [](const HttpReq* req, HttpResp* resp, SeriesWork* series) {
        if (req->content_type() == APPLICATION_URLENCODED) {
            // 1. 解析请求
            map<string, string>& form_kv = req->form_kv(); 
            string username = form_kv["username"];
            string password = form_kv["password"];

            // 2. 从数据库中获取salt和加密后的密码
            string sql = "SELECT salt, user_pwd FROM cloud_disk.tbl_user WHERE user_name = '";
            sql += username + "' limit 1";
            LogInfo("%s", sql.c_str());

            const string mysql_url = conf["mysql_url"];
            // 这里需要回调函数，因为是读操作，需要在回调中进行处理 - 登录验证
            auto mysql_task = WFTaskFactory::create_mysql_task(mysql_url, 1, 
            [resp, password, username, mysql_url, series](WFMySQLTask* mysql_task) {
                // 对MySQL任务进行状态检测
                int state = mysql_task->get_state();
                int error = mysql_task->get_error();
                if (state != WFT_STATE_SUCCESS) {
                    string error_str = WFGlobal::get_error_string(state, error);
                    LogError("%s", error_str.c_str());
                    resp->Error(error, error_str);
                    return;
                }

                // 对SQL进行语法检测
                protocol::MySQLResponse* mysql_resp = mysql_task->get_resp();
                if (mysql_resp->get_packet_type() == MYSQL_PACKET_ERROR) {
                    int error_code = mysql_resp->get_error_code();
                    string error_msg = mysql_resp->get_error_msg();
                    LogError("ERROR %d: %s", error_code, error_msg.c_str());
                    resp->Error(error_code, error_msg);
                    return;
                }

                protocol::MySQLResultCursor cursor(mysql_resp);
                if (cursor.get_cursor_status() == MYSQL_STATUS_GET_RESULT) {
                    // 读操作正常执行
                    vector<vector<protocol::MySQLCell>> rows; 
                    cursor.fetch_all(rows);

                    string rows_size = std::to_string(rows.size());
                    string suffix = rows.size() > 1 ? " rows in set." : " row in set";
                    string res = rows_size + suffix;
                    LogInfo("%s", res.c_str());

                    if (rows[0][0].is_string() && rows[0][1].is_string()) {
                        string salt = rows[0][0].as_string();
                        string db_pwd = rows[0][1].as_string(); // 数据库中存储的对应用户的加密密码
                        string in_pwd = encryptPassword(password, salt); // 加密用户登录输入的密码
                        if (db_pwd == in_pwd) { // 登录成功
                            // 生成Token
                            Token token(username, salt);
                            string gen_token = token.generateToken();

                            // 将最新的Token写入Redis - 由Redis来控制token的过期时间
                            const string redis_url = conf["redis_url"];
                            auto redis_task = WFTaskFactory::create_redis_task(redis_url, 1, [redis_url, username, series](WFRedisTask* ){
                                auto redis_task2 = WFTaskFactory::create_redis_task(redis_url ,1, nullptr);
                                // 设置过期时间 - 30 * 60s - 30min
                                // Web应用程序一般设置为15min~1h
                                redis_task2->get_req()->set_request("EXPIRE", {username, "1800"});
                                string cmd = "EXPIRE " + username + " 1800";
                                LogInfo("%s", cmd.c_str());
                                series->push_back(redis_task2);
                                
                            });
                            redis_task->get_req()->set_request("SET", {username, gen_token});
                            string cmd = "SET " + username + " " + gen_token;
                            LogInfo("%s", cmd.c_str());
                            series->push_back(redis_task);
                            
                            // token写入MySQL 已经写好了就不删了
                            // 将最新的Token写入MySQL - replace
                            // 这里就不再对写入数据库后的结果做判断了
                            auto mysql_task2 = WFTaskFactory::create_mysql_task(mysql_url, 1, nullptr);
                            string sql2 = "REPLACE INTO cloud_disk.tbl_user_token (user_name, user_token) VALUES ('";
                            sql2 += username + "', '" + gen_token + "')";
                            LogInfo("%s", sql2.c_str());
                            mysql_task2->get_req()->set_query(sql2.c_str());
                            series->push_back(mysql_task2);

                            // 响应
                            // 登录成功的话 前端页面需要服务器返回一个json
                            using Json = nlohmann::json;
                            Json resp_json;
                            Json data;
                            data["Token"] = gen_token;
                            data["Username"] = username;
                            data["Location"] = "/static/view/home.html"; // 注意：这里返回的是route，而不是服务器中home.html的存放路径
                            resp_json["data"] = data;
                            /* resp->Json(resp_json); */
                            // 不能直接发送一个json，前端解析不出来，只能发送一个string
                            resp->String(resp_json.dump());
                            LogInfo(resp_json.dump().c_str());

                        } else { resp->String("FAILED"); }
                    } else { resp->String("FAILED"); }
                } else { resp->String("FAILED"); }

            });
            mysql_task->get_req()->set_query(sql);
            series->push_back(mysql_task);
        }                  
    });
}

// 加载用户信息
void CloudDiskServer::loadUserInfoModule() {
    http_server.GET("/user/info", [](const HttpReq* req, HttpResp* resp, SeriesWork* series){
        // 1. 解析请求
        /* auto& query_list = req->query_list(); */
        // 因为以及确切知道查询词了，所以直接使用query来获取
        string username = req->query("username");
        string token = req->query("token");
        LogInfo("username = %s, token = %s", username.c_str(), token.c_str());

        // 2. 校验token
        const string redis_url = conf["redis_url"];
        // GitHub仓库中的代码是支持下面这种写法的，但是v0.9.3版本的代码中不支持
        /* resp->Redis(redis_url, "GET", {username}, [](WFRedisTask* redis_task){}); */
        auto redis_task = WFTaskFactory::create_redis_task(redis_url, 1, [token, resp, username](WFRedisTask* redis_task){
            protocol::RedisValue res;
            redis_task->get_resp()->get_result(res);
            if (res.is_string()) {
                string token_redis = res.string_value();
                if (token == token_redis) { // token校验成功
                    // 3. 查询数据库，获取用户的注册时间、邮箱、手机号
                    const string mysql_url = conf["mysql_url"];
                    string sql = "SELECT email, phone, signup_at FROM cloud_disk.tbl_user WHERE user_name = '";
                    sql += username + "' limit 1";
                    LogInfo("%s", sql.c_str());

                    // Redis不支持，但是MySQL还是支持的，换这种访问方式看下
                    // 这里就不对MySQL任务进行状态检查以及sql语法检测了
                    resp->MySQL(mysql_url, sql, [username, resp](protocol::MySQLResultCursor* cursor){
                        vector<vector<protocol::MySQLCell>> rows;
                        cursor->fetch_all(rows);

                        string rows_size = std::to_string(rows.size());
                        string suffix = rows.size() > 1 ? " rows in set." : " row in set";
                        string res = rows_size + suffix;
                        LogInfo("%s", res.c_str());

                        // 4. 响应
                        if (rows[0][0].is_string() && rows[0][1].is_string() && rows[0][2].is_datetime()) {
                            using Json  = nlohmann::json;
                            Json resp_json;
                            Json data;
                            data["Username"] = username;
                            data["Email"] = rows[0][0].as_string();
                            data["Phone"] = rows[0][1].as_string();
                            data["SignupAt"] = rows[0][2].as_datetime(); // as_datetime() return string
                            resp_json["data"] = data;
                            resp->String(resp_json.dump());
                            LogInfo(resp_json.dump().c_str());
                        }
                        else {
                            LogError("User info get failed");
                            resp->String("FAILED");
                        }
                    });

                } 
                else { 
                    LogError("token error");
                    resp->String("FAILED"); 
                }
            } 
            else { 
                LogError("token expired");
                resp->String("FAILED"); 
            }

        });
        redis_task->get_req()->set_request("GET", {username});
        string cmd = "GET " + username;
        LogInfo("%s", cmd.c_str());
        series->push_back(redis_task);

    });
}

// 加载用户文件列表
void CloudDiskServer::loadUserFileListModule() {
    http_server.POST("/file/query", [](const HttpReq* req, HttpResp* resp, SeriesWork* series){
        // 1. 解析请求
        string username = req->query("username");
        string token = req->query("token");
        auto& form_kv = req->form_kv();
        string limit = form_kv["limit"];
        LogInfo("username = %s, token = %s, limit = %s", username.c_str(), token.c_str(), limit.c_str());

        // 2. 校验token
        const string redis_url = conf["redis_url"];
        auto redis_task = WFTaskFactory::create_redis_task(redis_url, 1, [token, resp, username, limit](WFRedisTask* redis_task){
            protocol::RedisValue res;
            redis_task->get_resp()->get_result(res);
            if (res.is_string()) {
                string token_redis = res.string_value();
                if (token == token_redis) { // token校验成功
                    // 3. 查询数据库，获取用户文件的hash值、文件大小、文件名、上传时间、最后修改时间
                    const string mysql_url = conf["mysql_url"];
                    string sql = "SELECT file_hash, file_size, file_name, upload_at, last_update";
                    sql += " FROM cloud_disk.tbl_user_file WHERE user_name = '";
                    sql += username + "' limit " + limit;
                    LogInfo("%s", sql.c_str());

                    resp->MySQL(mysql_url, sql, [username, resp](protocol::MySQLResultCursor* cursor){
                        vector<vector<protocol::MySQLCell>> rows;
                        cursor->fetch_all(rows);

                        string rows_size = std::to_string(rows.size());
                        string suffix = rows.size() > 1 ? " rows in set." : " row in set";
                        string res = rows_size + suffix;
                        LogInfo("%s", res.c_str());
                        if (rows.size() == 0) { 
                            resp->String("Failed");
                            return;
                        }

                        // 4. 响应
                        using Json = nlohmann::json;
                        Json resp_json;
                        for (size_t i = 0; i < rows.size(); ++i) {
                            Json row;
                            row["FileHash"] = rows[i][0].as_string();
                            row["FileName"] = rows[i][2].as_string();
                            row["FileSize"] = rows[i][1].as_ulonglong(); // bigint
                            row["UploadAt"] = rows[i][3].as_datetime();
                            row["LastUpdated"] = rows[i][4].as_datetime();
                            resp_json.push_back(row);
                            LogInfo(row.dump().c_str());
                        }
                        resp->String(resp_json.dump());
                    });

                } 
                else { 
                    LogError("token error");
                    resp->String("FAILED"); 
                }
            } 
            else { 
                LogError("token expired");
                resp->String("FAILED"); 
            }

        });
        redis_task->get_req()->set_request("GET", {username});
        string cmd = "GET " + username;
        LogInfo("%s", cmd.c_str());
        series->push_back(redis_task);

            
    });
}

// 上传文件
void CloudDiskServer::loadFileUploadModule() {
    http_server.POST("/file/upload", [this](const HttpReq* req, HttpResp* resp, SeriesWork* series) {
        if (req->content_type() == MULTIPART_FORM_DATA) {
            // 1. 解析请求
            string username = req->query("username");
            string token = req->query("token");
            auto& form = req->form();
            string filename = form["file"].first;
            string content = form["file"].second;
            LogInfo("username = %s, token = %s, filename = %s", username.c_str(), token.c_str(), filename.c_str());

            // 2. 校验token
            const string redis_url = conf["redis_url"];
            auto redis_task = WFTaskFactory::create_redis_task(redis_url, 1, [resp, username, token, filename, content, series, this](WFRedisTask* redis_task){
                protocol::RedisValue res;
                redis_task->get_resp()->get_result(res);
                if (res.is_string()) {
                    string token_redis = res.string_value();
                    if (token == token_redis) { // token校验成功
                        // 2. 将文件内容写入本地
                        // 注意：如果是使用虚拟文件表的话，这里逻辑不同
                        string dirname = "./" + username + "/";
                        mkdir(dirname.c_str(), 0755);
                        string filepath = dirname + filename;
                        ofstream ofs(filepath);
                        if (ofs.is_open()) {
                            ofs << content;
                            ofs.close();
                            LogInfo("create %s complete", filepath.c_str());
                        }
                        else {
                            ofs.close();
                            LogError("open %s failed!", filepath.c_str());
                            resp->String("FAILED");
                            return;
                        }

/* v2.0 */
                        // 2.1 备份文件到阿里云OSS
                        // 阿里云上的Bucket也创建同名目录，并创建同名文件
                        string object_name = username + "/" + filename;
                        /* oss_uploader.doUpload(filepath, object_name); */
/* v2.0 */

/* v3.0 */
                        // v3.0优化：
                        // 将要备份的文件添加到消息队列中
                        // 不需要立刻执行备份操作，因此只需要实现一个RabbitMQ的生产者即可
                        // 消息本身可以采用JSON格式进行传输
                        using Json = nlohmann::json;
                        Json uploader_info;
                        uploader_info["filePath"] = filepath;
                        uploader_info["objectName"] = object_name;
                        publisher.publish(uploader_info.dump());
/* v3.0 */
                        
                        // 3. 重定向到个人中心页面
                        resp->headers["Location"] = "/static/view/home.html";
                        resp->set_status_code("301");
                        resp->set_reason_phrase("Moved Permanently");

                        // 4. 将文件信息更新到数据库中
                        Hash hash(filepath);
                        string filehash = hash.sha1();
                        const string mysql_url = conf["mysql_url"];
                        auto mysql_task = WFTaskFactory::create_mysql_task(mysql_url, 1, nullptr);
                        string sql = "INSERT INTO cloud_disk.tbl_user_file(user_name, file_hash, file_size, file_name) ";
                        sql += "VALUES('" + username + "', '" + filehash + "', " + std::to_string(content.size()) + ", '" + filename + "')";
                        LogInfo("%s", sql.c_str());
                        mysql_task->get_req()->set_query(sql);
                        series->push_back(mysql_task);
                    } 
                    else { 
                        LogError("token error");
                        resp->String("FAILED"); 
                    }
                } 
                else { 
                    LogError("token expired");
                    resp->String("FAILED"); 
                }

            });
            redis_task->get_req()->set_request("GET", {username});
            string cmd = "GET " + username;
            LogInfo("%s", cmd.c_str());
            series->push_back(redis_task);


        }
    });
}

// 下载文件
void CloudDiskServer::loadFileDownloadModule() {
    http_server.GET("/file/downloadurl", [](const HttpReq* req, HttpResp* resp, SeriesWork* series) {
        // 1. 解析请求
        string username = req->query("username");
        string token = req->query("token");
        string filename = req->query("filename");
        LogInfo("username = %s, token = %s, filename = %s", username.c_str(), token.c_str(), filename.c_str());

        // 2. 校验token
        const string redis_url = conf["redis_url"];
        auto redis_task = WFTaskFactory::create_redis_task(redis_url, 1, [resp, username, token, filename](WFRedisTask* redis_task){
            protocol::RedisValue res;
            redis_task->get_resp()->get_result(res);
            if (res.is_string()) {
                string token_redis = res.string_value();
                if (token == token_redis) { // token校验成功
                    // 4. 响应
                    // 返回下载链接
                    string downloadurl = "http://192.168.10.128:8888/" + filename;
                    // 客户端获取到下载链接之后，由nginx负责通过这个下载链接进行下载
                    resp->String(downloadurl);
                } 
                else { 
                    LogError("token error");
                    resp->String("FAILED"); 
                }
            } 
            else { 
                LogError("token expired");
                resp->String("FAILED"); 
            }

        });
        redis_task->get_req()->set_request("GET", {username});
        string cmd = "GET " + username;
        LogInfo("%s", cmd.c_str());
        series->push_back(redis_task);

            
    });
}
