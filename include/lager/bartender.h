#ifndef BARTENDER
#define BARTENDER

#include <future>
#include <memory>

#include "chp_server.h"
#include "forwarder.h"

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

protected:
    std::shared_ptr<ClusteredHashmapServer> registrar;
    std::shared_ptr<Forwarder> forwarder;
    std::shared_ptr<zmq::context_t> context;
};

#endif
