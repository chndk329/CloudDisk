#include "../include/Mylogger.h"
using namespace std;

void test() {
    int number = 100;
    const char* pstr = "hello, log4cpp";
    LogInfo("This is an info message. number = %d, &number = %p, str = %s\n", number, &number, pstr);
    LogError("This is an error message. number = %d, &number = %p, str = %s\n", number, &number, pstr);
    LogWarn("This is an warn message. number = %d, &number = %p, str = %s\n", number, &number, pstr);
    LogDebug("This is an debug message. number = %d, &number = %p, str = %s\n", number, &number, pstr);
}

int main(void)
{
    test();
    return 0;
}
