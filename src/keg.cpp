#include "keg.h"

Keg::Keg(const std::string& baseDir_in): baseDir(baseDir_in)
{
    if (!keg_utils::isDir(baseDir))
    {
        std::stringstream ss;
        ss << "unable to access " << baseDir;
        throw std::runtime_error(ss.str());
    }
}

void Keg::addFormat(const std::string& uuid, const std::string& formatStr)
{
    formatMap[uuid] = formatStr;
}

void Keg::start()
{
    std::stringstream ss;
    ss << baseDir << "/" << lager_utils::getCurrentTime() << ".lgr";
    logFileName = ss.str();

    // TODO check for file open

    logFile.open(logFileName.c_str(), std::ios::out | std::ios::binary);
}

void Keg::stop()
{
    // TODO write the formats and update the header

    logFile.close();
}

void Keg::write(const std::vector<uint8_t>& data, size_t size)
{
    // TODO check for file open

    logFile.write((char*)&data[0], size);
}
