#include "forwarder.h"

Forwarder::Forwarder(int basePort): running(false)
{
    frontendPort = basePort;
    backendPort = basePort + 1;
}

void Forwarder::init(std::shared_ptr<zmq::context_t> context_in)
{
    context = context_in;
}

void Forwarder::start()
{
    running = true;

    forwarderThreadHandle = std::thread(&Forwarder::forwarderThread, this);
}

void Forwarder::stop()
{
    running = false;

    if (forwarderThreadHandle.joinable())
    {
        forwarderThreadHandle.join();
    }
}

void Forwarder::forwarderThread()
{
    try
    {
        frontend.reset(new zmq::socket_t(*context.get(), ZMQ_SUB));
        frontend->bind(lager_utils::getLocalUri(frontendPort).c_str());
        std::cout << "forwarder frontend: " << lager_utils::getLocalUri(frontendPort) << std::endl;
        frontend->setsockopt(ZMQ_SUBSCRIBE, "", 0);

        backend.reset(new zmq::socket_t(*context.get(), ZMQ_PUB));
        backend->bind(lager_utils::getLocalUri(backendPort).c_str());
        std::cout << "forwarder backend: " << lager_utils::getLocalUri(backendPort) << std::endl;

        zmq_device(ZMQ_FORWARDER, (void*)*frontend.get(), (void*)*backend.get());
    }
    catch (zmq::error_t e)
    {
        std::cout << "socket failed: " << e.what() << std::endl;
        return;
    }

    running = false;
}
