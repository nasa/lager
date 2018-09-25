#ifndef TAP
#define TAP

#include <future>
#include <memory>
#include <vector>

#include "chp_client.h"
#include "data_format_parser.h"

/**
* @brief The data source object for the lager system
*/
class Tap
{
public:
    Tap();
    virtual ~Tap();

    bool init(const std::string& serverHost_in, int basePort, int timeOutMillis);
    void addItem(AbstractDataRefItem* item);
    std::vector<AbstractDataRefItem*> getItems() const;
    void start(const std::string& key_in);
    void stop();
    void log();
    uint8_t getFlag();
    void setFlag(uint8_t setFlag);

protected:
    void publisherThread();

    std::shared_ptr<ClusteredHashmapClient> chpClient;
    std::shared_ptr<zmq::context_t> context;
    std::thread publisherThreadHandle;
    std::condition_variable cv;
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
    off_t offsetCount;

    bool newData;
    bool running;
    bool publisherRunning;
};

#endif
