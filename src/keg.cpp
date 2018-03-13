#include "lager/keg.h"

/**
 * @brief Keg constructor
 * @param baseDir_in is a string containing the path to an accessible directory to store the files in
 * @throws runtime_error when directory is non-existent or inaccessible
 */
Keg::Keg(const std::string& baseDir_in): baseDir(baseDir_in), version(1), running(false)
{
    if (!keg_utils::isDir(baseDir))
    {
        std::stringstream ss;
        ss << "unable to access " << baseDir;
        throw std::runtime_error(ss.str());
    }
}

/**
 * @brief Adds the given data format to the format uuid map
 * @param uuid is a string containing the 16 byte uuid of the data format
 * @param formatStr is a string containing the xml of the data format
 * @throws runtime_error on duplicate uuid
 */
void Keg::addFormat(const std::string& uuid, const std::string& formatStr)
{
    if (formatMap.count(uuid))
    {
        if (formatMap[uuid] != formatStr)
        {
            throw std::runtime_error("attempted to add duplicate uuid with different format to formatMap");
        }
    }

    formatMap[uuid] = formatStr;

    writeFormatFile();
}

/**
 * @brief Adds the given key, value pair to the metadata set for the keg
 * @param key
 * @param value
 */
void Keg::setMetaData(const std::string& key, const std::string& value)
{
    metaMap[key] = value;
}

/**
 * @brief Opens the keg's output file and writes the initially blank version and offset values
 * @throws runtime_error if keg is already running
 */
void Keg::start()
{
    if (running)
    {
        throw std::runtime_error("attempted to start a keg that's already running");
    }

    std::stringstream ss;
    ss << baseDir << "/" << lager_utils::getCurrentTimeFormatted("%Y%m%d_%H%M%S") << ".lgr";
    logFileName = ss.str();
    ss << ".format";
    formatFileName = ss.str();

    // TODO check for file open

    uint16_t emptyVersion = 0;
    uint64_t emptyOffset = 0;

    logFile.open(logFileName.c_str(), std::ios::out | std::ios::binary);
    logFile.write((char*)&emptyVersion, sizeof(emptyVersion));
    logFile.write((char*)&emptyOffset, sizeof(emptyOffset));

    formatFile.open(formatFileName.c_str(), std::ios::out | std::ios::binary);
    running = true;
}

/**
 * @brief Calls the update to the file header and closes the file
 * @throws runtime_error if keg is not running
 */
void Keg::stop()
{
    if (!running)
    {
        throw std::runtime_error("attempted to stop a keg that's not running");
    }

    running = false;

    writeFormatsAndHeader();
    logFile.close();
    formatFile.close();

    // we no longer need the formats file
    std::remove(formatFileName.c_str());
}

/**
 * @brief Writes one "row" of data to the file
 * @param data is an array of bytes to write to the file (which should already contain the correct header)
 * @param size is the size of the given array
 */
void Keg::write(const std::vector<uint8_t>& data, size_t size)
{
    // TODO check for file open
    if (running)
    {
        logFile.write((char*)&data[0], size);
    }
}

/**
 * @brief Updates the file header with the finalized version and format offsets
 */
void Keg::writeFormatsAndHeader()
{
    uint64_t pos = logFile.tellp();

    // grab network order of the items we need to write
    uint64_t posN = lager_utils::htonll(pos);
    uint16_t versionN = htons(version);

    // write the formats
    std::string formatStr = getFormatString();
    logFile.write(reinterpret_cast<const char*>(formatStr.c_str()), formatStr.length());

    // seek back to beginning of file to write the version and offset
    logFile.seekp(0);

    logFile.write((char*)&versionN, sizeof(versionN));
    logFile.write((char*)&posN, sizeof(posN));
}

/**
 * @brief Writes formats to intermediate file in case of bad exit
 */
void Keg::writeFormatFile()
{
    std::string formatStr = getFormatString();
    formatFile.write(reinterpret_cast<const char*>(formatStr.c_str()), formatStr.length());
    formatFile.flush();
}

/**
 * @brief Helper function which calls the xml generator function to get the final xml format string
 * @throws runtime_error if xml generator fails
 */
std::string Keg::getFormatString()
{
    DataFormatParser p;

    if (p.createFromUuidMap(formatMap, metaMap))
    {
        return p.getXmlStr();
    }
    else
    {
        throw std::runtime_error("Unable to generate XML format for Keg");
    }
}
