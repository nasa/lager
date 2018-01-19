#include "chp_server.h"

ChpServer::ChpServer(int basePort): initialized(false), running(false), sequence(0)
{
    snapshotPort = basePort;
    publisherPort = basePort + 1;
    collectorPort = basePort + 2;

    updatedKeys.clear();
}

ChpServer::~ChpServer()
{
}

void ChpServer::init(std::shared_ptr<zmq::context_t> context_in)
{
    context = context_in;

    initialized = true;
}

void ChpServer::start()
{
    if (!initialized)
    {
        throw std::runtime_error("ChpServer started before initialized");
    }

    running = true;

    publisherThreadHandle = std::thread(&ChpServer::publisherThread, this);
    snapshotThreadHandle = std::thread(&ChpServer::snapshotThread, this);
    collectorThreadHandle = std::thread(&ChpServer::collectorThread, this);

    publisherThreadHandle.detach();
    snapshotThreadHandle.detach();
    collectorThreadHandle.detach();
}

void ChpServer::stop()
{
    std::unique_lock<std::mutex> lock(mutex);

    running = false;

    while (publisherRunning)
    {
        publisherCv.wait(lock);
    }

    while (snapshotRunning)
    {
        snapshotCv.wait(lock);
    }

    while (collectorRunning)
    {
        collectorCv.wait(lock);
    }
}

void ChpServer::addOrUpdateKeyValue(const std::string& key, const std::string& value)
{
    hashMap[key] = value;
    updatedKeys.push_back(key);
}

void ChpServer::removeKey(const std::string& key)
{
    hashMap[key] = "";
    updatedKeys.push_back(key);
}

void ChpServer::snapshotThread()
{
    snapshotRunning = true;

    try
    {
        snapshot.reset(new zmq::socket_t(*context.get(), ZMQ_ROUTER));
        snapshot->bind(lager_utils::getLocalUri(snapshotPort).c_str());

        zmq::pollitem_t items[] = {{static_cast<void*>(*snapshot.get()), 0, ZMQ_POLLIN, 0}};

        std::string identity("");
        std::string icanhaz("");
        std::string subtree("");

        while (running)
        {
            zmq::poll(&items[0], 1, 1000);

            if (items[0].revents & ZMQ_POLLIN)
            {
                zmq::message_t msg;

                snapshot->recv(&msg);
                identity = std::string(static_cast<char*>(msg.data()), msg.size());
                snapshot->recv(&msg);
                icanhaz = std::string(static_cast<char*>(msg.data()), msg.size());
                snapshot->recv(&msg);
                subtree = std::string(static_cast<char*>(msg.data()), msg.size());

                if (icanhaz == "ICANHAZ?")
                {
                    snapshotMap(identity);
                    snapshotBye(identity);
                }
            }
        }
    }
    catch (zmq::error_t e)
    {
        if (e.num() == ETERM)
        {
            snapshot->close();
            snapshotRunning = false;
            snapshotCv.notify_one();
        }
    }
}

void ChpServer::snapshotMap(const std::string& identity)
{
    std::string empty("");

    for (std::map<std::string, std::string>::iterator i = hashMap.begin(); i != hashMap.end(); ++i)
    {
        zmq::message_t identityMsg(identity.size());
        zmq::message_t frame0(i->first.size());
        zmq::message_t frame1(sizeof(double));
        zmq::message_t frame2(empty.size());
        zmq::message_t frame3(empty.size());
        zmq::message_t frame4(i->second.size());

        memcpy(identityMsg.data(), identity.c_str(), identity.size());
        memcpy(frame0.data(), i->first.c_str(), i->first.size());
        memcpy(frame1.data(), (void*)&sequence, sizeof(double));
        memcpy(frame2.data(), empty.c_str(), empty.size());
        memcpy(frame3.data(), empty.c_str(), empty.size());
        memcpy(frame4.data(), i->second.c_str(), i->second.size());

        snapshot->send(identityMsg, ZMQ_SNDMORE);
        snapshot->send(frame0, ZMQ_SNDMORE);
        snapshot->send(frame1, ZMQ_SNDMORE);
        snapshot->send(frame2, ZMQ_SNDMORE);
        snapshot->send(frame3, ZMQ_SNDMORE);
        snapshot->send(frame4);
    }
}

void ChpServer::snapshotBye(const std::string& identity)
{
    std::string kthxbai("KTHXBAI");
    std::string empty("");
    std::string subtree("");

    zmq::message_t identityMsg(identity.size());
    zmq::message_t frame0(kthxbai.size());
    zmq::message_t frame1(sizeof(double));
    zmq::message_t frame2(empty.size());
    zmq::message_t frame3(empty.size());
    zmq::message_t frame4(subtree.size());

    memcpy(identityMsg.data(), identity.c_str(), identity.size());
    memcpy(frame0.data(), kthxbai.c_str(), kthxbai.size());
    memcpy(frame1.data(), (void*)&sequence, sizeof(double));
    memcpy(frame2.data(), empty.c_str(), empty.size());
    memcpy(frame3.data(), empty.c_str(), empty.size());
    memcpy(frame4.data(), subtree.c_str(), subtree.size());

    snapshot->send(identityMsg, ZMQ_SNDMORE);
    snapshot->send(frame0, ZMQ_SNDMORE);
    snapshot->send(frame1, ZMQ_SNDMORE);
    snapshot->send(frame2, ZMQ_SNDMORE);
    snapshot->send(frame3, ZMQ_SNDMORE);
    snapshot->send(frame4);

    sequence++;
}

void ChpServer::publishUpdatedKeys()
{
    std::string empty("");
    std::vector<std::string> removedKeys;

    for (std::vector<std::string>::iterator i = updatedKeys.begin(); i != updatedKeys.end(); ++i)
    {
        zmq::message_t frame0((*i).size());
        zmq::message_t frame1(sizeof(double));
        zmq::message_t frame2(empty.size());
        zmq::message_t frame3(empty.size());
        zmq::message_t frame4(hashMap[*i].size());

        memcpy(frame0.data(), (*i).c_str(), (*i).size());
        memcpy(frame1.data(), (void*)&sequence, sizeof(double));
        memcpy(frame2.data(), empty.c_str(), empty.size());
        memcpy(frame3.data(), empty.c_str(), empty.size());
        memcpy(frame4.data(), hashMap[*i].c_str(), hashMap[*i].size());

        publisher->send(frame0, ZMQ_SNDMORE);
        publisher->send(frame1, ZMQ_SNDMORE);
        publisher->send(frame2, ZMQ_SNDMORE);
        publisher->send(frame3, ZMQ_SNDMORE);
        publisher->send(frame4);

        if (hashMap[*i].size() == 0)
        {
            removedKeys.push_back(*i);
        }

        sequence++;
    }

    for (std::vector<std::string>::iterator i = removedKeys.begin(); i != removedKeys.end(); ++i)
    {
        hashMap.erase(*i);
    }

    updatedKeys.clear();
}

void ChpServer::publisherThread()
{
    publisherRunning = true;

    try
    {
        publisher.reset(new zmq::socket_t(*context.get(), ZMQ_PUB));
        publisher->bind(lager_utils::getLocalUri(publisherPort).c_str());

        while (running)
        {
            if (updatedKeys.size() > 0)
            {
                publishUpdatedKeys();
            }
            else
            {
                publishHugz();
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
    catch (zmq::error_t e)
    {
        if (e.num() == ETERM)
        {
            publisher->close();
            publisherRunning = false;
            publisherCv.notify_one();
        }
    }
}

void ChpServer::publishHugz()
{
    std::string hugz("HUGZ");
    std::string empty("");
    double zero = 0;

    zmq::message_t frame0(hugz.size());
    zmq::message_t frame1(sizeof(double));
    zmq::message_t frame2(empty.size());
    zmq::message_t frame3(empty.size());
    zmq::message_t frame4(empty.size());

    memcpy(frame0.data(), hugz.c_str(), hugz.size());
    memcpy(frame1.data(), (void*)&zero, sizeof(double));
    memcpy(frame2.data(), empty.c_str(), empty.size());
    memcpy(frame3.data(), empty.c_str(), empty.size());
    memcpy(frame4.data(), empty.c_str(), empty.size());

    publisher->send(frame0, ZMQ_SNDMORE);
    publisher->send(frame1, ZMQ_SNDMORE);
    publisher->send(frame2, ZMQ_SNDMORE);
    publisher->send(frame3, ZMQ_SNDMORE);
    publisher->send(frame4);
}

void ChpServer::collectorThread()
{
    collectorRunning = true;

    try
    {
        collector.reset(new zmq::socket_t(*context.get(), ZMQ_SUB));
        collector->bind(lager_utils::getLocalUri(collectorPort).c_str());
        collector->setsockopt(ZMQ_SUBSCRIBE, "", 0);

        zmq::pollitem_t items[] = {{static_cast<void*>(*collector.get()), 0, ZMQ_POLLIN, 0}};

        std::string key("");
        std::string uuid("");
        std::string properties("");
        std::string value("");
        double sequence = 0;
        bool duplicateKey = false;

        while (running)
        {
            zmq::poll(&items[0], 1, 1000);

            if (items[0].revents & ZMQ_POLLIN)
            {
                zmq::message_t msg;

                collector->recv(&msg);
                key = std::string(static_cast<char*>(msg.data()), msg.size());

                collector->recv(&msg);
                sequence = *(double*)msg.data();

                collector->recv(&msg);
                uuid = std::string(static_cast<char*>(msg.data()), msg.size());

                collector->recv(&msg);
                properties = std::string(static_cast<char*>(msg.data()), msg.size());

                collector->recv(&msg);
                value = std::string(static_cast<char*>(msg.data()), msg.size());

                if (uuid.length() > 0)
                {
                    if (uuidMap.find(uuid) == uuidMap.end())
                    {
                        duplicateKey = false;

                        for (auto i = uuidMap.begin(); i != uuidMap.end(); ++i)
                        {
                            // Duplicate key exists in the map under another uuid
                            if (i->second == key)
                            {
                                duplicateKey = true;
                                std::cout << "key already registered by another uuid. TBD action" << std::endl;
                            }
                        }

                        if (!duplicateKey)
                        {
                            uuidMap[uuid] = key;
                        }
                    }
                }

                if (value.length() == 0)
                {
                    removeKey(key);
                }
                else
                {
                    addOrUpdateKeyValue(key, value);
                }
            }
        }
    }
    catch (zmq::error_t e)
    {
        if (e.num() == ETERM)
        {
            collector->close();
            collectorRunning = false;
            collectorCv.notify_one();
        }
    }
}
