#ifndef TAP
#define TAP

#include <memory>

#include "chp_client.h"

class Tap
{
public:
    Tap();

    void init(const std::string& serverHost_in, int basePort, int timeoutMillis_in);
    void start();
    void stop();

private:
    std::shared_ptr<ChpClient> chpClient;
    std::shared_ptr<zmq::context_t> zContext;
};

#endif
