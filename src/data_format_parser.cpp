#include <sstream>

#include "data_format_parser.h"

void XercesErrorHandler::reportParseException(const xercesc::SAXParseException& ex)
{
    char* message = xercesc::XMLString::transcode(ex.getMessage());
    std::stringstream ss;
    ss << message << " at line " << ex.getLineNumber() << " column " << ex.getColumnNumber();
    lastError = ss.str();
    xercesc::XMLString::release(&message);
}

void XercesErrorHandler::warning(const xercesc::SAXParseException& ex) {}

void XercesErrorHandler::error(const xercesc::SAXParseException& ex)
{
    reportParseException(ex);
}

void XercesErrorHandler::fatalError(const xercesc::SAXParseException& ex)
{
    reportParseException(ex);
}

// defaulting to a standard xsd location, prob should formalize this somehow
DataFormatParser::DataFormatParser() : DataFormatParser("data_format.xsd")
{
}

// note file path must be full path or same directory.  relative paths don't work
// TODO documentation: throws
DataFormatParser::DataFormatParser(const std::string& xsdFile_in)
{
    XMLPlatformUtils::Initialize();

    xsdFile = xsdFile_in;

    std::ifstream f(xsdFile);

    if (!f.good())
    {
        std::stringstream ss;
        ss << "failed to load schema file: " << xsdFile;
        throw std::runtime_error(ss.str());
    }

    tagFormat = XMLString::transcode("format");
    tagItem = XMLString::transcode("item");
    attVersion = XMLString::transcode("version");
    attName = XMLString::transcode("name");
    attType = XMLString::transcode("type");
    attSize = XMLString::transcode("size");
    attOffset = XMLString::transcode("offset");

    parser = new XercesDOMParser;
    errHandler = new XercesErrorHandler;
}

DataFormatParser::~DataFormatParser()
{
    if (parser != NULL)
    {
        delete parser;
        delete errHandler;
    }

    XMLString::release(&tagFormat);
    XMLString::release(&tagItem);
    XMLString::release(&attVersion);
    XMLString::release(&attName);
    XMLString::release(&attType);
    XMLString::release(&attSize);
    XMLString::release(&attOffset);

    XMLPlatformUtils::Terminate();
}

// TODO documentation: throws
std::shared_ptr<DataFormat> DataFormatParser::parseFromFile(const std::string& xmlFile)
{
    std::ifstream f(xmlFile);

    if (!f.good())
    {
        std::stringstream ss;
        ss << "failed to load xml file: " << xmlFile;
        throw std::runtime_error(ss.str());
    }

    xmlStr = std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

    parse();
    return format;
}

// TODO documentation: throws via parse()
std::shared_ptr<DataFormat> DataFormatParser::parseFromString(const std::string& xmlStr_in)
{
    xmlStr = xmlStr_in;
    parse();
    return format;
}

void DataFormatParser::parse()
{
    parser->loadGrammar(xsdFile.c_str(), Grammar::SchemaGrammarType);
    parser->setErrorHandler(errHandler);
    parser->setValidationScheme(XercesDOMParser::Val_Always);
    parser->setDoNamespaces(true);
    parser->setDoSchema(true);
    parser->setExternalNoNamespaceSchemaLocation(xsdFile.c_str());

    try
    {
        MemBufInputSource xmlBuf((const XMLByte*)xmlStr.c_str(), xmlStr.size(), "unused");

        parser->parse(xmlBuf);

        // because we passed in the XSD file, this error count will tell us if
        // the xml file is valid per the schema
        if (parser->getErrorCount() != 0)
        {
            std::stringstream ss;
            ss << "error in xml: " << errHandler->getLastError();
            errHandler->resetErrors();
            throw std::runtime_error(ss.str());
        }

        xercesc::DOMDocument* doc = parser->getDocument();
        DOMElement* formatElement = doc->getDocumentElement();

        const XMLCh* xVersion = formatElement->getAttribute(attVersion);
        std::string version(XMLString::transcode(xVersion));

        format.reset(new DataFormat(version));

        DOMNodeList* children = formatElement->getChildNodes();

        for (XMLSize_t i = 0; i < children->getLength(); ++i)
        {
            DOMNode* node = children->item(i);

            if (node->getNodeType() && node->getNodeType() == DOMNode::ELEMENT_NODE)
            {
                DOMElement* nodeElement = dynamic_cast<DOMElement*>(node);

                if (XMLString::equals(nodeElement->getTagName(), tagItem))
                {
                    const XMLCh* xName = nodeElement->getAttribute(attName);
                    const XMLCh* xType = nodeElement->getAttribute(attType);
                    const XMLCh* xSize = nodeElement->getAttribute(attSize);
                    const XMLCh* xOffset = nodeElement->getAttribute(attOffset);

                    unsigned int size;
                    unsigned int offset;

                    std::istringstream issSize(XMLString::transcode(xSize));
                    issSize >> size;

                    std::istringstream issOffset(XMLString::transcode(xOffset));
                    issOffset >> offset;

                    format->addItem(DataItem(std::string(XMLString::transcode(xName)),
                                             std::string(XMLString::transcode(xType)),
                                             size, offset));
                }
            }
        }
    }
    catch (const std::runtime_error& e)
    {
        throw std::runtime_error(e.what());
    }
}

bool DataFormatParser::createFromDataRefItems(const std::vector<AbstractDataRefItem*>& items, const std::string& version)
{
    std::stringstream ss;

    XMLCh* xTempStr = nullptr;
    XMLCh* xVersion = nullptr;
    XMLCh* xName = nullptr;
    XMLCh* xType = nullptr;
    XMLCh* xSize = nullptr;
    XMLCh* xOffset = nullptr;

    xTempStr = XMLString::transcode("Range");
    DOMImplementation* impl = DOMImplementationRegistry::getDOMImplementation(xTempStr);

    if (!impl)
    {
        return false;
    }

    xTempStr = XMLString::transcode("format");
    xercesc::DOMDocument* doc = impl->createDocument(nullptr, xTempStr, nullptr);
    DOMElement* root = doc->getDocumentElement();

    xVersion = XMLString::transcode(version.c_str());
    root->setAttribute(attVersion, xVersion);

    for (auto i = items.begin(); i != items.end(); ++i)
    {
        uint32_t size = (*i)->getSize();
        uint32_t offset = (*i)->getOffset();

        ss.str(std::string());
        ss << size;
        xSize = XMLString::transcode(ss.str().c_str());

        ss.str(std::string());
        ss << offset;
        xOffset = XMLString::transcode(ss.str().c_str());

        xName = XMLString::transcode((*i)->getName().c_str());
        xType = XMLString::transcode((*i)->getType().c_str());

        DOMElement* item = doc->createElement(tagItem);
        item->setAttribute(attName, xName);
        item->setAttribute(attType, xType);
        item->setAttribute(attSize, xSize);
        item->setAttribute(attOffset, xOffset);
        root->appendChild(item);
    }

    xmlStr = getStringFromDoc(doc);

    XMLString::release(&xTempStr);
    XMLString::release(&xVersion);
    XMLString::release(&xName);
    XMLString::release(&xType);
    XMLString::release(&xSize);
    XMLString::release(&xOffset);

    if (!isValid(xmlStr, items.size()))
    {
        return false;
    }

    return true;
}

std::string DataFormatParser::getStringFromDoc(xercesc::DOMDocument* doc)
{
    DOMImplementation* pImplement = nullptr;
    DOMLSSerializer* pSerializer = nullptr;
    MemBufFormatTarget* pTarget = nullptr;

    XMLCh* xTempStr;
    xTempStr = XMLString::transcode("LS");
    pImplement = DOMImplementationRegistry::getDOMImplementation(xTempStr);
    pSerializer = ((DOMImplementationLS*)pImplement)->createLSSerializer();
    pTarget = new MemBufFormatTarget();
    DOMLSOutput* pDomLsOutput = ((DOMImplementationLS*)pImplement)->createLSOutput();
    pDomLsOutput->setByteStream(pTarget);

    pSerializer->write(doc, pDomLsOutput);

    std::string xmlOutput((char*)pTarget->getRawBuffer(), pTarget->getLen());

    XMLString::release(&xTempStr);

    return xmlOutput;
}

// TODO documentation: throws via parseFromString()
bool DataFormatParser::isValid(const std::string& xml, unsigned int itemCount)
{
    std::shared_ptr<DataFormat> testFormat = parseFromString(xml);

    if (testFormat->getItemCount() != itemCount)
    {
        return false;
    }

    return true;
}
