#include "tap.h"

Tap::Tap(): publisherPort(0), running(false), newData(false)
{
}

void Tap::init(const std::string& serverHost_in, int basePort)
{
    context.reset(new zmq::context_t(1));

    publisherPort = basePort + FORWARDER_FRONTEND_OFFSET;
    uuid = lager_utils::getUuid();
    serverHost = serverHost_in;

    // 2000 default timeout for testing
    chpClient.reset(new ChpClient(serverHost_in, basePort, 2000));
    chpClient->init(context, uuid);
}

// defaults to use input file path, may re-think this later
void Tap::start(const std::string& dataKey_in, const std::string& dataFormatStr_in, bool isFile)
{
    DataFormatParser p;

    if (isFile)
    {
        dataFormat = p.parseFromFile(dataFormatStr_in);
        dataFormatStr = p.getXmlStr();
    }
    else
    {
        dataFormat = p.parseFromString(dataFormatStr_in);
        dataFormatStr = dataFormatStr_in;
    }

    dataKey = dataKey_in;

    running = true;

    chpClient->addOrUpdateKeyValue(dataKey, dataFormatStr);
    chpClient->start();

    publisherThreadHandle = std::thread(&Tap::publisherThread, this);
    publisherThreadHandle.detach();
}

void Tap::stop()
{
    running = false;

    context->close();

    std::unique_lock<std::mutex> lock(mutex);

    while (publisherRunning)
    {
        publisherCv.wait(lock);
    }

    chpClient->stop();
}

void Tap::log(int data)
{
    mutex.lock();
    newData = true;
    theInt = data;
    mutex.unlock();
}

void Tap::publisherThread()
{
    publisherRunning = true;
    zmq::socket_t publisher(*context.get(), ZMQ_PUB);

    int linger = 0;
    publisher.setsockopt(ZMQ_LINGER, &linger, sizeof(linger));
    publisher.connect(lager_utils::getRemoteUri(serverHost, publisherPort).c_str());

    lager_utils::sleep(1000);

    while (running)
    {
        if (newData)
        {
            zmq::message_t uuidMsg(uuid.size());
            zmq::message_t valueMsg(sizeof(int));

            memcpy(uuidMsg.data(), uuid.c_str(), uuid.size());
            mutex.lock();
            memcpy(valueMsg.data(), (void*)&theInt, sizeof(int));
            newData = false;
            mutex.unlock();

            publisher.send(uuidMsg, ZMQ_SNDMORE);
            publisher.send(valueMsg);

            std::cout << "tap::publish data = " << theInt << std::endl;
        }
    }

    publisher.close();
    publisherRunning = false;
    publisherCv.notify_one();
}
