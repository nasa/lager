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

void XercesErrorHandler::warning(const xercesc::SAXParseException& ex)
{
    reportParseException(ex);
}

void XercesErrorHandler::error(const xercesc::SAXParseException& ex)
{
    reportParseException(ex);
}

void XercesErrorHandler::fatalError(const xercesc::SAXParseException& ex)
{
    reportParseException(ex);
}

void XercesErrorHandler::resetErrors()
{
}

DataFormatParser::DataFormatParser(const std::string& xmlFile_in, const std::string& xsdFile_in)
{
    try
    {
        XMLPlatformUtils::Initialize();
    }
    catch (const XMLException& e)
    {
        char* message = XMLString::transcode(e.getMessage());
        std::cout << "error initializing xercesc: " << message << std::endl;
        XMLString::release(&message);
        return;
    }

    xmlFile = xmlFile_in;
    xsdFile = xsdFile_in;

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

    try
    {
        XMLString::release(&tagFormat);
        XMLString::release(&tagItem);
        XMLString::release(&attVersion);
        XMLString::release(&attName);
        XMLString::release(&attType);
        XMLString::release(&attSize);
        XMLString::release(&attOffset);
    }
    catch (...)
    {
        std::cout << "error destructing XMLStrings" << std::endl;
    }

    try
    {
        XMLPlatformUtils::Terminate();
    }
    catch (const XMLException& e)
    {
        char* message = XMLString::transcode(e.getMessage());
        std::cout << "destructor exception: " << message << std::endl;
        XMLString::release(&message);
    }
}

std::shared_ptr<DataFormat> DataFormatParser::parse()
{
    std::shared_ptr<DataFormat> format;

    parser->loadGrammar(xsdFile.c_str(), Grammar::SchemaGrammarType);
    parser->setErrorHandler(errHandler);
    parser->setValidationScheme(XercesDOMParser::Val_Always);
    parser->setDoNamespaces(true);
    parser->setDoSchema(true);
    parser->setExternalNoNamespaceSchemaLocation(xsdFile.c_str());

    try
    {
        parser->parse(xmlFile.c_str());

        // because we passed in the XSD file, this error count will tell us if
        // the xml file is valid per the schema
        if (parser->getErrorCount() != 0)
        {
            std::cout << "error in " << xmlFile << ": " << errHandler->getLastError() << std::endl;
            throw std::runtime_error("");
        }

        DOMDocument* doc = parser->getDocument();
        DOMElement* formatElement = doc->getDocumentElement();

        if (!formatElement)
        {
            std::cout << "no format element at the root of the document" << std::endl;
            throw std::runtime_error("");
        }

        if (!XMLString::equals(formatElement->getTagName(), tagFormat))
        {
            std::cout << "root element is not format as expected" << std::endl;
            throw std::runtime_error("");
        }

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
    catch (const XMLException& e)
    {
        char* message = XMLString::transcode(e.getMessage());
        std::cout << "xml exception: " << message << std::endl;
        XMLString::release(&message);
        throw std::runtime_error("");
    }
    catch (const DOMException& e)
    {
        char* message = XMLString::transcode(e.msg);
        std::cout << "dom exception: " << message << std::endl;
        XMLString::release(&message);
        throw std::runtime_error("");
    }
    catch (const SAXParseException& e)
    {
        char* message = XMLString::transcode(e.getMessage());
        std::cout << "sax exception: " << message << std::endl;
        XMLString::release(&message);
        throw std::runtime_error("");
    }
    catch (const std::runtime_error& e)
    {
        std::cout << "uncaught exception: " << e.what() << std::endl;
        throw std::runtime_error("");
    }

    return format;
}
