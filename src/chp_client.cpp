#include "lager/chp_client.h"

/**
 * @brief Constructor, sets the various ports needed by Lager
 * @param serverHost_in is a string containing the host or IP address of the Lager Bartender
 * @param basePort is an int base port Lager will use to calculate the other used ports
 * @param timeoutMillis_in is an int which sets the polling timeout of the various zmq sockets used
 */
ClusteredHashmapClient::ClusteredHashmapClient(const std::string& serverHost_in, int basePort, int timeoutMillis_in):
    initialized(false), running(false), snapshotRunning(false), subscriberRunning(false),
    publisherRunning(false), timedOut(false), sequence(-1), uuid("invalid"),
    serverHost(serverHost_in), timeoutMillis(timeoutMillis_in)
{
    snapshotPort = basePort + CHP_SNAPSHOT_OFFSET;
    subscriberPort = basePort + CHP_PUBLISHER_OFFSET;
    publisherPort = basePort + CHP_COLLECTOR_OFFSET;
}

/**
 * @brief Sets the zmq context and uuid to be used by the client
 * @param context_in is a shared_ptr to the zmq context initialized by the caller
 * @param uuid_in is a 16 byte string representation of a standard uuid
 */
void ClusteredHashmapClient::init(std::shared_ptr<zmq::context_t> context_in, const std::string& uuid_in)
{
    context = context_in;
    uuid = uuid_in;

    initialized = true;
}

/**
 * @brief Starts the two threads used by the client
 * @throws runtime_error if this function is called before init()
 */
void ClusteredHashmapClient::start()
{
    if (!initialized)
    {
        throw std::runtime_error("ClusteredHashmapClient started before initialized");
    }

    std::unique_lock<std::mutex> lock(mutex);

    running = true;

    snapshotThreadHandle = std::thread(&ClusteredHashmapClient::snapshotThread, this);
    subscriberThreadHandle = std::thread(&ClusteredHashmapClient::subscriberThread, this);
    publisherThreadHandle = std::thread(&ClusteredHashmapClient::publisherThread, this);

    snapshotThreadHandle.detach();
    subscriberThreadHandle.detach();
    publisherThreadHandle.detach();

    while (!subscriberRunning || !snapshotRunning || !publisherRunning)
    {
        cv.wait(lock);
    }
}

/**
 * @brief Stops the running threads used by the client
 * @throws runtime_error if a thread fails to end
 */
void ClusteredHashmapClient::stop()
{
    mutex.lock();
    running = false;
    mutex.unlock();

    unsigned int retries = 0;

    while (subscriberRunning)
    {
        if (retries > THREAD_CLOSE_WAIT_RETRIES)
        {
            throw std::runtime_error("ClusteredHashmapClient::subscriber thread failed to end");
        }

        lager_utils::sleepMillis(THREAD_CLOSE_WAIT_MILLIS);
        retries++;
    }

    retries = 0;

    while (snapshotRunning)
    {
        if (retries > THREAD_CLOSE_WAIT_RETRIES)
        {
            throw std::runtime_error("ClusteredHashmapClient::snapshot thread failed to end");
        }

        lager_utils::sleepMillis(THREAD_CLOSE_WAIT_MILLIS);
        retries++;
    }

    retries = 0;

    while (publisherRunning)
    {
        if (retries > THREAD_CLOSE_WAIT_RETRIES)
        {
            throw std::runtime_error("ClusteredHashmapClient::publisher thread failed to end");
        }

        lager_utils::sleepMillis(THREAD_CLOSE_WAIT_MILLIS);
        retries++;
    }
}

/**
 * @brief Fires off the publisher thread in a one-shot asynchronous way in order to add or update a key in the map
 * @param key is a string containing the key of the key, value pair
 * @param value is a string containing the value of the key, value pair, in Lager's case, the XML of the data format
 */
void ClusteredHashmapClient::addOrUpdateKeyValue(const std::string& key, const std::string& value)
{
    // std::async(std::launch::async, &ClusteredHashmapClient::publisherThread, this, key, value);
    // make sure we keep track of our own (key, value) changes.
    // we don't remove the key completely on delete because we want to
    // know if the removed item was in fact removed
    mutex.lock();
    selfMap[key] = value;
    mutex.unlock();
}

/**
 * @brief Fires off the publisher thread in a one-shot asynchronous way in order to remove a key in the map
 * @param key is a string containing the key of the key to remove
 */
void ClusteredHashmapClient::removeKey(const std::string& key)
{
    // Sending an empty string as the value will tell the server to remove the key
    // std::async(std::launch::async, &ClusteredHashmapClient::publisherThread, this, key, "");
    // make sure we keep track of our own (key, value) changes.
    // we don't remove the key completely on delete because we want to
    // know if the removed item was in fact removed
    mutex.lock();
    selfMap[key] = "";
    mutex.unlock();
}

/**
 * @brief Sets the callback method used whenever the hashmap is updated, to provide external objects notification
 * @param func is a function pointer passed in to be called when the hashmap is updated
 */
void ClusteredHashmapClient::setCallback(const std::function<void()>& func)
{
    hashMapUpdated = func;
}

/**
 * @brief Checks the self map to ensure this client's own key/values added are in the map.
 * If not, re-send the updates to the server.
 */
std::map<std::string, std::string> ClusteredHashmapClient::getUnsyncedHashmap()
{
    std::map<std::string, std::string> unsyncedHashmap;

    for (auto i = selfMap.begin(); i != selfMap.end(); ++i)
    {
        if (hashMap.find(i->first) == hashMap.end())
        {
            // if the item isn't in the hashmap and has a non-empty value,
            // we haven't received the update yet
            if (i->second.size() > 0)
            {
                unsyncedHashmap[i->first] = i->second;
            }
        }
        else
        {
            bool uuidsMatch = false;

            // we found the item, now we have to check to make sure it's
            // our uuid
            for (auto j = uuidMap.begin(); j != uuidMap.end(); ++j)
            {
                if (j->second == i->first)
                {
                    if (uuid == j->first)
                    {
                        uuidsMatch = true;
                    }
                }
            }

            // if the item is in the hashmap but has a different value,
            // we haven't received the update yet
            if (hashMap[i->first] != selfMap[i->first])
            {
                // make sure it's our uuid, else it's a dupe with a different client
                // TODO some kind of throw here perhaps?  user defined?
                if (uuidsMatch)
                {
                    unsyncedHashmap[i->first] = i->second;
                }
            }
        }
    }

    return unsyncedHashmap;
}

/**
 * @brief Main subscriber thread used to receive updates to the hashmap from the server
 */
void ClusteredHashmapClient::subscriberThread()
{
    timedOut = false;

    try
    {
        if (!context)
        {
            if (running)
            {
                throw std::runtime_error("ClusteredHashmapClient::subscriberThread attempted to start with a NULL context");
            }
            else
            {
                mutex.lock();
                subscriberRunning = false;
                timedOut = true;
                mutex.unlock();
                return;
            }
        }

        subscriber.reset(new zmq::socket_t(*context.get(), ZMQ_SUB));

        // setting high water mark of 1 so messages don't stack up
        int hwm = 1;

        // setting linger so the socket doesn't hang around after being stopped
        int linger = 0;

        subscriber->setsockopt(ZMQ_RCVHWM, &hwm, sizeof(hwm));
        subscriber->setsockopt(ZMQ_LINGER, &linger, sizeof(linger));

        // we want all messages from the server
        subscriber->setsockopt(ZMQ_SUBSCRIBE, "", 0);

        subscriber->connect(lager_utils::getRemoteUri(serverHost.c_str(), subscriberPort).c_str());

        std::string key("");
        std::string value("");
        std::string uuid("");

        // Sets up a poller for the subscriber socket
        zmq::pollitem_t items[] = {{static_cast<void*>(*subscriber.get()), 0, ZMQ_POLLIN, 0}};

        // counter to determine if the client is getting the latest message
        double sequence = 0;

        subscriberRunning = true;
        cv.notify_all();

        while (running)
        {
            zmq::poll(&items[0], 1, timeoutMillis);

            // Check for new data
            if (items[0].revents & ZMQ_POLLIN)
            {
                zmq::message_t msg;

                subscriber->recv(&msg);
                key = std::string(static_cast<char*>(msg.data()), msg.size());

                subscriber->recv(&msg);
                sequence = *(double*)msg.data();

                subscriber->recv(&msg);
                uuid = std::string(static_cast<char*>(msg.data()), msg.size());

                // this frame is empty so we just grab the message and do nothing with it
                subscriber->recv(&msg);

                subscriber->recv(&msg);
                value = std::string(static_cast<char*>(msg.data()), msg.size());

                // if this is a hugz message (watchdog) then there's nothing else to do
                if (key != "HUGZ")
                {
                    // TODO what should be the action here if sequence count fails?
                    if (sequence > this->sequence)
                    {
                        // when value is an empty string, we delete the key from the hashmap
                        if (value.length() == 0)
                        {
                            hashMap.erase(key);
                        }
                        else
                        {
                            // set the new hashmap key, value pair
                            hashMap[key] = value;

                            // keep the uuid mapping for later use
                            uuidMap[uuid] = key;

                            // if the user set a callback, call it
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
                // this occurs after timeoutMillis with no hugz or updates
                timedOut = true;
            }
        }
    }
    catch (zmq::error_t& e)
    {
        // This is the proper way of shutting down multithreaded ZMQ sockets.
        // The creator of the zmq context basically pulls the rug out from
        // under the socket.
        if (e.num() == ETERM)
        {
            try
            {
                if (subscriber)
                {
                    subscriber->close();
                }
            }
            catch (const zmq::error_t& e)
            {
                if (e.num() != ETERM)
                {
                    std::stringstream ss;
                    ss << "ClusteredHashmapClient::subscriberThread() uncaught zmq exception: " << e.what();
                    throw std::runtime_error(ss.str());
                }
            }

            mutex.lock();
            subscriberRunning = false;
            timedOut = true;
            mutex.unlock();
            return;
        }
    }

    try
    {
        if (subscriber)
        {
            subscriber->close();
        }
    }
    catch (const zmq::error_t& e)
    {
        if (e.num() != ETERM)
        {
            std::stringstream ss;
            ss << "ClusteredHashmapClient::subscriberThread() uncaught zmq exception: " << e.what();
            throw std::runtime_error(ss.str());
        }
    }

    mutex.lock();
    subscriberRunning = false;
    timedOut = true;
    mutex.unlock();
}

/**
 * @brief Main snapshot thread used to initially request the hashmap from the server
 *
 * The message conversation is an "ICANHAZ?" which the server then responds with zero or more
 * hashmap entries, ending with "KTHXBAI".
 */
void ClusteredHashmapClient::snapshotThread()
{
    // setting high water mark of 1 so messages don't stack up
    int hwm = 1;

    // linger zero so the socket shuts down nicely later
    int linger = 0;

    std::map<std::string, std::string> updateMap; // <key, value>
    std::map<std::string, std::string> updateUuids; // <uuid, key>

    // the value specified in the zmq chp protocol
    std::string icanhaz("ICANHAZ?");

    // currently unused but may later be used to request specific keys from the server
    std::string subtree("");

    std::string value("");
    std::string key("");
    std::string uuid("");

    try
    {
        if (!context)
        {
            if (running)
            {
                throw std::runtime_error("ClusteredHashmapClient::snapshotThread attempted to start with a NULL context");
            }
            else
            {
                mutex.lock();
                snapshotRunning = false;
                mutex.unlock();
                return;
            }
        }

        snapshot.reset(new zmq::socket_t(*context.get(), ZMQ_DEALER));
        snapshot->setsockopt(ZMQ_SNDHWM, &hwm, sizeof(hwm));
        snapshot->setsockopt(ZMQ_LINGER, &linger, sizeof(linger));
        snapshot->connect(lager_utils::getRemoteUri(serverHost.c_str(), snapshotPort).c_str());

        snapshotRunning = true;
        cv.notify_all();

        while (running)
        {
            if (key != "KTHXBAI")
            {
                // send the two multipart messages
                zmq::message_t reqFrame0(icanhaz.size());
                zmq::message_t reqFrame1(subtree.size());

                memcpy(reqFrame0.data(), icanhaz.c_str(), icanhaz.size());
                memcpy(reqFrame1.data(), subtree.c_str(), subtree.size());

                snapshot->send(reqFrame0, ZMQ_SNDMORE);
                snapshot->send(reqFrame1);

                // Sets up a poller for the subscriber socket
                zmq::pollitem_t items[] = {{static_cast<void*>(*snapshot.get()), 0, ZMQ_POLLIN, 0}};
                zmq::poll(items, 1, timeoutMillis);

                if (items[0].revents & ZMQ_POLLIN)
                {
                    zmq::message_t replyMsg;

                    snapshot->recv(&replyMsg);
                    key = std::string(static_cast<char*>(replyMsg.data()), replyMsg.size());

                    // this frame is the sequence number and is unused for the snapshot
                    snapshot->recv(&replyMsg);

                    snapshot->recv(&replyMsg);
                    uuid = std::string(static_cast<char*>(replyMsg.data()), replyMsg.size());

                    // this frame is empty so we just grab the message and do nothing with it
                    snapshot->recv(&replyMsg);

                    snapshot->recv(&replyMsg);
                    value = std::string(static_cast<char*>(replyMsg.data()), replyMsg.size());

                    // if value is empty we're supposed to delete, but since this is the first
                    // iteration that can't happen, we'll just ignore.
                    if (value.length() > 0)
                    {
                        updateMap[key] = value;
                        updateUuids[uuid] = key;
                    }
                }
                else
                {
                    // If we get here, we didn't get a complete hashmap set, only process a complete set
                    // TODO some kind of error should probably be thrown
                    updateMap.clear();
                    updateUuids.clear();
                }
            }
            else
            {
                // we got the KTHXBAI message
                break;
            }
        }
    }
    catch (zmq::error_t& e)
    {
        // This is the proper way of shutting down multithreaded ZMQ sockets.
        // The creator of the zmq context basically pulls the rug out from
        // under the socket.
        if (e.num() == ETERM)
        {
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
                    ss << "ClusteredHashmapClient::snapshotThread() uncaught zmq exception: " << e.what();
                    throw std::runtime_error(ss.str());
                }
            }

            mutex.lock();
            snapshotRunning = false;
            mutex.unlock();
            return;
        }
    }

    // we got a complete hashmap set, now update the actual hashmap
    for (auto i = updateMap.begin(); i != updateMap.end(); ++i)
    {
        hashMap[i->first] = i->second;
    }

    // now update the uuid map so we can trace uuids to keys
    for (auto i = updateUuids.begin(); i != updateUuids.end(); ++i)
    {
        uuidMap[i->first] = i->second;
    }

    if (updateMap.size() > 0)
    {
        // if we had updates and the user specified a callback, call it
        if (hashMapUpdated)
        {
            hashMapUpdated();
        }
    }

    // This thread ends after one successful call, subsequent hashmap updates come
    // from the subscriber thread
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
            ss << "ClusteredHashmapClient::snapshotThread() uncaught zmq exception: " << e.what();
            throw std::runtime_error(ss.str());
        }
    }

    mutex.lock();
    snapshotRunning = false;
    mutex.unlock();
}

/**
 * @brief Main publisher thread for the chp client to send updates to the server
 * @param key is a string value of the hashmap key to update
 * @param value is a string value of the hashmap value to update (empty string removes the key from the map)
 */
void ClusteredHashmapClient::publisherThread()
{
    // setting high water mark of 1 so messages don't stack up
    int hwm = 1;

    // linger zero so the socket shuts down nicely later
    int linger = 0;

    // TODO implement properties
    std::string properties("");
    double zero = 0;

    std::map<std::string, std::string> unsyncedHashmap;

    try
    {
        if (!context)
        {
            if (running)
            {
                throw std::runtime_error("ClusteredHashmapClient::publisherThread attempted to start with a NULL context");
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
        publisher->setsockopt(ZMQ_SNDHWM, &hwm, sizeof(hwm));
        publisher->setsockopt(ZMQ_LINGER, &linger, sizeof(linger));
        publisher->connect(lager_utils::getRemoteUri(serverHost.c_str(), publisherPort).c_str());

        publisherRunning = true;
        cv.notify_all();

        while (running)
        {
            // keep sending the updates until we get them back from the server
            unsyncedHashmap = getUnsyncedHashmap();

            for (auto i = unsyncedHashmap.begin(); i != unsyncedHashmap.end(); ++i)
            {
                zmq::message_t frame0(i->first.size());
                zmq::message_t frame1(sizeof(double));
                zmq::message_t frame2(uuid.size());
                zmq::message_t frame3(properties.size());
                zmq::message_t frame4(i->second.size());

                memcpy(frame0.data(), i->first.c_str(), i->first.size());
                memcpy(frame1.data(), (void*)&zero, sizeof(double));
                memcpy(frame2.data(), uuid.c_str(), uuid.size());
                memcpy(frame3.data(), properties.c_str(), properties.size());
                memcpy(frame4.data(), i->second.c_str(), i->second.size());

                publisher->send(frame0, ZMQ_SNDMORE);
                publisher->send(frame1, ZMQ_SNDMORE);
                publisher->send(frame2, ZMQ_SNDMORE);
                publisher->send(frame3, ZMQ_SNDMORE);
                publisher->send(frame4);
            }

            // keep this from running at full speed
            lager_utils::sleepMillis(100);
        }
    }
    catch (const zmq::error_t& e)
    {
        // This is the proper way of shutting down multithreaded ZMQ sockets.
        // The creator of the zmq context basically pulls the rug out from
        // under the socket.
        if (e.num() == ETERM)
        {
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
                    ss << "ClusteredHashmapClient::publisherThread() uncaught zmq exception: " << e.what();
                    throw std::runtime_error(ss.str());
                }
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
            ss << "ClusteredHashmapClient::publisherThread() uncaught zmq exception: " << e.what();
            throw std::runtime_error(ss.str());
        }
    }

    mutex.lock();
    publisherRunning = false;
    mutex.unlock();
}
