#include "bartender.h"

Bartender::Bartender()
{
}

void Bartender::init(int basePort)
{
    zContext.reset(new zmq::context_t(1));

    registrar.reset(new ChpServer(basePort));
    registrar->init(zContext);
}

void Bartender::start()
{
    registrar->start();
}

void Bartender::stop()
{
    registrar->stop();
}
