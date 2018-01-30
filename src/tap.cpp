#include "tap.h"

Tap::Tap(): publisherPort(0), running(false), newData(false), flags(0), offsetCount(0)
{
}

Tap::~Tap()
{
}

bool Tap::init(const std::string& serverHost_in, int basePort)
{
    context.reset(new zmq::context_t(1));

    publisherPort = basePort + FORWARDER_FRONTEND_OFFSET;

    if (publisherPort < 0 || publisherPort > 65535)
    {
        // TODO provide stream output of errors?
        return false;
    }

    uuid = lager_utils::getUuid();
    serverHost = serverHost_in;

    // TODO make this not magic 2000 default timeout for testing
    chpClient.reset(new ClusteredHashmapClient(serverHost_in, basePort, 2000));
    chpClient->init(context, uuid);

    return true;
}

void Tap::addItem(AbstractDataRefItem* item)
{
    item->setOffset(offsetCount);
    dataRefItems.push_back(item);
    offsetCount += item->getSize();
}

void Tap::start(const std::string& key_in)
{
    version = "BEERR01";

    DataFormatParser p;

    if (p.createFromDataRefItems(dataRefItems, version))
    {
        formatStr = p.getXmlStr();
    }
    else
    {
        throw std::runtime_error("unable to build xml format of tap");
    }

    key = key_in;

    running = true;

    chpClient->addOrUpdateKeyValue(key, formatStr);
    chpClient->start();

    publisherThreadHandle = std::thread(&Tap::publisherThread, this);
    publisherThreadHandle.detach();
}

void Tap::stop()
{
    running = false;

    context->close();

    std::unique_lock<std::mutex> lock(mutex);

    while (publisherRunning)
    {
        publisherCv.wait(lock);
    }

    chpClient->stop();
}

void Tap::log()
{
    mutex.lock();
    timestamp = lager_utils::getCurrentTime();
    newData = true;
    mutex.unlock();
}

void Tap::publisherThread()
{
    publisherRunning = true;
    zmq::socket_t publisher(*context.get(), ZMQ_PUB);

    int linger = 0;
    publisher.setsockopt(ZMQ_LINGER, &linger, sizeof(linger));
    publisher.connect(lager_utils::getRemoteUri(serverHost, publisherPort).c_str());

    lager_utils::sleepMillis(1000);

    while (running)
    {
        if (newData)
        {
            zmq::message_t uuidMsg(uuid.size());
            zmq::message_t versionMsg(version.size());
            zmq::message_t flagsMsg(sizeof(flags));
            zmq::message_t timestampMsg(sizeof(timestamp));

            mutex.lock();

            memcpy(uuidMsg.data(), uuid.c_str(), uuid.size());
            memcpy(versionMsg.data(), version.c_str(), version.size());
            memcpy(flagsMsg.data(), (void*)&flags, sizeof(flags));
            memcpy(timestampMsg.data(), (void*)&timestamp, sizeof(timestamp));

            publisher.send(uuidMsg, ZMQ_SNDMORE);
            publisher.send(versionMsg, ZMQ_SNDMORE);
            publisher.send(flagsMsg, ZMQ_SNDMORE);
            publisher.send(timestampMsg, ZMQ_SNDMORE);

            for (unsigned int i = 0; i < dataRefItems.size(); ++i)
            {
                zmq::message_t tmp(dataRefItems[i]->getSize());
                memcpy(tmp.data(), dataRefItems[i]->getDataRef(), dataRefItems[i]->getSize());

                if (i < dataRefItems.size() - 1)
                {
                    publisher.send(tmp, ZMQ_SNDMORE);
                }
                else
                {
                    publisher.send(tmp);
                }
            }

            newData = false;
            mutex.unlock();
        }
    }

    publisher.close();
    publisherRunning = false;
    publisherCv.notify_one();
}
