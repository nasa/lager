#include "mug.h"

Mug::Mug(): running(false)
{
}

Mug::~Mug()
{
}

bool Mug::init(const std::string& serverHost_in, int basePort)
{
    subscriberPort = basePort + FORWARDER_BACKEND_OFFSET;

    if (subscriberPort < 0 || subscriberPort > 65535)
    {
        // TODO provide stream output of errors?
        return false;
    }

    serverHost = serverHost_in;

    context.reset(new zmq::context_t(1));

    // TODO make this not magic 2000 default timeout for testing
    chpClient.reset(new ClusteredHashmapClient(serverHost_in, basePort, 2000));
    chpClient->init(context, uuid);
    hashMapUpdatedHandle = std::bind(&Mug::hashMapUpdated, this);
    chpClient->setCallback(hashMapUpdatedHandle);

    formatParser.reset(new DataFormatParser);

    return true;
}

void Mug::start()
{
    running = true;

    subscriberThreadHandle = std::thread(&Mug::subscriberThread, this);
    subscriberThreadHandle.detach();

    chpClient->start();
}

void Mug::stop()
{
    running = false;

    context->close();

    std::unique_lock<std::mutex> lock(mutex);

    while (subscriberRunning)
    {
        subscriberCv.wait(lock);
    }

    chpClient->stop();
}

void Mug::hashMapUpdated()
{
    std::map<std::string, std::string> tmpUuidMap;
    std::shared_ptr<DataFormat> tmpDataFormat;

    mutex.lock();
    hashMap = chpClient->getHashMap();
    tmpUuidMap = chpClient->getUuidMap();

    // TODO later only parse the subscribed topic names
    for (auto i = hashMap.begin(); i != hashMap.end(); ++i)
    {
        // parse the xml data format into a DataFormat object
        tmpDataFormat = formatParser->parseFromString(i->second);

        // look for our DataFormat object's topic name in the uuid map
        for (auto j = tmpUuidMap.begin(); j != tmpUuidMap.end(); ++j)
        {
            if (j->second == i->first)
            {
                // index the formats by uuid for ease of use as the data comes in
                formatMap[j->first] = tmpDataFormat;

                // set the data buffer size (including the timestamp)
                dataMap[j->first].resize(tmpDataFormat->getItemsSize() + TIMESTAMP_SIZE_BYTES);
                break;
            }
        }
    }

    mutex.unlock();
}

void Mug::subscriberThread()
{
    subscriberRunning = true;

    try
    {
        subscriber.reset(new zmq::socket_t(*context.get(), ZMQ_SUB));
        subscriber->connect(lager_utils::getRemoteUri(serverHost.c_str(), subscriberPort).c_str());
        subscriber->setsockopt(ZMQ_SUBSCRIBE, "", 0);

        std::string key("");
        std::string empty("");
        std::string value("");
        double sequence = 0;

        std::string uuid;
        std::string version;
        unsigned int compression;
        uint64_t timestamp;

        uint8_t tmp8;
        uint16_t tmp16;
        uint32_t tmp32;
        uint64_t tmp64;
        off_t offset;

        zmq::pollitem_t items[] = {{static_cast<void*>(*subscriber.get()), 0, ZMQ_POLLIN, 0}};

        while (running)
        {
            // TODO check if this timeout should be different or setable by user
            zmq::poll(&items[0], 1, 1000);

            if (items[0].revents & ZMQ_POLLIN)
            {
                offset = 0;

                zmq::message_t msg;

                subscriber->recv(&msg);
                std::string uuid(static_cast<char*>(msg.data()), msg.size());

                subscriber->recv(&msg);
                std::string version(static_cast<char*>(msg.data()), msg.size());

                subscriber->recv(&msg);
                compression = *(int*)msg.data();

                subscriber->recv(&msg);
                timestamp = *(uint64_t*)msg.data();

                *(reinterpret_cast<uint64_t*>(dataMap[uuid].data() + offset)) = timestamp;
                offset += TIMESTAMP_SIZE_BYTES;

                if (!formatMap[uuid])
                {
                    continue;
                }

                for (unsigned int i = 0; i != formatMap[uuid]->getItemCount(); ++i)
                {
                    subscriber->recv(&msg);

                    mutex.lock();

                    switch (msg.size())
                    {
                        case 1:
                            tmp8 = *(uint8_t*)msg.data();
                            dataMap[uuid][offset] = tmp8;
                            offset++;
                            break;

                        case 2:
                            tmp16 = ntohs(*(uint16_t*)msg.data());
                            *(reinterpret_cast<uint16_t*>(dataMap[uuid].data() + offset)) = tmp16;
                            offset += 2;
                            break;

                        case 4:
                            tmp32 = ntohl(*(uint32_t*)msg.data());
                            *(reinterpret_cast<uint32_t*>(dataMap[uuid].data() + offset)) = tmp32;
                            offset += 4;
                            break;

                        case 8:
                            tmp64 = lager_utils::ntohll(*(uint64_t*)msg.data());
                            *(reinterpret_cast<uint64_t*>(dataMap[uuid].data() + offset)) = tmp64;
                            offset += 8;
                            break;

                        default:
                            throw std::runtime_error("received unsupported zmq message size");
                            break;
                    }

                    mutex.unlock();
                }
            }
        }
    }
    catch (zmq::error_t e)
    {
        if (e.num() == ETERM)
        {
            subscriber->close();
            subscriberRunning = false;
            subscriberCv.notify_one();
        }
    }
}
