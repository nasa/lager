#ifndef TAP
#define TAP

#include <future>
#include <memory>
#include <vector>

#include "chp_client.h"

class Tap
{
public:
    Tap();

    void init(const std::string& serverHost_in, int basePort);
    void start();
    void stop();
    void log(int data);

private:
    void publisherThread();

    std::shared_ptr<ChpClient> chpClient;
    std::shared_ptr<zmq::context_t> context;
    std::thread publisherThreadHandle;
    std::mutex mutex;
    std::condition_variable publisherCv;

    std::string uuid;
    std::string serverHost;

    int publisherPort;
    int theInt;

    bool newData;
    bool running;
    bool publisherRunning;
};

#endif
