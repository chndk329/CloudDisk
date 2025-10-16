#ifndef __Token_H__
#define __Token_H__

#include <string>
using std::string;

class Token
{
public:
    Token(const string& username_, const string& salt_);

    string generateToken() const;

private:
    string username;
    string salt;
};

#endif

