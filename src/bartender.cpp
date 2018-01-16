#include "bartender.h"

Bartender::Bartender()
{
}

Bartender::~Bartender()
{
}

void Bartender::init(int basePort)
{
    context.reset(new zmq::context_t(1));

    registrar.reset(new ChpServer(basePort));
    registrar->init(context);

    forwarder.reset(new Forwarder(basePort + 10));
    forwarder->init(context);
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
