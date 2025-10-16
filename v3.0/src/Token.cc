#include "../include/Token.h"
#include <openssl/evp.h> // for EVP_xxx
// -lssl -lcrypto

Token::Token(const string& username_, const string& salt_)
    : username(username_), salt(salt_)
{}

// 先根据salt和username生成md5
// 然后md5 + 时间 -> token
string Token::generateToken() const {
    string tmp = salt + username;

    // $ md5sum home.html 
    // fecd68c16d22699ebd4e44cb61e9f602  home.html
    // 32个字符，但是使用十六进制，一个字节可以表示2个字符
    // 在 OpenSSL 中，MD5 散列算法生成的散列值长度固定为 16 字节
    unsigned char md5[EVP_MAX_MD_SIZE] = { 0 };
    unsigned int md5_len;

    // EVP_MD_CTX_new 创建一个md5上下文对象
    EVP_MD_CTX* md5ctx = EVP_MD_CTX_new();

    // 第一个参数：要初始化的散列算法上下文对象
    // 第二个参数：要使用的散列算法类型
    // 第三个参数：要使用的加密引擎
    EVP_DigestInit_ex(md5ctx, EVP_md5(), nullptr);

    // 第二个参数：指向要计算md5码的数据的指针
    // 第三个参数：要计算md5码的数据的长度
    EVP_DigestUpdate(md5ctx, (const unsigned char*)tmp.c_str(), tmp.size());

    // 计算md5，将结果存放在md5，长度存放在md5_len
    EVP_DigestFinal_ex(md5ctx, md5, &md5_len);

    // 释放 MD5 上下文对象
    EVP_MD_CTX_free(md5ctx);

    // fragment 用于存储每个散列结果的两位十六进制表示。
    char fragment[3] = { 0 };
    string res;
    for (int i = 0; i < 16; ++i) {
        sprintf(fragment, "%02x", md5[i]);
        res += fragment;
    }

    // 获取当前时间
    time_t secs = time(0);
    struct tm* ptm = localtime(&secs);

    // buf 用于存储格式化后的时间
    // 4 + 2 + 2 + 2 + 2 + 2 = 14
    // 年  月  日  时  分  秒
    char buf[15] = { 0 };
    sprintf(buf, "%04d%02d%02d%02d%02d%02d",
            ptm->tm_year + 1900,
            ptm->tm_mon + 1,
            ptm->tm_mday,
            ptm->tm_hour,
            ptm->tm_min,
            ptm->tm_sec);

    return res + buf;
}
