#ifndef KEG
#define KEG

#include <cstdio>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <sys/stat.h>

#include "data_format_parser.h"
#include "lager/keg_utils.h"
#include "lager/lager_utils.h"

/**
 * @brief Object used to write lager data to non-volatile storage
 */
class Keg
{
public:
    explicit Keg(const std::string& baseDir_in);
    void start();
    void stop();
    void write(const std::vector<uint8_t>& data, size_t size);
    void addFormat(const std::string& uuid, const std::string& formatStr);
    void setMetaData(const std::string& key, const std::string& value);
    const std::string getLogFile() { return logFileName; }
    const std::string getFormatFile() { return formatFileName; }

protected:
    void writeFormatsAndHeader();
    void writeFormatFile();
    std::string getFormatString();
    std::fstream logFile;
    std::fstream formatFile;

    std::map<std::string, std::string> formatMap; // <uuid, format xml>
    std::map<std::string, std::string> metaMap; // <key, value>
    std::vector<uint8_t> data;

    std::string logFileName;
    std::string formatFileName;
    std::string baseDir;

    uint16_t version;
    bool running;
};

#endif
