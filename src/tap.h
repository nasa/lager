#ifndef TAP
#define TAP

#include <future>
#include <memory>
#include <vector>

#include "chp_client.h"
#include "data_format_parser.h"
#include "data_ref_item.h"

class Tap
{
public:
    Tap();
    virtual ~Tap();

    bool init(const std::string& serverHost_in, int basePort);
    void addItem(AbstractDataRefItem* item);
    void start(const std::string& key_in);
    void stop();
    void log();

private:
    void publisherThread();

    std::shared_ptr<ClusteredHashmapClient> chpClient;
    std::shared_ptr<zmq::context_t> context;
    std::thread publisherThreadHandle;
    std::condition_variable publisherCv;
    std::mutex mutex;

    std::vector<AbstractDataRefItem*> dataRefItems;

    std::string uuid;
    std::string key;
    std::string formatStr;
    std::string version;
    std::string serverHost;

    uint64_t timestamp;
    uint8_t flags;

    int publisherPort;
    uint32_t offsetCount;

    bool newData;
    bool running;
    bool publisherRunning;

    // demo
    uint32_t int1;
    uint32_t int2;
};

#endif
