#ifndef __CloudDiskServer_H__
#define __CloudDiskServer_H__

/* v2.0 */
/* #include "../include/OssUploader.h" */
/* v2.0 */

/* v3.0 */
#include "../include/Amqp.h"
/* v3.0 */

#include <wfrest/HttpServer.h> // for HttpServer
#include <workflow/WFFacilities.h> // for WaitGroup

using namespace wfrest;


class CloudDiskServer
{
public:
    CloudDiskServer();

    void start(unsigned short port);

private:
    // 注册接口
    void loadModules();

    /* 部署接口 */

    // 加载静态资源
    void loadStaticResources();
    // 注册
    void loadSignUpModule();
    // 登录
    void loadSignInModule();
    // 加载用户信息
    void loadUserInfoModule();
    // 加载用户文件列表
    void loadUserFileListModule();
    // 上传文件
    void loadFileUploadModule();
    // 下载文件
    void loadFileDownloadModule();

private:
    HttpServer http_server;
    WFFacilities::WaitGroup wait_group;

/* v2.0 */
    /* OssUploader oss_uploader; */
/* v2.0 */

/* v3.0 */
    Publisher publisher;
/* v3.0 */
};

// 扩展：可以写一个配置文件，然后使用Configuration单例类来读取
/* const string SIGN_UP_ROUTE = "/CloudDisk/user/signup"; */
/* const string SING_IN_ROUTE = "/CloudDisk/static/view/signin.html"; */
#endif
