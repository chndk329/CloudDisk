#ifndef __genPhone_H__
#define __genPhone_H__

#include <string>
#include <ctime> // for time
#include <cstdlib> // for srand
using std::string;

string generatePhone() {
    srand(time(0));

    string phone;
    // 手机号为11位
    for (int i = 0; i < 11; ++i) {
        int num = rand() % 10; // 生成随机数0~9
        phone += std::to_string(num);
    }

    return phone;
}

#endif
