#ifndef FORWARDER
#define FORWARDER

#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>

#include <zmq.hpp>

#include "lager_defines.h"
#include "lager_utils.h"

class Forwarder final
{
public:
    Forwarder(int basePort);

    void init(std::shared_ptr<zmq::context_t> context_in);
    void start();
    void stop();

private:
    void forwarderThread();

    std::shared_ptr<zmq::context_t> context;
    std::shared_ptr<zmq::socket_t> frontend;
    std::shared_ptr<zmq::socket_t> backend;

    std::thread forwarderThreadHandle;
    std::mutex mutex;
    std::condition_variable cv;

    int frontendPort;
    int backendPort;

    bool running;
};

#endif
