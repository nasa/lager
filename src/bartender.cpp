#include "bartender.h"

Bartender::Bartender()
{
}

bool Bartender::init(int basePort)
{
    // make sure basePort is a valid port
    if (basePort < 0 || basePort > 65535)
    {
        // TODO have user optional stream output?
        return false;
    }

    try
    {
        context.reset(new zmq::context_t(1));

        registrar.reset(new ClusteredHashmapServer(basePort));
        registrar->init(context);

        forwarder.reset(new Forwarder(basePort));
        forwarder->init(context);
    }
    catch (...)
    {
        // TODO have user optional stream output?
        return false;
    }

    return true;
}

void Bartender::start()
{
    registrar->start();
    forwarder->start();
}

void Bartender::stop()
{
    context->close();
    registrar->stop();
    forwarder->stop();
}
