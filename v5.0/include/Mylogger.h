// 需要链接log4cpp pthread库
#ifndef __Mylogger_H__
#define __Mylogger_H__

#include "../include/Singleton.h"
#include "../include/Configuration.h"

#include <log4cpp/PatternLayout.hh> // for PatternLayout
#include <log4cpp/OstreamAppender.hh> // for OstreamAppender
#include <log4cpp/FileAppender.hh> // for FileAppender
#include <log4cpp/Category.hh> // for Category
using namespace log4cpp;

#include <iostream>
using std::cout;
using std::stringstream;

class Mylogger : public Singleton<Mylogger>
{
public:
    // 模板直接就在这实现，分文件麻烦，而且模板的实现文件一般命名为.tcc，还有类实现的代码
    // 刚好模板的实现代码也不多，直接就在这实现了
    template <typename... Args>
    void info(const char* msg, const Args& ... rest) { logger.info(msg, rest...); }

    template <typename... Args>
    void error(const char* msg, const Args& ... rest) { logger.error(msg, rest...); }

    template <typename... Args>
    void warn(const char* msg, const Args& ... rest) { logger.warn(msg, rest...); }

    template <typename... Args>
    void debug(const char* msg, const Args& ... rest) { logger.debug(msg, rest...); }
    
    Mylogger()
        : logger(Category::getRoot().getInstance("logger"))
    {
        // 1. 设置日志布局
        // 即使布局一样也需要一人一个，因为绑定就没了
        PatternLayout* ptn1 = new PatternLayout();
        ptn1->setConversionPattern("%d %c [%p] %m%n");

        PatternLayout* ptn2 = new PatternLayout();
        ptn2->setConversionPattern("%d %c [%p] %m%n");

        // 2. 设置日志输出目的地
        OstreamAppender* ostreamApp = new OstreamAppender("concole", &cout);
        ostreamApp->setLayout(ptn1);

        FileAppender* fileApp = new FileAppender("file", Configuration::getInstance()["log_path"]);
        fileApp->setLayout(ptn2);

        // 3.1 设置模块本身的优先级
        logger.setPriority(Priority::DEBUG);

        // 3.2 设置日志目的地
        logger.setAppender(ostreamApp);
        logger.setAppender(fileApp);

    }

    ~Mylogger() {
        Category::shutdown();
    }

private:
    // 引用，则必须在初始化列表中初始化
    Category& logger;
};

// 先使用字符串IO拼接前缀 [xxx.cc: yyy(): zzz]
// 再调用str()，stringstream -> string
// 接着调用append()将传入的内容format拼接在后面，期间需要为format构造一个临时的string对象才能调用append
// 最后 string -> const char* 传入info中，还有后面的参数
#define LogInfo(format, ...) {\
    stringstream ss;\
    ss << "[" << __FILE__ << ":" << __FUNCTION__ << "():" << __LINE__ << "] ";\
    Mylogger::getInstance().info(ss.str().append(string(format)).c_str(), ##__VA_ARGS__);\
}

#define LogError(format, ...) {\
    stringstream ss;\
    ss << "[" << __FILE__ << ":" << __FUNCTION__ << "():" << __LINE__ << "] ";\
    Mylogger::getInstance().error(ss.str().append(string(format)).c_str(), ##__VA_ARGS__);\
}

#define LogWarn(format, ...) {\
    stringstream ss;\
    ss << "[" << __FILE__ << ":" << __FUNCTION__ << "():" << __LINE__ << "] ";\
    Mylogger::getInstance().warn(ss.str().append(string(format)).c_str(), ##__VA_ARGS__);\
}

#define LogDebug(format, ...) {\
    stringstream ss;\
    ss << "[" << __FILE__ << ":" << __FUNCTION__ << "():" << __LINE__ << "] ";\
    Mylogger::getInstance().debug(ss.str().append(string(format)).c_str(), ##__VA_ARGS__);\
}

#endif
