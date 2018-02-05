#ifndef MUG
#define MUG

#include <future>
#include <map>
#include <memory>
#include <thread>
#include <vector>

#include "chp_client.h"
#include "data_format_parser.h"

class Mug
{
public:
    Mug();
    virtual ~Mug();

    bool init(const std::string& serverHost_in, int basePort);
    void start();
    void stop();

private:
    void subscriberThread();
    void hashMapUpdated();

    std::shared_ptr<ClusteredHashmapClient> chpClient;
    std::shared_ptr<zmq::context_t> context;
    std::shared_ptr<zmq::socket_t> subscriber;
    std::function<void()> hashMapUpdatedHandle;

    std::thread subscriberThreadHandle;
    std::mutex mutex;
    std::condition_variable subscriberCv;

    std::map<std::string, std::string> hashMap; // <topic name, xml format>
    std::map<std::string, std::shared_ptr<DataFormat>> formatMap; // <uuid, dataformat>
    std::map<std::string, std::vector<uint8_t>> dataMap; // <uuid, data>
    std::vector<std::string> subscribedList; // <topic name>
    std::shared_ptr<DataFormatParser> formatParser;

    std::string serverHost;
    std::string uuid;

    int subscriberPort;

    bool running;
    bool subscriberRunning;
};

#endif
