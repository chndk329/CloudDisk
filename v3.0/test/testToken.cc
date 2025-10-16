#include "../include/Token.h"
#include <iostream>
using namespace std;

void test() {
   Token token("Tvux", "$6$PKdhtXMmr18n2L9K$"); 
   string res = token.generateToken();
   cout << "token = " << res << endl;
}

int main(void)
{
    test();
    return 0;
}

