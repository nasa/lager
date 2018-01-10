#ifndef BARTENDER
#define BARTENDER

#include <memory>

#include "chp_server.h"

class Bartender
{
public:
    Bartender();

    void init(int basePort);
    void start();
    void stop();

private:
    std::shared_ptr<ChpServer> registrar;
    std::shared_ptr<zmq::context_t> zContext;
};

#endif
