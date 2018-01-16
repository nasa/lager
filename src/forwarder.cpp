#include "forwarder.h"

Forwarder::Forwarder(int basePort): running(false)
{
    frontendPort = basePort;
    backendPort = basePort + 1;
}

Forwarder::~Forwarder()
{
}

void Forwarder::init(std::shared_ptr<zmq::context_t> context_in)
{
    context = context_in;
}

void Forwarder::start()
{
    running = true;

    forwarderThreadHandle = std::thread(&Forwarder::forwarderThread, this);
    forwarderThreadHandle.detach();
}

void Forwarder::stop()
{
    std::unique_lock<std::mutex> lock(mutex);

    while (running)
    {
        cv.wait(lock);
    }
}

void Forwarder::forwarderThread()
{
    try
    {
        frontend.reset(new zmq::socket_t(*context.get(), ZMQ_SUB));
        frontend->bind(lager_utils::getLocalUri(frontendPort).c_str());
        frontend->setsockopt(ZMQ_SUBSCRIBE, "", 0);

        backend.reset(new zmq::socket_t(*context.get(), ZMQ_PUB));
        backend->bind(lager_utils::getLocalUri(backendPort).c_str());

        zmq_device(ZMQ_FORWARDER, (void*)*frontend.get(), (void*)*backend.get());
    }
    catch (zmq::error_t e)
    {
        if (e.num() != ETERM)
        {
            std::cout << "forwarder socket failed: " << e.what() << std::endl;
        }
    }

    frontend->close();
    backend->close();
    cv.notify_one();
    running = false;
}
