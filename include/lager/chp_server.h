#ifndef CHP_SERVER
#define CHP_SERVER

#include <condition_variable>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <sstream>
#include <thread>

#ifdef __linux__
#include <unistd.h>
#endif

#include <zmq.hpp>

#include "lager_defines.h"
#include "lager/lager_utils.h"

class ClusteredHashmapServer final
{
public:
    explicit ClusteredHashmapServer(int basePort);

    void init(std::shared_ptr<zmq::context_t> context_in);
    void addOrUpdateKeyValue(const std::string& key, const std::string& value);
    void removeKey(const std::string& key);
    void start();
    void stop();
    std::map<std::string, std::string> getHashMap() {return hashMap;}

private:
    void snapshotThread();
    void snapshotMap(const std::string& identity);
    void snapshotBye(const std::string& identity);
    void publisherThread();
    void publishUpdatedKeys();
    void publishHugz();
    void collectorThread();
    void snapshotStopped();
    void initialize(std::shared_ptr<zmq::context_t> context_in);

    std::shared_ptr<zmq::context_t> context;
    std::shared_ptr<zmq::socket_t> snapshot;
    std::shared_ptr<zmq::socket_t> publisher;
    std::shared_ptr<zmq::socket_t> collector;

    std::thread publisherThreadHandle;
    std::thread snapshotThreadHandle;
    std::thread collectorThreadHandle;
    std::condition_variable publisherCv;
    std::condition_variable snapshotCv;
    std::condition_variable collectorCv;
    std::mutex mutex;

    std::map<std::string, std::string> hashMap; // <topic name, xml format>
    std::map<std::string, std::string> uuidMap; // <uuid, topic name>
    std::vector<std::string> updatedKeys; // <topic name>

    int snapshotPort;
    int publisherPort;
    int collectorPort;

    double sequence;

    bool publisherRunning;
    bool snapshotRunning;
    bool collectorRunning;
    bool running;
    bool initialized;
};

#endif
