#ifndef __Amqp_H__
#define __Amqp_H__

#include <SimpleAmqpClient/SimpleAmqpClient.h>
using namespace AmqpClient;

#include <string>
using std::string;

struct AmqpInfo
{
    const string amqp_url = "amqp://guest:guest@localhost:5672";
    const string Exchange = "uploadserver.trans";
    const string Queue = "ossqueue";
    const string Routing_key = "oss";
};

class Publisher {
public:
    Publisher()
        : channel(AmqpClient::Channel::Create())
    {}

    void publish(const string& msg) {
        BasicMessage::ptr_t message = BasicMessage::Create(msg);
        channel->BasicPublish(amqp_info.Exchange, amqp_info.Routing_key, message);
    }

private:
    AmqpInfo amqp_info;
    Channel::ptr_t channel;
};

class Consumer {
public:
    Consumer()
        : channel(AmqpClient::Channel::Create())
    {
        // 指定从某一个队列中获取数据
        channel->BasicConsume(amqp_info.Queue);
    }

    // 消费到一条就返回一条
    bool consume(string& msg) {
        Envelope::ptr_t envelope;
        bool ret = channel->BasicConsumeMessage(envelope, 3000);
        if (ret) {
            msg = envelope->Message()->Body();
        }
        return ret;
    }

private:
    AmqpInfo amqp_info;
    Channel::ptr_t channel;
};

#endif
