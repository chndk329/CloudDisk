#include "user.pb.h"
// g++ testUser.cc user.pb.cc -lprotobuf

#include <iostream>
#include <string>
using namespace std;

void test() {
    ReqSignup req;
    req.set_username("Tvux");
    req.set_password("123");

    // 序列化
    string serialize_str;
    // 序列化为一个字符串
    req.SerializeToString(&serialize_str);
    // 序列化之后是一个二进制的形式
    for (size_t i = 0; i < serialize_str.size(); ++i) {
        // 以十六进制的形式输出serialize_str的每一个字节
        printf("%02x", serialize_str[i]);
    }
    printf("\n");
}

int main(void)
{
    test();
    return 0;
}

