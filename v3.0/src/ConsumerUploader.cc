#include "../include/Amqp.h"
#include "../include/OssUploader.h"
#include "../include/Mylogger.h"

#include <nlohmann/json.hpp>
using Json = nlohmann::json;

#include <iostream>
using namespace std;

int main(void)
{
    Consumer consumer;
    OssUploader oss_uploader;
    string msg;

    while (1) {
        // 不断读取消息队列中的消息
        bool hasMsg = consumer.consume(msg);
        if (hasMsg) {
            Json uploader_info = Json::parse(msg);
            string filepath = uploader_info["filePath"];
            string object_name = uploader_info["objectName"];
            LogInfo("filePath = %s, objectName = %s", filepath.c_str(), object_name.c_str());
            

            // 调用OssUploader的对象进行备份
            oss_uploader.doUpload(filepath, object_name);
        }
    }
    return 0;
}
