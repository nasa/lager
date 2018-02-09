#include "mug.h"

Mug::Mug(): running(false)
{
}

Mug::~Mug()
{
}

bool Mug::init(const std::string& serverHost_in, int basePort, const std::string& kegDir)
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

    keg.reset(new Keg(kegDir));

    return true;
}

void Mug::start()
{
    running = true;

    keg->start();

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
    keg->stop();
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
                keg->addFormat(j->first, i->second);
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
        std::vector<uint8_t> data;

        uint8_t tmp8;
        uint16_t tmp16;
        uint32_t tmp32;
        uint64_t tmp64;
        off_t offset;

        uint32_t rcvMore = 0;
        size_t moreSize = sizeof(rcvMore);

        zmq::pollitem_t items[] = {{static_cast<void*>(*subscriber.get()), 0, ZMQ_POLLIN, 0}};

        while (running)
        {
            // TODO check if this timeout should be different or setable by user
            zmq::poll(&items[0], 1, 1000);

            if (items[0].revents & ZMQ_POLLIN)
            {
                data.clear();
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

                for (size_t i = 0; i < uuid.size(); ++i)
                {
                    data.push_back(uuid[i]);
                }

                offset += UUID_SIZE_BYTES;

                data.resize(data.size() + TIMESTAMP_SIZE_BYTES);
                *(reinterpret_cast<uint64_t*>(data.data() + offset)) = timestamp;

                offset += TIMESTAMP_SIZE_BYTES;

                subscriber->getsockopt(ZMQ_RCVMORE, &rcvMore, &moreSize);

                while (rcvMore != 0)
                {
                    subscriber->recv(&msg);

                    // TODO eventually move to a size + blob architecture, if necessary,
                    // since these reallocs may be expensive
                    switch (msg.size())
                    {
                        case 1:
                            data.resize(data.size() + 1);
                            tmp8 = *(uint8_t*)msg.data();
                            data.push_back(tmp8);
                            offset += 1;
                            break;

                        case 2:
                            data.resize(data.size() + 2);
                            tmp16 = *(uint16_t*)msg.data();
                            *(reinterpret_cast<uint16_t*>(data.data() + offset)) = tmp16;
                            offset += 2;
                            break;

                        case 4:
                            data.resize(data.size() + 4);
                            tmp32 = *(uint32_t*)msg.data();
                            *(reinterpret_cast<uint32_t*>(data.data() + offset)) = tmp32;
                            offset += 4;
                            break;

                        case 8:
                            data.resize(data.size() + 8);
                            tmp64 = *(uint64_t*)msg.data();
                            *(reinterpret_cast<uint64_t*>(data.data() + offset)) = tmp64;
                            offset += 8;
                            break;

                        default:
                            throw std::runtime_error("received unsupported zmq message size");
                            break;
                    }

                    keg->write(data, data.size());
                    subscriber->getsockopt(ZMQ_RCVMORE, &rcvMore, &moreSize);
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
