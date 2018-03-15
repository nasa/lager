#include "lager/tap.h"

Tap::Tap(): publisherPort(0), running(false), newData(false), flags(0), offsetCount(0), timestamp(0),
    publisherRunning(false)
{
}

Tap::~Tap()
{
    // TODO make these smart pointers
    for (auto i = dataRefItems.begin(); i != dataRefItems.end(); ++i)
    {
        // delete (*i);
    }
}

/**
* @brief Starts the zmq context and initializes the tap with the given bartender information
* @param serverHost_in is a string containing the IP address or hostname of the bartender to connect to
* @param basePort is an integer containing the port of the bartender to connect to
* @param timeOutMillis is the timeout to the bartender
* @returns true on success, false on failure
*/
bool Tap::init(const std::string& serverHost_in, int basePort, int timeOutMillis)
{
    context.reset(new zmq::context_t(1));

    publisherPort = basePort + FORWARDER_FRONTEND_OFFSET;

    if (publisherPort < 0 || publisherPort > BASEPORT_MAX)
    {
        // TODO provide stream output of errors?
        return false;
    }

    uuid = lager_utils::getUuid();
    serverHost = serverHost_in;

    // TODO make this not magic 2000 default timeout for testing
    chpClient.reset(new ClusteredHashmapClient(serverHost_in, basePort, timeOutMillis));
    chpClient->init(context, uuid);

    return true;
}

/**
* @brief Adds a new data column item to the tap which contains a reference to actual user data being logged
* @param item is an DataRefItem inherited, templated object containing the info about a particular column as
* well as a reference to the actual data.
*/
void Tap::addItem(AbstractDataRefItem* item)
{
    // set the offset of the new item based on order of addition
    item->setOffset(offsetCount);

    dataRefItems.push_back(item);

    // keeps track of the offset for later generation of the data format xml
    offsetCount += item->getSize();
}

/**
* @brief Starts the tap by setting up the CHP connection to the bartender and starting the publisher thread
* @param key_in is the "topic name" of this particular tap as it will be represented by the bartender
* @throws runtime_error if the xml format of the tap was unable to be generated
*/
void Tap::start(const std::string& key_in)
{
    // TODO this should probably be compiled in from the cmake or something
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

    chpClient->start();
    // sets the hashmap value so it will be sent to the bartender
    chpClient->addOrUpdateKeyValue(key, formatStr);

    publisherThreadHandle = std::thread(&Tap::publisherThread, this);
    publisherThreadHandle.detach();
}

/**
* @brief Stops the tap by closing the zmq context and stopping chp and the publisher thread
* @throws runtime_error if the publisher thread fails to end
*/
void Tap::stop()
{
    mutex.lock();
    running = false;
    mutex.unlock();

    zmq_ctx_shutdown((void*)*context.get());

    unsigned int retries = 0;

    while (publisherRunning)
    {
        if (retries > THREAD_CLOSE_WAIT_RETRIES)
        {
            throw std::runtime_error("Tap::publisher thread failed to end");
        }

        lager_utils::sleepMillis(THREAD_CLOSE_WAIT_MILLIS);
        retries++;
    }

    chpClient->stop();
    context->close();
}

/**
* @brief Logs the data references set up by the tap with the current system timestamp
* The newData flag indicates to the publisher thread that it needs to publish again
*/
// TODO allow user to pass the timestamp in
void Tap::log()
{
    mutex.lock();
    timestamp = lager_utils::getCurrentTime();
    newData = true;
    mutex.unlock();
}

/**
* @brief Main publisher thread of the tap
*/
void Tap::publisherThread()
{
    publisherRunning = true;
    zmq::socket_t publisher(*context.get(), ZMQ_PUB);

    // setting linger so the socket doesn't hang around after being stopped
    int linger = 0;

    try
    {
        publisher.setsockopt(ZMQ_LINGER, &linger, sizeof(linger));
        publisher.connect(lager_utils::getRemoteUri(serverHost, publisherPort).c_str());

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

                // TODO endianness
                memcpy(flagsMsg.data(), (void*)&flags, sizeof(flags));
                uint64_t networkTimestamp = lager_utils::htonll(timestamp);
                memcpy(timestampMsg.data(), (void*)&networkTimestamp, sizeof(timestamp));

                publisher.send(uuidMsg, ZMQ_SNDMORE);
                publisher.send(versionMsg, ZMQ_SNDMORE);
                publisher.send(flagsMsg, ZMQ_SNDMORE);
                publisher.send(timestampMsg, ZMQ_SNDMORE);

                for (unsigned int i = 0; i < dataRefItems.size(); ++i)
                {
                    zmq::message_t tmp(dataRefItems[i]->getSize());
                    dataRefItems[i]->getNetworkDataRef(tmp.data());

                    // make sure to use the sndmore flag until the last message
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
    }
    catch (const zmq::error_t& e)
    {
        // This is the proper way of shutting down multithreaded ZMQ sockets.
        // The creator of the zmq context basically pulls the rug out from
        // under the socket.
        if (e.num() == ETERM)
        {
            publisher.close();
            mutex.lock();
            publisherRunning = false;
            mutex.unlock();
            return;
        }
    }

    publisher.close();
    mutex.lock();
    publisherRunning = false;
    mutex.unlock();
}
