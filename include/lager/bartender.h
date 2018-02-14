#ifndef BARTENDER
#define BARTENDER

#include <future>
#include <memory>

#include "lager/chp_server.h"
#include "lager/forwarder.h"

/**
 * @brief Bartender provides a middleware layer for the Lager system
 */
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
