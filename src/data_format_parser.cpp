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

        // parser->parse(xmlFile.c_str());
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

        DOMDocument* doc = parser->getDocument();
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

bool DataFormatParser::createFromDataRefItems(std::vector<AbstractDataRefItem*> items)
{
    std::stringstream ss;

    XMLCh tempStr[100];
    XMLCh* xVersion;
    XMLCh* xName;
    XMLCh* xType;
    XMLCh* xSize;
    XMLCh* xOffset;

    XMLString::transcode("Range", tempStr, 99);
    DOMImplementation* impl = DOMImplementationRegistry::getDOMImplementation(tempStr);

    XMLString::transcode("format", tempStr, 99);
    DOMDocument* doc = impl->createDocument(0, tempStr, 0);
    DOMElement* root = doc->getDocumentElement();

    xVersion = XMLString::transcode("BEERR01");
    root->setAttribute(attVersion, xVersion);

    for (std::vector<AbstractDataRefItem*>::iterator i = items.begin(); i != items.end(); ++i)
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

    std::cout << getStringFromDoc(doc) << std::endl;;

    return true;
}

std::string DataFormatParser::getStringFromDoc(DOMDocument* doc)
{
    DOMImplementation* pImplement = NULL;
    DOMLSSerializer* pSerializer = NULL;
    XMLFormatTarget* pTarget = NULL;

    XMLCh tempStr[100];
    XMLString::transcode("LS", tempStr, 99);
    pImplement = DOMImplementationRegistry::getDOMImplementation(tempStr);
    pSerializer = ((DOMImplementationLS*)pImplement)->createLSSerializer();
    pTarget = new MemBufFormatTarget();
    DOMLSOutput* pDomLsOutput = ((DOMImplementationLS*)pImplement)->createLSOutput();
    pDomLsOutput->setByteStream(pTarget);

    pSerializer->write(doc, pDomLsOutput);

    std::string xmlOutput((char*)((MemBufFormatTarget*)pTarget)->getRawBuffer());

    return xmlOutput;
}
