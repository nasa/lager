#include "tap.h"

Tap::Tap()
{

}

void Tap::init(const std::string& serverHost_in, int basePort, int timeoutMillis_in)
{
    zContext.reset(new zmq::context_t(1));

    chpClient.reset(new ChpClient(serverHost_in, basePort, timeoutMillis_in));
    chpClient->init(zContext);
}

void Tap::start()
{
    chpClient->start();
}

void Tap::stop()
{
    chpClient->stop();
}
