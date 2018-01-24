#ifndef BARTENDER
#define BARTENDER

#include <future>
#include <memory>

#include "chp_server.h"
#include "forwarder.h"

class Bartender final
{
public:
    Bartender();

    bool init(int basePort);
    void start();
    void stop();

private:
    std::shared_ptr<ClusteredHashmapServer> registrar;
    std::shared_ptr<Forwarder> forwarder;
    std::shared_ptr<zmq::context_t> context;
};

#endif
