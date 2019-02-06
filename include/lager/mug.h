#ifndef MUG
#define MUG

#include <future>
#include <map>
#include <memory>
#include <thread>
#include <vector>

#include "chp_client.h"
#include "data_format_parser.h"
#include "lager/keg.h"

/**
* @brief The data sink object for the lager system
*/
class Mug
{
public:
    Mug();
    virtual ~Mug();

    bool init(const std::string& serverHost_in, int basePort, int timeOutMillis,
              const std::string& kegDir = "./");
    void start();
    void stop();

protected:
    void subscriberThread();
    void hashMapUpdated();

    std::shared_ptr<Keg> keg;
    std::shared_ptr<ClusteredHashmapClient> chpClient;
    std::shared_ptr<zmq::context_t> context;
    std::shared_ptr<zmq::socket_t> subscriber;
    std::function<void()> hashMapUpdatedHandle;

    std::thread subscriberThreadHandle;
    std::mutex mutex;

    std::map<std::string, std::string> hashMap; // <topic name, xml format>
    std::map<std::string, std::shared_ptr<DataFormat>> formatMap; // <uuid, dataformat>
    std::vector<std::string> subscribedList; // <topic name>
    std::shared_ptr<DataFormatParser> formatParser;

    std::string serverHost;
    std::string uuid;

    int subscriberPort;

    bool running;
    bool subscriberRunning;
};

#endif
