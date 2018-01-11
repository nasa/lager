#ifndef BARTENDER
#define BARTENDER

#include <memory>

#include "chp_server.h"
#include "forwarder.h"

class Bartender
{
public:
    Bartender();
    ~Bartender();

    void init(int basePort);
    void start();
    void stop();

private:
    std::shared_ptr<ChpServer> registrar;
    std::shared_ptr<Forwarder> forwarder;
    std::shared_ptr<zmq::context_t> context;
};

#endif
