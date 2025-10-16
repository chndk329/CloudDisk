#include "../include/Hash.h"
#include <iostream>
using namespace std;

void test() {
    Hash hash("./Token.cc");
    cout << hash.sha1() << endl;
}

int main(void)
{
    test();
    return 0;
}

