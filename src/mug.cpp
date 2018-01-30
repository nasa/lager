#include "mug.h"

Mug::Mug(): running(false)
{
}

Mug::~Mug()
{
}

bool Mug::init(const std::string& serverHost_in, int basePort)
{
    subscriberPort = basePort + FORWARDER_BACKEND_OFFSET;

    if (subscriberPort < 0 || subscriberPort > 65535)
    {
        // TODO provide stream output of errors?
        return false;
    }

    serverHost = serverHost_in;

    // testing
    payloads["test"].resize(8);

    context.reset(new zmq::context_t(1));

    return true;
}

void Mug::start()
{
    running = true;

    subscriberThreadHandle = std::thread(&Mug::subscriberThread, this);
    subscriberThreadHandle.detach();
}

void Mug::stop()
{
    running = false;

    context->close();

    std::unique_lock<std::mutex> lock(mutex);

    while (subscriberRunning)
    {
        subscriberCv.wait(lock);
    }

    // chpClient->stop();
}

void Mug::subscriberThread()
{
    subscriberRunning = true;

    try
    {
        subscriber.reset(new zmq::socket_t(*context.get(), ZMQ_SUB));
        subscriber->connect(lager_utils::getRemoteUri(serverHost.c_str(), subscriberPort).c_str());
        subscriber->setsockopt(ZMQ_SUBSCRIBE, "", 0);

        std::string key("");
        std::string empty("");
        std::string value("");
        double sequence = 0;

        std::string uuid;
        std::string version;
        unsigned int compression;
        uint64_t timestamp;
        uint32_t int1;
        int32_t int2;
        double double1;

        zmq::pollitem_t items[] = {{static_cast<void*>(*subscriber.get()), 0, ZMQ_POLLIN, 0}};

        while (running)
        {
            // 1000 ms to wait here if nothing is ready, check if this should be different or setable by user
            zmq::poll(&items[0], 1, 1000);

            if (items[0].revents & ZMQ_POLLIN)
            {
                zmq::message_t msg;

                subscriber->recv(&msg);
                std::string uuid(static_cast<char*>(msg.data()), msg.size());

                subscriber->recv(&msg);
                std::string version(static_cast<char*>(msg.data()), msg.size());

                subscriber->recv(&msg);
                compression = *(int*)msg.data();

                subscriber->recv(&msg);
                timestamp = *(uint64_t*)msg.data();

                subscriber->recv(&msg);
                uint32_t tmp = ntohl(*(uint32_t*)msg.data());
                int1 = *reinterpret_cast<uint32_t*>(&tmp);

                subscriber->recv(&msg);
                tmp = ntohl(*(uint32_t*)msg.data());
                int2 = *reinterpret_cast<int32_t*>(&tmp);

                subscriber->recv(&msg);
                uint64_t tmp64 = lager_utils::ntohll(*(uint64_t*)msg.data());
                double1 = *reinterpret_cast<double*>(&tmp64);

                std::cout << "uint1: " << int1 << " int2: " << int2 << " double1: " << double1 << std::endl;
                std::cout << "timestamp: " << timestamp << std::endl;
            }
        }
    }
    catch (zmq::error_t e)
    {
        if (e.num() == ETERM)
        {
            subscriber->close();
            subscriberRunning = false;
            subscriberCv.notify_one();
        }
    }
}
