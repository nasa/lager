#include "lager/bartender.h"

Bartender::Bartender()
{
}

/**
 * @brief Starts the zmq context and initializes the underlying CHP and XPUB sockets
 * @param basePort is the base port used for all Lager communication
 * @return true on success, false on failure
 */
bool Bartender::init(int basePort)
{
    // make sure basePort is a valid port
    if (basePort < 0 || basePort > BASEPORT_MAX)
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

/**
 * @brief Starts the sockets listening
 */
void Bartender::start()
{
    registrar->start();
    forwarder->start();
}

/**
 * @brief Closes the zmq context and stops the sockets
 */
void Bartender::stop()
{
    zmq_ctx_shutdown((void*)*context.get());

    registrar->stop();
    forwarder->stop();

    context->close();
}
