#ifndef __Configuration_H__
#define __Configuration_H__

#include "Singleton.h"

// map适用于需要进行频繁插入和删除的场景
// unordered_map适用于快速查找的场景
#include <unordered_map> // for unordered_map
#include <string> // for string
#include <fstream> // for ifstream
#include <iostream> // for cerr, endl
#include <sstream> // for istringstream

using std::unordered_map;
using std::string;
using std::ifstream;
using std::cerr;
using std::endl;
using std::istringstream;

// 单例类
// 就两个函数，直接在这写完好了，不然每次编译的时候都需要去包含实现文件，麻烦
class Configuration : public Singleton<Configuration>
{
private:
    // 因为只有一个数据成员，所以直接写在最前面了，不然后面都是函数实现看不到了
    unordered_map<string, string> conf_map;

public:
    Configuration() {
        const string conf_path = "../conf/CloudDisk.conf";
        ifstream ifs(conf_path);
        if (!ifs.good()) {
            cerr << "open " << conf_path << "failed!" << endl;
            ifs.close();
            return;
        }

        string line;
        while (getline(ifs, line)) {
            istringstream iss(line);
            string key, value;
            iss >> key >> value;
            if (key[0] == '#') {
                // 跳过注释
                continue;
            }
            conf_map[key] = value;
        }

        ifs.close();
    }

    string operator[](const string& key) {
        if (conf_map.count(key) == 0) {
            return nullptr;
        }
        return conf_map[key];
    }

};

#endif
