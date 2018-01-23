#include "tap.h"

Tap::Tap(): publisherPort(0), running(false), newData(false), useCompression(0)
{
}

void Tap::init(const std::string& serverHost_in, int basePort)
{
    context.reset(new zmq::context_t(1));

    publisherPort = basePort + FORWARDER_FRONTEND_OFFSET;
    uuid = lager_utils::getUuid();
    serverHost = serverHost_in;

    // 2000 default timeout for testing
    chpClient.reset(new ChpClient(serverHost_in, basePort, 2000));
    chpClient->init(context, uuid);
}

// defaults to use input file path, may re-think this later
void Tap::start(const std::string& key_in, const std::string& formatStr_in, bool isFile)
{
    DataFormatParser p;

    if (isFile)
    {
        format = p.parseFromFile(formatStr_in);
        formatStr = p.getXmlStr();
    }
    else
    {
        format = p.parseFromString(formatStr_in);
        formatStr = formatStr_in;
    }

    version = format->getVersion();
    key = key_in;

    payloadSize = format->getPayloadSize();
    payload.resize(payloadSize);

    running = true;

    chpClient->addOrUpdateKeyValue(key, formatStr);
    chpClient->start();

    publisherThreadHandle = std::thread(&Tap::publisherThread, this);
    publisherThreadHandle.detach();
}

void Tap::updateData()
{
    mutex.lock();
    timestamp = lager_utils::getCurrentTime();
    newData = true;
    mutex.unlock();
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
    updateData();
}

void Tap::publisherThread()
{
    publisherRunning = true;
    zmq::socket_t publisher(*context.get(), ZMQ_PUB);

    int linger = 0;
    publisher.setsockopt(ZMQ_LINGER, &linger, sizeof(linger));
    publisher.connect(lager_utils::getRemoteUri(serverHost, publisherPort).c_str());

    lager_utils::sleep(1000);

    while (running)
    {
        if (newData)
        {
            zmq::message_t uuidMsg(uuid.size());
            zmq::message_t versionMsg(version.size());
            zmq::message_t compressionMsg(sizeof(useCompression));
            zmq::message_t timestampMsg(sizeof(timestamp));
            zmq::message_t payloadMsg(payload.size());

            mutex.lock();
            memcpy(uuidMsg.data(), uuid.c_str(), uuid.size());
            memcpy(versionMsg.data(), version.c_str(), version.size());
            memcpy(compressionMsg.data(), (void*)&useCompression, sizeof(useCompression));
            memcpy(timestampMsg.data(), (void*)&timestamp, sizeof(timestamp));
            memcpy(payloadMsg.data(), (void*)&payload, payload.size());
            newData = false;
            mutex.unlock();

            publisher.send(uuidMsg, ZMQ_SNDMORE);
            publisher.send(versionMsg, ZMQ_SNDMORE);
            publisher.send(compressionMsg, ZMQ_SNDMORE);
            publisher.send(timestampMsg, ZMQ_SNDMORE);
            publisher.send(payloadMsg);
        }
    }

    publisher.close();
    publisherRunning = false;
    publisherCv.notify_one();
}
