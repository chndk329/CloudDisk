#include "../include/Configuration.h"

#include <iostream>
using namespace std;

void test1() {
    Configuration& conf = Configuration::getInstance();
    cout << conf["signup_route"] << endl;
}

void test2() {
    cout << Configuration::getInstance()["signup_route"] << endl;
}

int main(void)
{
    test1();
    test2();
    return 0;
}

