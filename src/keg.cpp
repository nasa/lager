#include "keg.h"

Keg::Keg(const std::string& baseDir_in): baseDir(baseDir_in), version(1), running(false)
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

    uint16_t emptyVersion = 0;
    uint64_t emptyOffset = 0;

    logFile.open(logFileName.c_str(), std::ios::out | std::ios::binary);
    logFile.write((char*)&emptyVersion, sizeof(uint16_t));
    logFile.write((char*)&emptyOffset, sizeof(uint64_t));
    running = true;
}

void Keg::stop()
{
    running = false;
    writeHeaderAndFormats();
    logFile.close();
}

void Keg::write(const std::vector<uint8_t>& data, size_t size)
{
    // TODO check for file open
    if (running)
    {
        logFile.write((char*)&data[0], size);
    }
}

void Keg::writeHeaderAndFormats()
{
    uint64_t pos = logFile.tellp();

    // grab network order of the items we need to write
    uint64_t posN = lager_utils::htonll(pos);
    uint16_t versionN = htons(version);

    // seek back to beginning of file to write the version and offset
    logFile.seekp(0);

    logFile.write((char*)&versionN, sizeof(uint16_t));
    logFile.write((char*)&posN, sizeof(uint64_t));

    // seek the file back to where it needs to be
    logFile.seekp(pos);

    // write the formats
    std::string formatStr = getFormatString();
    logFile.write(reinterpret_cast<const char*>(formatStr.c_str()), formatStr.length());
}

std::string Keg::getFormatString()
{
    DataFormatParser p;

    if (p.createFromUuidMap(formatMap))
    {
        return p.getXmlStr();
    }

    return "<invalid_xml>unimplemented format</invalid_xml>";
}
