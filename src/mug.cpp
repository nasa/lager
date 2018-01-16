#include "mug.h"

Mug::Mug(): running(false)
{
}

Mug::~Mug()
{
    zmq_term((void*)*context.get());
}

void Mug::init(const std::string& serverHost_in, int basePort)
{
    // frontendPort = basePort;
    subscriberPort = basePort + 11;

    serverHost = serverHost_in;

    context.reset(new zmq::context_t(1));
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

                int theInt = 0;
                subscriber->recv(&msg);
                theInt = *(int*)msg.data();

                // std::cout << "uuid = " << uuid.c_str() << std::endl;
                std::cout << "theInt = " << theInt << std::endl;
            }
        }
    }
    catch (zmq::error_t e)
    {
        if (e.num() != ETERM)
        {
            std::cout << "subscriber socket failed: " << e.what() << std::endl;
        }
    }

    subscriber->close();
    subscriberRunning = false;
    subscriberCv.notify_one();
}
