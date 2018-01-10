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
    void publish(int data);

    std::shared_ptr<ChpClient> chpClient;
    std::shared_ptr<zmq::context_t> context;
    std::shared_ptr<zmq::socket_t> publisher;

    std::string uuid;

    std::vector<uint8_t> uuidBytes;

    int publisherPort;

    bool running;
};

#endif
