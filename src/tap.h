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
    void start(const std::string& dataKey_in, const std::string& dataFormatStr_in, bool isFile = true);
    void stop();
    void log(int data);

private:
    void publisherThread();

    std::shared_ptr<ChpClient> chpClient;
    std::shared_ptr<zmq::context_t> context;
    std::thread publisherThreadHandle;
    std::mutex mutex;
    std::condition_variable publisherCv;

    std::shared_ptr<DataFormat> dataFormat;

    std::string uuid;
    std::string dataKey;
    std::string dataFormatStr;
    std::string serverHost;

    int publisherPort;
    int theInt;

    bool newData;
    bool running;
    bool publisherRunning;
};

#endif
