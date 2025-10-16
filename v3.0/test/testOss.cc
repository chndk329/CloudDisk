#include "../include/OssUploader.h"
#include <iostream>
using namespace std;

void test() {
    OssUploader oss_uploader;
    oss_uploader.doUpload("./testUpload.txt", "Tvux/testUpload.txt");
}

int main(void)
{
    test();
    return 0;
}

