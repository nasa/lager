#ifndef DATA_FORMAT_PARSER
#define DATA_FORMAT_PARSER

#include <fstream>
#include <memory>
#include <sstream>
#include <stdint.h>
#include <string>
#include <vector>

#include <xercesc/dom/DOM.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/framework/MemBufFormatTarget.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/TranscodingException.hpp>
#include <xercesc/util/XMLString.hpp>

#include "data_format.h"
#include "data_ref_item.h"

using namespace xercesc;

/**
 * @brief Custom error handler for xercesc parsing
 */
class XercesErrorHandler : public xercesc::ErrorHandler
{
public:
    void warning(const xercesc::SAXParseException& ex);
    void error(const xercesc::SAXParseException& ex);
    void fatalError(const xercesc::SAXParseException& ex);
    void resetErrors() {};
    std::string getLastError() {return lastError;};
private:
    void reportParseException(const xercesc::SAXParseException& ex);
    std::string lastError;
};

/**
 * @brief A DOM based parser to help lager go from xml to structured formats of data to log
 */
class DataFormatParser
{
public:
    DataFormatParser();
    explicit DataFormatParser(const std::string& xsdFile_in);
    ~DataFormatParser();

    std::string getXmlStr() {return xmlStr;};

    std::shared_ptr<DataFormat> parseFromFile(const std::string& xmlFile);
    std::shared_ptr<DataFormat> parseFromString(const std::string& xmlStr_in);
    bool createFromDataRefItems(const std::vector<AbstractDataRefItem*>& items,
                                const std::string& version, const std::string& key);
    bool createFromUuidMap(const std::map<std::string, std::string>& uuidMap,
                           const std::map<std::string, std::string>& metaMap);
    bool isValid(const std::string& xml, unsigned int itemCount);

private:
    void parse();
    std::string getStringFromDoc(xercesc::DOMDocument* doc);

    XercesDOMParser* parser;
    XercesErrorHandler* errHandler;

    XMLCh* tagFormat;
    XMLCh* tagItem;
    XMLCh* tagFormats;
    XMLCh* tagMeta;
    XMLCh* tagMetaData;
    XMLCh* attVersion;
    XMLCh* attName;
    XMLCh* attType;
    XMLCh* attSize;
    XMLCh* attOffset;
    XMLCh* attUuid;
    XMLCh* attKey;
    XMLCh* attValue;

    std::shared_ptr<DataFormat> format;

    std::string xmlStr;
    std::string xsdFile;
};

#endif
