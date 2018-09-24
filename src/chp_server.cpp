#include "lager/chp_server.h"

/**
 * @brief Ctor sets up ports for the three sockets used
 */
ClusteredHashmapServer::ClusteredHashmapServer(int basePort): initialized(false),
    running(false), publisherRunning(false), snapshotRunning(false), collectorRunning(false), sequence(0)
{
    snapshotPort = basePort + CHP_SNAPSHOT_OFFSET;
    publisherPort = basePort + CHP_PUBLISHER_OFFSET;
    collectorPort = basePort + CHP_COLLECTOR_OFFSET;

    updatedKeys.clear();
}

/**
 * @brief Pulls in the user specified ZMQ context
 * @param context_in is a shared_ptr to an initialized ZMQ context
 */
void ClusteredHashmapServer::init(std::shared_ptr<zmq::context_t> context_in)
{
    context = context_in;

    initialized = true;
}

/**
 * @brief Starts up the three sockets needed with threads
 * @throws std::runtime_error
 */
void ClusteredHashmapServer::start()
{
    if (!initialized)
    {
        throw std::runtime_error("ClusteredHashmapServer started before initialized");
    }

    running = true;

    publisherThreadHandle = std::thread(&ClusteredHashmapServer::publisherThread, this);
    snapshotThreadHandle = std::thread(&ClusteredHashmapServer::snapshotThread, this);
    collectorThreadHandle = std::thread(&ClusteredHashmapServer::collectorThread, this);

    publisherThreadHandle.detach();
    snapshotThreadHandle.detach();
    collectorThreadHandle.detach();
}

/**
 * @brief Stops the server and associated threads
 * @throws runtime_error if a thread fails to end
 */
void ClusteredHashmapServer::stop()
{
    mutex.lock();
    running = false;
    mutex.unlock();

    unsigned int retries = 0;

    while (publisherRunning)
    {
        if (retries > THREAD_CLOSE_WAIT_RETRIES)
        {
            throw std::runtime_error("ClusteredHashmapServer::publisher thread failed to end");
        }

        lager_utils::sleepMillis(THREAD_CLOSE_WAIT_MILLIS);
        retries++;

    }

    retries = 0;

    while (snapshotRunning)
    {
        if (retries > THREAD_CLOSE_WAIT_RETRIES)
        {
            throw std::runtime_error("ClusteredHashmapServer::snapshot thread failed to end");
        }

        lager_utils::sleepMillis(THREAD_CLOSE_WAIT_MILLIS);
        retries++;
    }

    retries = 0;

    while (collectorRunning)
    {
        if (retries > THREAD_CLOSE_WAIT_RETRIES)
        {
            throw std::runtime_error("ClusteredHashmapServer::collector thread failed to end");
        }

        lager_utils::sleepMillis(THREAD_CLOSE_WAIT_MILLIS);
        retries++;
    }
}

/**
 * @brief Updates the given key with the given value.  If the key doesn't exist, it is added.
 * @param key is a string containing the "nice" topic name
 * @param value is a string containing the XML data format of the topic
 */
void ClusteredHashmapServer::addOrUpdateKeyValue(const std::string& key, const std::string& value)
{
    hashMap[key] = value;
    updatedKeys.push_back(key);
}

/**
 * @brief Removes the given key from the hashmap.
 * @param key is a string containing the "nice" topic name
 */
void ClusteredHashmapServer::removeKey(const std::string& key)
{
    hashMap[key] = "";
    updatedKeys.push_back(key);
}

/**
 * @brief The main loop for the snapshot portion of the Clustered Hashmap Protocol
 */
void ClusteredHashmapServer::snapshotThread()
{
    snapshotRunning = true;

    // setting linger so the socket doesn't hang around after being stopped
    int linger = 0;

    try
    {
        if (!context)
        {
            if (running)
            {
                throw std::runtime_error("ClusteredHashmapServer::snapshotThread() attempted to start with a NULL context");
            }
            else
            {
                mutex.lock();
                snapshotRunning = false;
                mutex.unlock();
                return;
            }
        }

        snapshot.reset(new zmq::socket_t(*context.get(), ZMQ_ROUTER));
        snapshot->setsockopt(ZMQ_LINGER, &linger, sizeof(linger));
        snapshot->bind(lager_utils::getLocalUri(snapshotPort).c_str());

        // Sets up a poller for the snapshot socket
        zmq::pollitem_t items[] = {{static_cast<void*>(*snapshot.get()), 0, ZMQ_POLLIN, 0}};

        std::string identity("");
        std::string icanhaz("");

        while (running)
        {
            zmq::poll(&items[0], 1, -1);

            if (items[0].revents & ZMQ_POLLIN)
            {
                zmq::message_t msg;

                snapshot->recv(&msg);
                identity = std::string(static_cast<char*>(msg.data()), msg.size());

                snapshot->recv(&msg);
                icanhaz = std::string(static_cast<char*>(msg.data()), msg.size());

                // This is the subtree frame which is not used by this implementation, so
                // it is ignored.
                snapshot->recv(&msg);

                if (icanhaz == "ICANHAZ?")
                {
                    snapshotMap(identity);
                    snapshotBye(identity);
                }
            }
        }
    }
    catch (const zmq::error_t& e)
    {
        // This is the proper way of shutting down multithreaded ZMQ sockets.
        // The creator of the zmq context basically pulls the rug out from
        // under the socket.
        if (e.num() == ETERM)
        {
            if (snapshot)
            {
                snapshot->close();
            }

            mutex.lock();
            snapshotRunning = false;
            mutex.unlock();
            return;
        }
    }

    try
    {
        if (snapshot)
        {
            snapshot->close();
        }
    }
    catch (const zmq::error_t& e)
    {
        if (e.num() != ETERM)
        {
            std::stringstream ss;
            ss << "ClusteredHashmapServer::snapshotThread() uncaught zmq exception: " << e.what();
            throw std::runtime_error(ss.str());
        }
    }

    mutex.lock();
    snapshotRunning = false;
    mutex.unlock();
}

/**
 * @brief Sends the actual hashmap on the snapshot connection
 * @param identity is the identity of the originating peer so the ROUTER socket routes proprerly
 */
void ClusteredHashmapServer::snapshotMap(const std::string& identity)
{
    std::string empty("");
    std::string tmpUuid("");

    // Iterate the hashmap and send each entry
    for (auto i = hashMap.begin(); i != hashMap.end(); ++i)
    {
        // Find the associated uuid that matches the topic name
        for (auto u = uuidMap.begin(); u != uuidMap.end(); ++u)
        {
            if (u->second == i->first)
            {
                tmpUuid = u->first;
                break;
            }
        }

        // TODO throw here if uuid not found, shouldn't happen?

        try
        {
            // Build the frames of the ZMQ message and send
            zmq::message_t identityMsg(identity.size());
            zmq::message_t frame0(i->first.size());
            zmq::message_t frame1(sizeof(double));
            zmq::message_t frame2(tmpUuid.size());
            zmq::message_t frame3(empty.size());
            zmq::message_t frame4(i->second.size());

            memcpy(identityMsg.data(), identity.c_str(), identity.size());
            memcpy(frame0.data(), i->first.c_str(), i->first.size());
            memcpy(frame1.data(), (void*)&sequence, sizeof(double));
            memcpy(frame2.data(), tmpUuid.c_str(), tmpUuid.size());
            memcpy(frame3.data(), empty.c_str(), empty.size());
            memcpy(frame4.data(), i->second.c_str(), i->second.size());

            snapshot->send(identityMsg, ZMQ_SNDMORE);
            snapshot->send(frame0, ZMQ_SNDMORE);
            snapshot->send(frame1, ZMQ_SNDMORE);
            snapshot->send(frame2, ZMQ_SNDMORE);
            snapshot->send(frame3, ZMQ_SNDMORE);
            snapshot->send(frame4);
        }
        catch (const zmq::error_t& e)
        {
            if (e.num() != ETERM)
            {
                std::stringstream ss;
                ss << "ClusteredHashmapServer::snapshotMap() uncaught zmq exception: " << e.what();
                throw std::runtime_error(ss.str());
            }
        }
    }
}

/**
 * @brief Sends the final message after the hashmap is completed
 * @param identity is the identity of the originating peer so the ROUTER socket routes proprerly
 */
void ClusteredHashmapServer::snapshotBye(const std::string& identity)
{
    std::string kthxbai("KTHXBAI");
    std::string empty("");
    std::string subtree("");

    try
    {
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
    }
    catch (const zmq::error_t& e)
    {
        if (e.num() != ETERM)
        {
            std::stringstream ss;
            ss << "ClusteredHashmapServer::snapshotBye() uncaught zmq exception: " << e.what();
            throw std::runtime_error(ss.str());
        }
    }

    sequence++;
}

/**
 * @brief Sends any keys which have been updated since the last update on the publisher
 */
void ClusteredHashmapServer::publishUpdatedKeys()
{
    std::string empty("");
    std::string tmpUuid("");
    std::vector<std::string> removedKeys;

    // Iterate the updated map and send each entry
    for (auto i = updatedKeys.begin(); i != updatedKeys.end(); ++i)
    {
        std::string tmpKey = *i;

        // Find the associated uuid that matches the topic name
        for (auto u = uuidMap.begin(); u != uuidMap.end(); ++u)
        {
            if (u->second == tmpKey)
            {
                tmpUuid = u->first;
                break;
            }
        }

        // TODO throw here if uuid not found, shouldn't happen?

        zmq::message_t frame0((tmpKey).size());
        zmq::message_t frame1(sizeof(double));
        zmq::message_t frame2(tmpUuid.size());
        zmq::message_t frame3(empty.size());
        zmq::message_t frame4(hashMap[tmpKey].size());

        memcpy(frame0.data(), (tmpKey).c_str(), (tmpKey).size());
        memcpy(frame1.data(), (void*)&sequence, sizeof(double));
        memcpy(frame2.data(), tmpUuid.c_str(), tmpUuid.size());
        memcpy(frame3.data(), empty.c_str(), empty.size());
        memcpy(frame4.data(), hashMap[tmpKey].c_str(), hashMap[tmpKey].size());

        publisher->send(frame0, ZMQ_SNDMORE);
        publisher->send(frame1, ZMQ_SNDMORE);
        publisher->send(frame2, ZMQ_SNDMORE);
        publisher->send(frame3, ZMQ_SNDMORE);
        publisher->send(frame4);

        // if the entry is empty that means it needs to be removed
        if (hashMap[tmpKey].size() == 0)
        {
            removedKeys.push_back(tmpKey);
        }

        // clients should check for sequence to ensure they have the most recent version
        sequence++;
    }

    // Remove any deleted keys from the hashmap
    for (auto i = removedKeys.begin(); i != removedKeys.end(); ++i)
    {
        hashMap.erase(*i);
    }

    updatedKeys.clear();
}

/**
 * @brief Publisher main thread. Either sends HUGZ watchdog message or updated hashmap keys if they exist.
 */
void ClusteredHashmapServer::publisherThread()
{
    publisherRunning = true;

    // setting high water mark of 1 so messages don't stack up
    int hwm = 1;

    // setting linger so the socket doesn't hang around after being stopped
    int linger = 0;

    try
    {
        if (!context)
        {
            if (running)
            {
                throw std::runtime_error("ClusteredHashmapServer::publisherThread() attempted to start with a NULL context");
            }
            else
            {
                mutex.lock();
                publisherRunning = false;
                mutex.unlock();
                return;
            }
        }

        publisher.reset(new zmq::socket_t(*context.get(), ZMQ_PUB));
        publisher->setsockopt(ZMQ_RCVHWM, &hwm, sizeof(hwm));
        publisher->setsockopt(ZMQ_LINGER, &linger, sizeof(linger));
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

            // TODO this could probably run slower
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    catch (const zmq::error_t& e)
    {
        // This is the proper way of shutting down multithreaded ZMQ sockets.
        // The creator of the zmq context basically pulls the rug out from
        // under the socket.
        if (e.num() == ETERM)
        {
            if (publisher)
            {
                publisher->close();
            }

            mutex.lock();
            publisherRunning = false;
            mutex.unlock();
            return;
        }
    }

    try
    {
        if (publisher)
        {
            publisher->close();
        }
    }
    catch (const zmq::error_t& e)
    {
        if (e.num() != ETERM)
        {
            std::stringstream ss;
            ss << "ClusteredHashmapServer::publisherThread() uncaught zmq exception: " << e.what();
            throw std::runtime_error(ss.str());
        }
    }

    mutex.lock();
    publisherRunning = false;
    mutex.unlock();
}

/**
 * @brief Sends the hugz watchdog message
 */
void ClusteredHashmapServer::publishHugz()
{
    std::string hugz("HUGZ");
    std::string empty("");
    double zero = 0;

    try
    {
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
    catch (const zmq::error_t& e)
    {
        if (e.num() != ETERM)
        {
            std::stringstream ss;
            ss << "ClusteredHashmapServer::publishHugz() uncaught zmq exception: " << e.what();
            throw std::runtime_error(ss.str());
        }
    }
}

/**
 * @brief Main thread for the collector socket
 */
void ClusteredHashmapServer::collectorThread()
{
    collectorRunning = true;

    // setting high water mark of 1 so messages don't stack up
    int hwm = 1;

    // setting linger so the socket doesn't hang around after being stopped
    int linger = 0;

    try
    {
        if (!context)
        {
            if (running)
            {
                throw std::runtime_error("ClusteredHashmapServer::collectorThread() attempted to start with a NULL context");
            }
            else
            {
                mutex.lock();
                collectorRunning = false;
                mutex.unlock();
                return;
            }
        }

        collector.reset(new zmq::socket_t(*context.get(), ZMQ_SUB));
        collector->setsockopt(ZMQ_RCVHWM, &hwm, sizeof(hwm));
        collector->setsockopt(ZMQ_LINGER, &linger, sizeof(linger));
        collector->bind(lager_utils::getLocalUri(collectorPort).c_str());

        // We want all messages on the socket
        collector->setsockopt(ZMQ_SUBSCRIBE, "", 0);

        // Sets up a poller for the collector socket
        zmq::pollitem_t items[] = {{static_cast<void*>(*collector.get()), 0, ZMQ_POLLIN, 0}};

        std::string key("");
        std::string uuid("");
        std::string value("");
        bool duplicateKey = false;

        while (running)
        {
            zmq::poll(&items[0], 1, -1);

            // Check for new data
            if (items[0].revents & ZMQ_POLLIN)
            {
                zmq::message_t msg;

                // Grab the new messages
                // TODO should check ZMQ_RCVMORE to ensure we don't get stuck
                collector->recv(&msg);
                key = std::string(static_cast<char*>(msg.data()), msg.size());

                // This is the sequence number frame and has no significance in this piece of CHP
                collector->recv(&msg);

                collector->recv(&msg);
                uuid = std::string(static_cast<char*>(msg.data()), msg.size());

                // This is the properties frame and is currently unused, so do nothing
                collector->recv(&msg);

                collector->recv(&msg);
                value = std::string(static_cast<char*>(msg.data()), msg.size());

                // Uuid is required, if it's empty, ignore this message
                // TODO should we really ignore this, or bubble up an error?
                if (uuid.length() > 0)
                {
                    // Check to see if the uuid is already in use
                    if (uuidMap.find(uuid) == uuidMap.end())
                    {
                        duplicateKey = false;

                        for (auto i = uuidMap.begin(); i != uuidMap.end(); ++i)
                        {
                            // Duplicate key exists in the map under another uuid, for now we ignore it
                            if (i->second == key)
                            {
                                duplicateKey = true;
                                std::cout << "key already registered by another uuid. TBD action" << std::endl;
                            }
                        }

                        // add the key to the uuid map
                        if (!duplicateKey)
                        {
                            uuidMap[uuid] = key;
                        }
                    }
                }

                // If the value is an empty string it means to remove that key from the map
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
    catch (const zmq::error_t& e)
    {
        // This is the proper way of shutting down multithreaded ZMQ sockets.
        // The creator of the zmq context basically pulls the rug out from
        // under the socket.
        if (e.num() == ETERM)
        {
            if (collector)
            {
                collector->close();
            }

            mutex.lock();
            collectorRunning = false;
            mutex.unlock();
            return;
        }
    }

    try
    {
        if (collector)
        {
            collector->close();
        }
    }
    catch (const zmq::error_t& e)
    {
        if (e.num() != ETERM)
        {
            std::stringstream ss;
            ss << "ClusteredHashmapServer::collectorThread() uncaught zmq exception: " << e.what();
            throw std::runtime_error(ss.str());
        }
    }

    mutex.lock();
    collectorRunning = false;
    mutex.unlock();
}
