#include "chp_client.h"

ClusteredHashmapClient::ClusteredHashmapClient(const std::string& serverHost_in, int basePort, int timeoutMillis_in):
    initialized(false), running(false), timedOut(false), sequence(-1), uuid("invalid")
{
    snapshotPort = basePort + CHP_SNAPSHOT_OFFSET;
    subscriberPort = basePort + CHP_PUBLISHER_OFFSET;
    publisherPort = basePort + CHP_COLLECTOR_OFFSET;
    timeoutMillis = timeoutMillis_in;
    serverHost = serverHost_in;
}

void ClusteredHashmapClient::init(std::shared_ptr<zmq::context_t> context_in, const std::string& uuid_in)
{
    context = context_in;
    uuid = uuid_in;

    initialized = true;
}

void ClusteredHashmapClient::start()
{
    if (!initialized)
    {
        throw std::runtime_error("ClusteredHashmapClient started before initialized");
    }

    running = true;

    snapshotThreadHandle = std::thread(&ClusteredHashmapClient::snapshotThread, this);
    subscriberThreadHandle = std::thread(&ClusteredHashmapClient::subscriberThread, this);

    snapshotThreadHandle.detach();
    subscriberThreadHandle.detach();
}

void ClusteredHashmapClient::stop()
{
    std::unique_lock<std::mutex> lock(mutex);

    running = false;

    while (subscriberRunning)
    {
        subscriberCv.wait(lock);
    }

    while (snapshotRunning)
    {
        snapshotCv.wait(lock);
    }
}

void ClusteredHashmapClient::addOrUpdateKeyValue(const std::string& key, const std::string& value)
{
    std::async(std::launch::async, &ClusteredHashmapClient::publisherThread, this, key, value);
}

void ClusteredHashmapClient::removeKey(const std::string& key)
{
    std::async(std::launch::async, &ClusteredHashmapClient::publisherThread, this, key, "");
}

void ClusteredHashmapClient::setCallback(const std::function<void()>& func)
{
    hashMapUpdated = func;
}

void ClusteredHashmapClient::subscriberThread()
{
    timedOut = false;
    subscriberRunning = true;
    subscriber.reset(new zmq::socket_t(*context.get(), ZMQ_SUB));

    int linger = 0;
    subscriber->setsockopt(ZMQ_SUBSCRIBE, "", 0);
    subscriber->setsockopt(ZMQ_LINGER, &linger, sizeof(int));
    subscriber->connect(lager_utils::getRemoteUri(serverHost.c_str(), subscriberPort).c_str());

    std::string key("");
    std::string empty("");
    std::string value("");
    double sequence = 0;

    zmq::pollitem_t items[] = {{static_cast<void*>(*subscriber.get()), 0, ZMQ_POLLIN, 0}};

    try
    {
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

                if (key != "HUGZ")
                {
                    // TODO what should be the action here if sequence count fails?
                    if (sequence > this->sequence)
                    {
                        if (value.length() == 0)
                        {
                            hashMap.erase(key);
                        }
                        else
                        {
                            hashMap[key] = value;

                            if (hashMapUpdated)
                            {
                                hashMapUpdated();
                            }
                        }
                    }
                }

                this->sequence = sequence;
            }
            else
            {
                timedOut = true;
            }
        }
    }
    catch (zmq::error_t& e)
    {
        if (e.num() == ETERM)
        {
            subscriber->close();
            subscriberRunning = false;
            subscriberCv.notify_one();
        }
    }
}

void ClusteredHashmapClient::snapshotThread()
{
    snapshotRunning = true;
    int hwm = 1;
    int linger = 0;
    snapshot.reset(new zmq::socket_t(*context.get(), ZMQ_DEALER));
    snapshot->setsockopt(ZMQ_SNDHWM, &hwm, sizeof(int));
    snapshot->setsockopt(ZMQ_LINGER, &linger, sizeof(int));
    snapshot->connect(lager_utils::getRemoteUri(serverHost.c_str(), snapshotPort).c_str());

    std::map<std::string, std::string> updateMap;
    std::string icanhaz("ICANHAZ?");
    std::string subtree("");
    std::string value("");
    std::string key("");
    std::string empty("");
    double sequence = 0;

    try
    {
        while (running)
        {
            if (key != "KTHXBAI")
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
                    }
                    else
                    {
                        // TODO throw error
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
            else
            {
                break;
            }
        }
    }
    catch (zmq::error_t& e)
    {
        if (e.num() == ETERM)
        {
            snapshot->close();
            snapshotRunning = false;
            snapshotCv.notify_one();
        }
    }

    for (auto i = updateMap.begin(); i != updateMap.end(); ++i)
    {
        hashMap[i->first] = i->second;
    }

    if (updateMap.size() > 0)
    {
        this->sequence = sequence;
    }

    snapshot->close();
    snapshotRunning = false;
    snapshotCv.notify_one();
}

void ClusteredHashmapClient::publisherThread(const std::string& key, const std::string& value)
{
    int hwm = 1;
    int linger = 0;
    publisher.reset(new zmq::socket_t(*context.get(), ZMQ_PUB));
    publisher->setsockopt(ZMQ_SNDHWM, &hwm, sizeof(int));
    publisher->setsockopt(ZMQ_LINGER, &linger, sizeof(int));
    publisher->connect(lager_utils::getRemoteUri(serverHost.c_str(), publisherPort).c_str());

    lager_utils::sleepMillis(1000);

    // TODO implement properties
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

    publisher->close();
}
