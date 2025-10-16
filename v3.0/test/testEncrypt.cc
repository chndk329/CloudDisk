#include "../include/EncryptPassword.h"
#include <iostream>
using namespace std;

void test() {
    string salt = generateSalt();
    string encrypted = encryptPassword("123456", salt);
    cout << "salt = " << salt << endl;
    cout << "encrypted password = " << encrypted << endl;
    cout << encrypted.size() << endl;
}

int main(void)
{
    test();
    return 0;
}

