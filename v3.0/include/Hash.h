#ifndef __Hash_H__
#define __Hash_H__

#include <string>
using std::string;

class Hash
{
public:
    Hash(const string& filename_);

    string sha1() const;

private:
    string filename;
};

#endif
