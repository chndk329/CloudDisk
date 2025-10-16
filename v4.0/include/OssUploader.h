#ifndef __OssUploader_H__
#define __OssUploader_H__
#include <alibabacloud/oss/OssClient.h>

#include <string>

using std::string;


struct OssInfo
{
    //定制化数据,需要进行修改
    string AccessKeyId = "YourAccessKeyId";
    string AccessKeySecret = "YourAccessKeySecret";
    string Endpoint = "oss-cn-guangzhou.aliyuncs.com";
    string BucketName = "tvux";
};

// 将本地文件备份到阿里云OSS
class OssUploader
{
public:
    OssUploader(const OssInfo & info = OssInfo())
    : _info(info)
    , _conf()
    , _ossClient(_info.Endpoint, _info.AccessKeyId, _info.AccessKeySecret, _conf)
    {   AlibabaCloud::OSS::InitializeSdk();    }

    // filename：本地文件路径
  	// objectName：阿里云OSS上的目的路径
    bool doUpload(const string & filename, const string & objectName)
    {
        auto outcome = _ossClient.PutObject(_info.BucketName, objectName, filename);
        
        bool ret = outcome.isSuccess();
        if(!ret) {
            std::cout << "PutObject fail" <<
            ",code:" << outcome.error().Code() <<
            ",message:" << outcome.error().Message() <<
            ",requestId:" << outcome.error().RequestId() << std::endl;
        }
        return ret;
    }

    ~OssUploader() {    AlibabaCloud::OSS::ShutdownSdk();  }

private:
    OssInfo _info;
    AlibabaCloud::OSS::ClientConfiguration _conf;
    AlibabaCloud::OSS::OssClient _ossClient;
};

#endif
