#include <SimpleAmqpClient/SimpleAmqpClient.h> // -lSimpleAmqpClient

#include <iostream>
#include <string>
using namespace std;

struct AmqpInfo
{
    const string amqp_url = "amqp://guest:guest@localhost:5672";
    const string Exchange = "mytest";
    const string Queue = "queue1";
    const string Routing_key = "key1";
};

void createPublisher() {
    using namespace AmqpClient;
    AmqpInfo amqp_info;
    Channel::ptr_t channel = Channel::Create();

    BasicMessage::ptr_t message = BasicMessage::Create("this is testRabbitMQ");
    channel->BasicPublish(amqp_info.Exchange, amqp_info.Routing_key, message);

    // 暂停来看一下channel状态
    pause();
}

void createConsumer() {
    using namespace AmqpClient;
    AmqpInfo amqp_info;
    Channel::ptr_t channel = Channel::Create();

    // 指定从哪个队列中获取数据
    channel->BasicConsume(amqp_info.Queue);

    // 定义一个信封
    Envelope::ptr_t envelope;
    while (1) {
        bool ret = channel->BasicConsumeMessage(envelope, 3000);
        if (ret) {
            string msg = envelope->Message()->Body();
            cout << "consume msg: " << msg << endl;
        }
        else {
            cout << "get message timeout" << endl;
            break;
        }
    }
}

int main(void)
{
    createPublisher();
    /* createConsumer(); */
    return 0;
}

