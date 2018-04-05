#include "lager/forwarder.h"

/**
 * @brief Constructor for forwarder
 * @param basePort is the base port which Lager uses to calculate all used ports in the system
 */
Forwarder::Forwarder(int basePort): running(false)
{
    frontendPort = basePort + FORWARDER_FRONTEND_OFFSET;
    backendPort = basePort + FORWARDER_BACKEND_OFFSET;
}

/**
 * @brief Initializes the forwarder with the given zmq context
 * @param context_in is a shared pointer to an initialized zmq context
 */
void Forwarder::init(std::shared_ptr<zmq::context_t> context_in)
{
    context = context_in;
}

/**
 * @brief Starts the main forwarder thread
 */
void Forwarder::start()
{
    running = true;

    forwarderThreadHandle = std::thread(&Forwarder::forwarderThread, this);
    forwarderThreadHandle.detach();
}

/**
 * @brief Stops the main forwarder thread
 */
void Forwarder::stop()
{
    std::unique_lock<std::mutex> lock(mutex);

    while (running)
    {
        cv.wait(lock);
    }
}

/**
 * @brief Main forwarder thread
 */
void Forwarder::forwarderThread()
{
    frontend.reset(new zmq::socket_t(*context.get(), ZMQ_SUB));
    frontend->bind(lager_utils::getLocalUri(frontendPort).c_str());

    // Make sure the frontend subscribes to everything
    frontend->setsockopt(ZMQ_SUBSCRIBE, "", 0);

    backend.reset(new zmq::socket_t(*context.get(), ZMQ_PUB));
    backend->bind(lager_utils::getLocalUri(backendPort).c_str());

    // C call which sets up the forwarder and blocks until the zmq context is closed
    zmq_device(ZMQ_FORWARDER, (void*)*frontend.get(), (void*)*backend.get());

    frontend->close();
    backend->close();
    cv.notify_one();
    running = false;
}
