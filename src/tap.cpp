#include "tap.h"

Tap::Tap(): publisherPort(0), running(false), newData(false)
{
}

void Tap::init(const std::string& serverHost_in, int basePort)
{
    context.reset(new zmq::context_t(1));

    publisherPort = basePort + 10;
    uuid = lager_utils::getUuid();
    serverHost = serverHost_in;

    // 2000 default timeout for testing
    chpClient.reset(new ChpClient(serverHost_in, basePort, 2000));
    chpClient->init(context, uuid);
}

void Tap::start()
{
    running = true;

    chpClient->start();
    publisherThreadHandle = std::thread(&Tap::publisherThread, this);
    publisherThreadHandle.detach();
}

void Tap::stop()
{
    std::unique_lock<std::mutex> lock(mutex);

    // closing the context unblocks any currently blocked zmq sockets
    context->close();
    chpClient->stop();

    running = false;

    while (publisherRunning)
    {
        publisherCv.wait(lock);
    }
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

    try
    {
        int linger = 0;
        zmq::socket_t publisher(*context.get(), ZMQ_PUB);
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
    }
    catch (zmq::error_t e)
    {
        // publisher.close();
        publisherRunning = false;
        publisherCv.notify_one();
        std::cout << "publisher socket failed: " << e.what() << std::endl;
        return;
    }
}
