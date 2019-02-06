#include "lager/mug.h"

#ifdef WITH_LTTNG
#include "lager.tp.h"
#endif

/**
* @brief Constructor, sets an invalid port to ensure the user initializes properly
*/
Mug::Mug(): running(false), subscriberPort(-1), subscriberRunning(false)
{
}

Mug::~Mug()
{
}

/**
* @brief Initializes the mug by creating the zmq context and starting up the CHP client
* @param serverHost_in is a string containing the IP or hostname of the bartender to connect to
* @param basePort is an integer containing a the port of the bartender to connect to
* @param timeOutMillis is an integer containing the timeout for the bartender
* @param kegDir is a string containing a path to an accessible directory to store the keg files
* @returns true on successful initialization, false on failure
*/
bool Mug::init(const std::string& serverHost_in, int basePort, int timeOutMillis, const std::string& kegDir)
{
    subscriberPort = basePort + FORWARDER_BACKEND_OFFSET;

    if (subscriberPort < 0 || subscriberPort > BASEPORT_MAX)
    {
        // TODO provide stream output of errors?
        return false;
    }

    serverHost = serverHost_in;

    context.reset(new zmq::context_t(1));

    // TODO make this not magic 2000 default timeout for testing
    chpClient.reset(new ClusteredHashmapClient(serverHost_in, basePort, timeOutMillis));
    chpClient->init(context, uuid);
    hashMapUpdatedHandle = std::bind(&Mug::hashMapUpdated, this);
    chpClient->setCallback(hashMapUpdatedHandle);

    formatParser.reset(new DataFormatParser);

    keg.reset(new Keg(kegDir));

    return true;
}

/**
* @brief Starts the mug subscriber thread
*/
void Mug::start()
{
    running = true;

    keg->start();

    subscriberThreadHandle = std::thread(&Mug::subscriberThread, this);
    subscriberThreadHandle.detach();

    chpClient->start();
}

/**
* @brief Closes the zmq context and stops the subscriber and chp threads
* @throws runtime_error if the subscriber thread fails to end
*/
void Mug::stop()
{
    mutex.lock();
    running = false;
    mutex.unlock();

    zmq_ctx_shutdown((void*)*context.get());

    unsigned int retries = 0;

    while (subscriberRunning)
    {
        if (retries > THREAD_CLOSE_WAIT_RETRIES)
        {
            throw std::runtime_error("Mug::subscriber thread failed to end");
        }

        lager_utils::sleepMillis(THREAD_CLOSE_WAIT_MILLIS);
        retries++;
    }

    chpClient->stop();
    keg->stop();
    context->close();
}

/**
* @brief Callback function to update the hashmap and format maps whenever the chp client hashmap is updated
*/
void Mug::hashMapUpdated()
{
    std::map<std::string, std::string> tmpUuidMap; // <uuid, topic name>
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

/**
* @brief The main data subscriber thread
*/
void Mug::subscriberThread()
{
    subscriberRunning = true;

    // setting linger so the socket doesn't hang around after being stopped
    int linger = 0;

    try
    {
        if (!context)
        {
            if (running)
            {
                throw std::runtime_error("Mug::subscriberThread attempted to start with a NULL context");
            }
            else
            {
                mutex.lock();
                subscriberRunning = false;
                mutex.unlock();
                return;
            }
        }

        subscriber.reset(new zmq::socket_t(*context.get(), ZMQ_SUB));
        subscriber->setsockopt(ZMQ_LINGER, &linger, sizeof(linger));
        subscriber->connect(lager_utils::getRemoteUri(serverHost.c_str(), subscriberPort).c_str());

        // subscribe to everything (for now)
        // TODO set up filter by uuid of subscribed topics
        subscriber->setsockopt(ZMQ_SUBSCRIBE, "", 0);

        uint64_t timestamp;
        std::vector<uint8_t> data; // the main buffer we're keeping

        // all data that comes from a tap will be one of these sizes
        uint8_t tmp8;
        uint16_t tmp16;
        uint32_t tmp32;
        uint64_t tmp64;

        off_t offset; // keeps track of the current offset of the buffer while recieving more frames

        uint32_t rcvMore = 0;
        size_t moreSize = sizeof(rcvMore);

        // set up a poller for the subscriber socket
        zmq::pollitem_t items[] = {{static_cast<void*>(*subscriber.get()), 0, ZMQ_POLLIN, 0}};

#ifdef WITH_LTTNG
        int msgPartCount = 0;
        tracepoint(lager_tp, lager_tp_general, "Mug::subscriberThread()", "start");
#endif

        while (running)
        {
            // TODO check if this timeout should be different or setable by user
            zmq::poll(&items[0], 1, 1000);

            if (items[0].revents & ZMQ_POLLIN)
            {
                // clear the buffer each time
                data.clear();
                offset = 0;

                zmq::message_t msg;

                subscriber->recv(&msg);
                std::string uuid(static_cast<char*>(msg.data()), msg.size());

                // Version frame, string, currently unused
                subscriber->recv(&msg);

                // Compression frame, uint16_t, currently unused
                subscriber->recv(&msg);

                subscriber->recv(&msg);
                timestamp = *static_cast<uint64_t*>(msg.data());

                if (uuid.size() != UUID_SIZE_BYTES)
                {
                    throw std::runtime_error("received invalid uuid size");
                }

#ifdef WITH_LTTNG
                // the four message parts above
                msgPartCount = 4;
#endif

                // uuid is first in the buffer
                for (size_t i = 0; i < uuid.size(); ++i)
                {
                    data.push_back(uuid[i]);
                }

                offset += UUID_SIZE_BYTES;

                // timestamp is next in the buffer
                data.resize(data.size() + TIMESTAMP_SIZE_BYTES);
                *(reinterpret_cast<uint64_t*>(data.data() + offset)) = timestamp;

                offset += TIMESTAMP_SIZE_BYTES;

                // make sure we have more of the multipart zmq message waiting
                subscriber->getsockopt(ZMQ_RCVMORE, &rcvMore, &moreSize);

                while (rcvMore != 0)
                {
                    subscriber->recv(&msg);

                    // TODO eventually move to a size + blob architecture, if necessary,
                    // since these reallocs may be expensive
                    switch (msg.size())
                    {
                        case 1:
                            tmp8 = *static_cast<uint8_t*>(msg.data());
                            data.push_back(tmp8);
                            offset += 1;
                            break;

                        case 2:
                            data.resize(data.size() + 2);
                            tmp16 = *static_cast<uint16_t*>(msg.data());
                            *(reinterpret_cast<uint16_t*>(data.data() + offset)) = tmp16;
                            offset += 2;
                            break;

                        case 4:
                            data.resize(data.size() + 4);
                            tmp32 = *static_cast<uint32_t*>(msg.data());
                            *(reinterpret_cast<uint32_t*>(data.data() + offset)) = tmp32;
                            offset += 4;
                            break;

                        case 8:
                            data.resize(data.size() + 8);
                            tmp64 = *static_cast<uint64_t*>(msg.data());
                            *(reinterpret_cast<uint64_t*>(data.data() + offset)) = tmp64;
                            offset += 8;
                            break;

                        default:
                            throw std::runtime_error("received unsupported zmq message size");
                            break;
                    }

#ifdef WITH_LTTNG
                    msgPartCount++;
#endif
                    // check for more multipart messages
                    subscriber->getsockopt(ZMQ_RCVMORE, &rcvMore, &moreSize);
                }

#ifdef WITH_LTTNG
                tracepoint(lager_tp, lager_tp_zmq_msg, "Mug::subscriberThread()", "msg_rcvd",
                           msgPartCount, data.size());
#endif

                // write out the data
                // TODO this should probably be some kind of callback function
                keg->write(data, data.size());
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
            if (subscriber)
            {
                subscriber->close();
            }

            mutex.lock();
            subscriberRunning = false;
            mutex.unlock();
            return;
        }
    }

    try
    {
        subscriber->close();
    }
    catch (const zmq::error_t& e)
    {
        if (e.num() != ETERM)
        {
            std::stringstream ss;
            ss << "Mug::subscriberThread() uncaught zmq exception: " << e.what();
            throw std::runtime_error(ss.str());
        }
    }

    mutex.lock();
    subscriberRunning = false;
    mutex.unlock();
}
