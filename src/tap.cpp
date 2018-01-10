#include "tap.h"

Tap::Tap(): publisherPort(0), running(false)
{

}

void Tap::init(const std::string& serverHost_in, int basePort)
{
    context.reset(new zmq::context_t(1));

    publisherPort = basePort + 10;
    uuid = lager_utils::getUuid();
    uuidBytes = lager_utils::getUuidBytes(uuid);

    // 2000 default timeout for testing
    chpClient.reset(new ChpClient(serverHost_in, basePort, 2000));
    chpClient->init(context, uuid);

    try
    {
        int linger = 0;
        publisher.reset(new zmq::socket_t(*context.get(), ZMQ_PUB));
        publisher->setsockopt(ZMQ_LINGER, &linger, sizeof(linger));
        publisher->connect(lager_utils::getRemoteUri(serverHost_in, publisherPort).c_str());

        std::cout << "tap::publisher: " << lager_utils::getRemoteUri(serverHost_in, publisherPort) << std::endl;
    }
    catch (zmq::error_t e)
    {
        std::cout << "publisher socket failed: " << e.what() << std::endl;
        return;
    }

    lager_utils::sleep(1000);
}

void Tap::start()
{
    running = true;

    chpClient->start();
}

void Tap::stop()
{
    chpClient->stop();
}

void Tap::log(int data)
{
    std::async(std::launch::async, &Tap::publish, this, data);
}

void Tap::publish(int data)
{
    std::vector<uint8_t> bytes;
    bytes.resize(4);

    for (unsigned int i = 0; i < 4; ++i)
    {
        bytes[i] = (data >> (i * 8));
    }

    zmq::message_t uuidMsg(uuid.c_str(), uuid.size());
    zmq::message_t valueMsg((void*)&data, sizeof(int));

    publisher->send(uuidMsg, ZMQ_SNDMORE);
    publisher->send(valueMsg);

    std::cout << "tap::publish data = " << data << std::endl;
}
