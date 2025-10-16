#ifndef __EncryptPassword_H__
#define __EncryptPassword_H__

#include <string>
#include <ctime>
#include <vector>
// -lcrypt
#include <crypt.h>


using std::string;
using std::vector;

// 随机生成盐值
// 默认生成16位的盐值
string generateSalt(int length = 16) {
    const string lower = "abcdefghijklmnopqrstuvwxyz";
    const string upper = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const string digit = "0123456789";
    // 26 + 26 + 10 = 62
    const string characters = lower + upper + digit;

    vector<int> indexs(length);
    srand(time(0));
    for (int i = 0; i < length; ++i) {
        indexs[i] = rand() % 62; // 随机0 ~ 61
    }

    string salt;
    for (int i = 0; i < length; ++i) {
        salt += characters[indexs[i]];
    }

    // 存储在/etc/shadow文件中的密码哈希值通常以以下格式表示：$id$salt$hashed_password。
    // 其中，id是哈希算法的标识符，salt是随机生成的盐值，hashed_password是最终的密码哈希值。
    return "$6$" + salt + "$";
    // 直接返回这个值存到数据库中，这样登录验证的时候方便一些
}

// 加密密码
string encryptPassword(const string& password, const string& salt) {
    return crypt(password.c_str(), salt.c_str()); // 106位的加密密码
}

#endif
