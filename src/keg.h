#ifndef KEG
#define KEG

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <sys/stat.h>

#include "data_format_parser.h"
#include "keg_utils.h"
#include "lager_utils.h"

class Keg
{
public:
    Keg(const std::string& baseDir_in);
    void start();
    void stop();
    void write(const std::vector<uint8_t>& data, size_t size);
    void addFormat(const std::string& uuid, const std::string& formatStr);

private:
    void writeHeaderAndFormats();
    std::string getFormatString();
    std::fstream logFile;

    std::map<std::string, std::string> formatMap; // <uuid, format xml>
    std::vector<uint8_t> data;

    std::string logFileName;
    std::string baseDir;

    uint16_t version;
    bool running;
};

#endif
