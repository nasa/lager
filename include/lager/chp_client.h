#ifndef CHP_CLIENT
#define CHP_CLIENT

#include <future>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <thread>

#ifdef _WIN32
#pragma comment(lib, "rpcrt4.lib")
#else
#include <unistd.h>
#endif

#include <zmq.hpp>

#include "lager_defines.h"
#include "lager/lager_utils.h"

/**
 * @brief Client implementation of ZMQ Clustered Hashmap Protocol
 * https://rfc.zeromq.org/spec:12/CHP/
 */
class ClusteredHashmapClient final
{
public:
    ClusteredHashmapClient(const std::string& serverHost_in, int basePort, int timeoutMillis_in);

    void init(std::shared_ptr<zmq::context_t> context_in, const std::string& uuid);
    void addOrUpdateKeyValue(const std::string& key, const std::string& value);
    void removeKey(const std::string& key);
    void setCallback(const std::function<void()>& func);
    void start();
    void stop();
    bool isTimedOut() {return timedOut;};
    std::map<std::string, std::string> getHashMap() {return hashMap;}
    std::map<std::string, std::string> getUuidMap() {return uuidMap;}

private:
    void snapshotThread();
    void subscriberThread();
    void publisherThread();
    void initialize(std::shared_ptr<zmq::context_t> context_in);
    std::map<std::string, std::string> getUnsyncedHashmap();

    std::shared_ptr<zmq::context_t> context;
    std::shared_ptr<zmq::socket_t> snapshot;
    std::shared_ptr<zmq::socket_t> subscriber;
    std::shared_ptr<zmq::socket_t> publisher;
    std::function<void()> hashMapUpdated;

    std::thread snapshotThreadHandle;
    std::thread subscriberThreadHandle;
    std::thread publisherThreadHandle;
    std::condition_variable cv;
    std::mutex mutex;

    std::map<std::string, std::string> hashMap; // <topic name, xml format>
    std::map<std::string, std::string> uuidMap; // <uuid, topic name>
    std::map<std::string, std::string> selfMap; // <topic name, xml format>

    std::string uuid;
    std::string serverHost;

    int snapshotPort;
    int subscriberPort;
    int publisherPort;
    int timeoutMillis;

    double sequence;

    bool initialized;
    bool running;
    bool timedOut;
    bool snapshotRunning;
    bool subscriberRunning;
    bool publisherRunning;
};

#endif
