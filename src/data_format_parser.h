#ifndef DATA_FORMAT_PARSER
#define DATA_FORMAT_PARSER

#include <fstream>
#include <memory>
#include <string>

#include <xercesc/dom/DOM.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>

#include "data_format.h"

using namespace xercesc;

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

class DataFormatParser
{
public:
    DataFormatParser();
    DataFormatParser(const std::string& xsdFile_in);
    ~DataFormatParser();

    std::string getXmlStr() {return xmlStr;};

    std::shared_ptr<DataFormat> parseFromFile(const std::string& xmlFile);
    std::shared_ptr<DataFormat> parseFromString(const std::string& xmlStr_in);

private:
    void parse();

    XercesDOMParser* parser;
    XercesErrorHandler* errHandler;

    XMLCh* tagFormat;
    XMLCh* tagItem;
    XMLCh* attVersion;
    XMLCh* attName;
    XMLCh* attType;
    XMLCh* attSize;
    XMLCh* attOffset;

    std::shared_ptr<DataFormat> format;

    std::string xmlStr;
    std::string xsdFile;
};

#endif
