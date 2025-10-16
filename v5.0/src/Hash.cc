#if 0
这些警告是由于 OpenSSL 3.0 中的 SHA-1 函数被标记为已弃用导致的。在新版本的 OpenSSL 中，SHA-1 函数已被标记为不推荐使用，因为 SHA-1 算法存在安全性漏洞。
为了解决这些警告，您可以考虑使用更安全的哈希算法，如 SHA-256、SHA-3 或 SHA-512，以替代 SHA-1。这些算法提供更高的安全性，并且不会触发弃用警告。
如果您仍然希望使用 SHA-1，可以通过在编译选项中添加 -Wno-deprecated-declarations 来禁用弃用警告。但请注意，这样做可能会带来潜在的安全风险，因为 SHA-1 算法已被证明不再安全。
#endif
#include "../include/Hash.h"
#include <openssl/sha.h> // -lssl
#include <strings.h> // for bzero
#include <sys/stat.h> // for open
#include <fcntl.h> // for open
#include <unistd.h> // for read

Hash::Hash(const string& filename_)
    : filename(filename_)
{}

string Hash::sha1() const {
    int fd = open(filename.c_str(), O_RDONLY);
    if(fd == -1) {
        perror("open");
        return string();
    }
    char buff[1024] = {0};
    SHA_CTX ctx;
    SHA1_Init(&ctx);

    while(1) {
        int ret = read(fd, buff, 1024);
        if(ret == 0) {
            break;
        }
        SHA1_Update(&ctx, buff, ret);
        bzero(buff, sizeof(buff));
    }
    unsigned char md[20] = {0};
    SHA1_Final(md, &ctx);
    char fragment[3] = {0};
    string result;
    for(int i = 0; i < 20; ++i) {
        sprintf(fragment, "%02x", md[i]);
        result += fragment;
    }
    return result;
}
