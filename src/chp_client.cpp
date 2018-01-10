#include "chp_client.h"

ChpClient::ChpClient(const std::string& serverHost_in, int basePort, int timeoutMillis_in):
    running(false), sequence(-1), uuid("invalid")
{
    snapshotPort = basePort;
    subscriberPort = basePort + 1;
    publisherPort = basePort + 2;
    timeoutMillis = timeoutMillis_in;
    serverHost = serverHost_in;
}

ChpClient::~ChpClient()
{
    zmq_term((void*)&context);
}

void ChpClient::init(std::shared_ptr<zmq::context_t> context_in, const std::string& uuid_in)
{
    context = context_in;
    uuid = uuid_in;
}

void ChpClient::start()
{
    running = true;

    snapshotThreadHandle = std::thread(&ChpClient::snapshotThread, this);
    subscriberThreadHandle = std::thread(&ChpClient::subscriberThread, this);
}

void ChpClient::stop()
{
    running = false;

    if (subscriberThreadHandle.joinable())
    {
        subscriberThreadHandle.join();
    }

    if (snapshotThreadHandle.joinable())
    {
        snapshotThreadHandle.join();
    }

    if (publisherThreadHandle.joinable())
    {
        publisherThreadHandle.join();
    }
}

void ChpClient::addOrUpdateKeyValue(const std::string& key, const std::string& value)
{
    publisherThreadHandle = std::thread(&ChpClient::publisherThread, this, key, value);
}

void ChpClient::removeKey(const std::string& key)
{
    publisherThreadHandle = std::thread(&ChpClient::publisherThread, this, key, "");
}

void ChpClient::subscriberThread()
{
    subscriber.reset(new zmq::socket_t(*context.get(), ZMQ_SUB));
    subscriber->connect(lager_utils::getRemoteUri(serverHost.c_str(), subscriberPort).c_str());
    subscriber->setsockopt(ZMQ_SUBSCRIBE, "", 0);

    std::string key("");
    std::string empty("");
    std::string value("");
    double sequence = 0;

    zmq::pollitem_t items[] = {{static_cast<void*>(*subscriber.get()), 0, ZMQ_POLLIN, 0}};

    while (running)
    {
        zmq::poll(&items[0], 1, timeoutMillis);

        if (items[0].revents & ZMQ_POLLIN)
        {
            zmq::message_t msg;

            subscriber->recv(&msg);
            key = std::string(static_cast<char*>(msg.data()), msg.size());

            subscriber->recv(&msg);
            sequence = *(double*)msg.data();

            subscriber->recv(&msg);
            empty = std::string(static_cast<char*>(msg.data()), msg.size());

            subscriber->recv(&msg);
            empty = std::string(static_cast<char*>(msg.data()), msg.size());

            subscriber->recv(&msg);
            value = std::string(static_cast<char*>(msg.data()), msg.size());

            if (key == "HUGZ")
            {
                std::cout << "hugz received" << std::endl;
            }
            else
            {
                if (sequence > this->sequence)
                {
                    if (value.length() == 0)
                    {
                        hashMap.erase(key);
                        std::cout << "removed key " << key << " from hashMap" << std::endl;
                    }
                    else
                    {
                        hashMap[key] = value;
                        std::cout << "updated hashMap with (" << key << ", " << value << ")" << std::endl;
                    }
                }
                else
                {
                    std::cout << "bad sequence check (new = " << sequence << ", old = " << this->sequence << ")" << std::endl;
                }

                lager_utils::printHashMap(hashMap);
            }

            this->sequence = sequence;
        }
        else
        {
            std::cout << "no hugz in " << timeoutMillis << "ms, TBD action." << std::endl;
        }
    }
}

void ChpClient::snapshotThread()
{
    int hwm = 1;
    snapshot.reset(new zmq::socket_t(*context.get(), ZMQ_DEALER));
    snapshot->setsockopt(ZMQ_SNDHWM, &hwm, sizeof(hwm));
    snapshot->connect(lager_utils::getRemoteUri(serverHost.c_str(), snapshotPort).c_str());

    std::map<std::string, std::string> updateMap;
    std::string icanhaz("ICANHAZ?");
    std::string subtree("");
    std::string value("");
    std::string key("");
    std::string empty("");
    double sequence = 0;

    while (key != "KTHXBAI")
    {
        zmq::message_t reqFrame0(icanhaz.size());
        zmq::message_t reqFrame1(subtree.size());

        memcpy(reqFrame0.data(), icanhaz.c_str(), icanhaz.size());
        memcpy(reqFrame1.data(), subtree.c_str(), subtree.size());

        snapshot->send(reqFrame0, ZMQ_SNDMORE);
        snapshot->send(reqFrame1);

        zmq::pollitem_t items[] = {{static_cast<void*>(*snapshot.get()), 0, ZMQ_POLLIN, 0}};

        zmq::poll(items, 1, timeoutMillis);

        if (items[0].revents & ZMQ_POLLIN)
        {
            zmq::message_t replyMsg;

            snapshot->recv(&replyMsg);
            key = std::string(static_cast<char*>(replyMsg.data()), replyMsg.size());

            snapshot->recv(&replyMsg);
            sequence = *(double*)replyMsg.data();

            snapshot->recv(&replyMsg);
            empty = std::string(static_cast<char*>(replyMsg.data()), replyMsg.size());

            snapshot->recv(&replyMsg);
            empty = std::string(static_cast<char*>(replyMsg.data()), replyMsg.size());

            snapshot->recv(&replyMsg);
            value = std::string(static_cast<char*>(replyMsg.data()), replyMsg.size());

            if (sequence > this->sequence)
            {
                if (value.length() > 0)
                {
                    updateMap[key] = value;
                }
                else if (key != "KTHXBAI")
                {
                    // this shouldn't happen
                    std::cout << "got initial snapshot (key, value) with empty value" << std::endl;
                }
            }
            else
            {
                std::cout << "bad sequence check (new = " << sequence << ", old = " << this->sequence << ")" << std::endl;
            }
        }
        else
        {
            // only process a complete set
            updateMap.clear();
            std::cout << "no reply from snapshot server in " << timeoutMillis << "ms, retrying infinite times" << std::endl;
        }
    }

    for (auto i = updateMap.begin(); i != updateMap.end(); ++i)
    {
        hashMap[i->first] = i->second;
    }

    this->sequence = sequence;
    std::cout << "received hashMap" << std::endl;
    lager_utils::printHashMap(hashMap);
}

void ChpClient::publisherThread(const std::string& key, const std::string& value)
{
    int hwm = 1;
    publisher.reset(new zmq::socket_t(*context.get(), ZMQ_PUB));
    publisher->setsockopt(ZMQ_SNDHWM, &hwm, sizeof(hwm));
    publisher->connect(lager_utils::getRemoteUri(serverHost.c_str(), publisherPort).c_str());

    lager_utils::sleep(1000);

    // @TODO implement properties
    std::string properties("");
    double zero = 0;

    zmq::message_t frame0(key.size());
    zmq::message_t frame1(sizeof(double));
    zmq::message_t frame2(uuid.size());
    zmq::message_t frame3(properties.size());
    zmq::message_t frame4(value.size());

    memcpy(frame0.data(), key.c_str(), key.size());
    memcpy(frame1.data(), (void*)&zero, sizeof(double));
    memcpy(frame2.data(), uuid.c_str(), uuid.size());
    memcpy(frame3.data(), properties.c_str(), properties.size());
    memcpy(frame4.data(), value.c_str(), value.size());

    publisher->send(frame0, ZMQ_SNDMORE);
    publisher->send(frame1, ZMQ_SNDMORE);
    publisher->send(frame2, ZMQ_SNDMORE);
    publisher->send(frame3, ZMQ_SNDMORE);
    publisher->send(frame4);

    std::cout << "sent updated key " << key << std::endl;
}
