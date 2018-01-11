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

#include "lager_utils.h"

class ChpClient
{
public:
    ChpClient(const std::string& serverHost_in, int basePort, int timeoutMillis_in);

    void init(std::shared_ptr<zmq::context_t> context_in, const std::string& uuid);
    void addOrUpdateKeyValue(const std::string& key, const std::string& value);
    void removeKey(const std::string& key);
    void start();
    void stop();

private:
    void snapshotThread();
    void subscriberThread();
    void publisherThread(const std::string& key, const std::string& value);
    void initialize(std::shared_ptr<zmq::context_t> context_in);

    std::shared_ptr<zmq::context_t> context;
    std::shared_ptr<zmq::socket_t> snapshot;
    std::shared_ptr<zmq::socket_t> subscriber;
    std::shared_ptr<zmq::socket_t> publisher;

    std::thread snapshotThreadHandle;
    std::thread subscriberThreadHandle;
    std::thread publisherThreadHandle;
    std::condition_variable snapshotCv;
    std::condition_variable subscriberCv;
    std::mutex mutex;

    std::map<std::string, std::string> hashMap;

    std::string uuid;
    std::string serverHost;

    int snapshotPort;
    int subscriberPort;
    int publisherPort;
    int timeoutMillis;

    double sequence;

    bool running;
    bool snapshotRunning;
    bool subscriberRunning;
};

#endif
