#ifndef __Singleton_H__
#define __Singleton_H__

// 单例类，需要创建哪个类的单例对象就填入哪个类
template<typename T>
class Singleton
{
public:
    static T& getInstance() {
        static T t;
        return t;
    }

    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
protected:
    Singleton() = default;
    ~Singleton() = default;
};

#endif

