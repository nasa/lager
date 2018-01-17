#include <iostream>

#include <xercesc/dom/DOM.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>

#include "data_format.h"

using namespace xercesc;

class CErrorHandler : public xercesc::ErrorHandler
{
public:
    /** Warning message method */
    void warning(const xercesc::SAXParseException& ex);
    /** Error message method */
    void error(const xercesc::SAXParseException& ex);
    /** Fatal error message method */
    void fatalError(const xercesc::SAXParseException& ex);
    /** Errors resetter method */
    void resetErrors();
private:
    /** Based message reporter method */
    void reportParseException(const xercesc::SAXParseException& ex);
};
void CErrorHandler::reportParseException(const xercesc::SAXParseException& ex)
{
    char* message = xercesc::XMLString::transcode(ex.getMessage());
    std::cout << message << " at line " << ex.getLineNumber() << " column " << ex.getColumnNumber() << std::endl;

    xercesc::XMLString::release(&message);
}

void CErrorHandler::warning(const xercesc::SAXParseException& ex)
{
    reportParseException(ex);
}

void CErrorHandler::error(const xercesc::SAXParseException& ex)
{
    reportParseException(ex);
}

void CErrorHandler::fatalError(const xercesc::SAXParseException& ex)
{
    reportParseException(ex);
}

void CErrorHandler::resetErrors()
{
}

int main(int argc, char* argv[])
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
        return 1;
    }

    std::string xmlFile("sample_format.xml");
    std::string xsdFile("data_format.xsd");

    XercesDOMParser* parser = new XercesDOMParser();
    ErrorHandler* errHandler = (ErrorHandler*) new CErrorHandler();

    parser->loadGrammar(xsdFile.c_str(), Grammar::SchemaGrammarType);
    parser->setErrorHandler(errHandler);
    parser->setValidationScheme(XercesDOMParser::Val_Always);
    parser->setDoNamespaces(true);
    parser->setDoSchema(true);
    parser->setExternalNoNamespaceSchemaLocation(xsdFile.c_str());

    try
    {
        parser->parse(xmlFile.c_str());

        if (parser->getErrorCount() != 0)
        {
            std::cout << "schema error" << std::endl;
        }

        // DOMDocument* doc = parser->getDocument();
        // DOMElement* root = doc->getDocumentElement();
    }
    catch (const XMLException& e)
    {
        char* message = XMLString::transcode(e.getMessage());
        std::cout << "xml exception: " << message << std::endl;
        XMLString::release(&message);
        return -1;
    }
    catch (const DOMException& e)
    {
        char* message = XMLString::transcode(e.msg);
        std::cout << "dom exception: " << message << std::endl;
        XMLString::release(&message);
        return -1;
    }
    catch (const SAXParseException& e)
    {
        char* message = XMLString::transcode(e.getMessage());
        std::cout << "sax exception: " << message << std::endl;
        XMLString::release(&message);
        return -1;
    }
    catch (const std::runtime_error& e)
    {
        std::cout << "uncaught exception: " << e.what() << std::endl;
        return -1;
    }

    delete parser;
    delete errHandler;

    return 0;
}
