#ifndef TAP
#define TAP

#include <future>
#include <memory>
#include <vector>

#include "chp_client.h"
#include "data_format.h"
#include "data_format_parser.h"

class Tap
{
public:
    Tap();

    void init(const std::string& serverHost_in, int basePort);
    void start(const std::string& key_in, const std::string& formatStr_in, bool isFile = true);
    void stop();
    void log();

private:
    void publisherThread();
    void initData();
    void updateData();

    std::shared_ptr<ChpClient> chpClient;
    std::shared_ptr<zmq::context_t> context;
    std::thread publisherThreadHandle;
    std::mutex mutex;
    std::condition_variable publisherCv;

    std::shared_ptr<DataFormat> format;
    std::vector<uint8_t> payload;

    std::string uuid;
    std::string key;
    std::string formatStr;
    std::string version;
    std::string serverHost;

    uint64_t timestamp;

    unsigned int payloadSize;
    unsigned int useCompression;

    int publisherPort;

    bool newData;
    bool running;
    bool publisherRunning;
};

#endif
