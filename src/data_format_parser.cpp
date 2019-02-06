#include "lager/data_format_parser.h"

/**
 * @brief Generates xml error string and stores in member lastError
 * @param ex is a SAXParseException to process (Xerces re-uses the SAX in the DOM parsing)
 */
void XercesErrorHandler::reportParseException(const xercesc::SAXParseException& ex)
{
    char* message = xercesc::XMLString::transcode(ex.getMessage());
    std::stringstream ss;
    ss << message << " at line " << ex.getLineNumber() << " column " << ex.getColumnNumber();
    lastError = ss.str();
    xercesc::XMLString::release(&message);
}

/**
 * @brief XML parser warnings, have not found any XML case yet that fires this function
 * @param ex is a SAXParseException to process
 */
void XercesErrorHandler::warning(const xercesc::SAXParseException& ex) {}

/**
 * @brief XML parser errors
 * @param ex is a SAXParseException to process
 */
void XercesErrorHandler::error(const xercesc::SAXParseException& ex)
{
    reportParseException(ex);
}

/**
 * @brief XML parser errors
 * @param ex is a SAXParseException to process
 */
void XercesErrorHandler::fatalError(const xercesc::SAXParseException& ex)
{
    reportParseException(ex);
}

/**
 * @brief Ctor with automatically filled in schema file
 */
// TODO should formalize this location somehow
DataFormatParser::DataFormatParser() : DataFormatParser("data_format.xsd")
{
}

/**
 * @brief Ctor with schema file parameter
 * @param xsdFile_in is a *full path* to a xsd schema file to use (relative path's don't work)
 * @throws runtime_error on inability to open file
 */
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
    tagFormats = XMLString::transcode("formats");
    tagMeta = XMLString::transcode("meta");
    tagMetaData = XMLString::transcode("metadata");
    tagItem = XMLString::transcode("item");
    attVersion = XMLString::transcode("version");
    attName = XMLString::transcode("name");
    attType = XMLString::transcode("type");
    attSize = XMLString::transcode("size");
    attOffset = XMLString::transcode("offset");
    attUuid = XMLString::transcode("uuid");
    attKey = XMLString::transcode("key");
    attValue = XMLString::transcode("value");

    parser = new XercesDOMParser;
    errHandler = new XercesErrorHandler;
}

/**
 * @brief Dtor, releases xerces releated items
 */
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
    XMLString::release(&tagFormats);
    XMLString::release(&tagMeta);
    XMLString::release(&tagMetaData);
    XMLString::release(&attUuid);
    XMLString::release(&attKey);
    XMLString::release(&attValue);

    XMLPlatformUtils::Terminate();
}

/**
 * @brief Parses a given file and returns a structure to the parsed DataFormat
 * @param xmlFile is a string with a path to an xml file to parse
 * @returns shared_ptr to a DataFormat containing the parsed data
 * @throws runtime_error on inability to open file or on bad parse or schema violation
 */
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

/**
 * @brief Parses a given xml string and returns a structure to the parsed DataFormat
 * @param xmlStr_in is a string with the xml data format to parse
 * @returns shared_ptr to a DataFormat containing the parsed data
 * @throws runtime_error on a bad parse or schema violation
 */
std::shared_ptr<DataFormat> DataFormatParser::parseFromString(const std::string& xmlStr_in)
{
    xmlStr = xmlStr_in;
    parse();
    return format;
}

/**
 * @brief Parses the xml contained in the member string xmlStr and stores into member format
 * @throws runtime_error on a bad parse or schema violation
 */
void DataFormatParser::parse()
{
    // use our custom error handler
    parser->setErrorHandler(errHandler);

    // loads the xsd schema file for use by the parser
    parser->loadGrammar(xsdFile.c_str(), Grammar::SchemaGrammarType);
    parser->setValidationScheme(XercesDOMParser::Val_Always);
    parser->setDoNamespaces(true);
    parser->setDoSchema(true);
    parser->setExternalNoNamespaceSchemaLocation(xsdFile.c_str());

    try
    {
        // create the buffer necessary to parse the xml string, 3rd param is unused
        MemBufInputSource xmlBuf((const XMLByte*)xmlStr.c_str(), xmlStr.size(), "unused");

        parser->parse(xmlBuf);

        // because we passed in the XSD file, this error count will tell us if
        // the xml file is valid per the schema
        if (parser->getErrorCount() != 0)
        {
            std::stringstream ss;

            // grab the detailed error from the handler
            ss << "error in xml: " << errHandler->getLastError();

            // now that we've gotten the error we can reset the error count in the handler
            // (not sure if this really means anything)
            errHandler->resetErrors();

            throw std::runtime_error(ss.str());
        }

        xercesc::DOMDocument* doc = parser->getDocument();
        DOMElement* formatElement = doc->getDocumentElement();

        // grab the version
        const XMLCh* xVersion = formatElement->getAttribute(attVersion);
        char* cVersion = XMLString::transcode(xVersion);
        std::string version(cVersion);

        // grab the key
        const XMLCh* xKey = formatElement->getAttribute(attKey);
        char* cKey = XMLString::transcode(xKey);
        std::string key(cKey);

        format.reset(new DataFormat(version, key));

        DOMNodeList* children = formatElement->getChildNodes();

        // iterate the child nodes, looking for item elements
        for (XMLSize_t i = 0; i < children->getLength(); ++i)
        {
            DOMNode* node = children->item(i);

            if (node->getNodeType() && node->getNodeType() == DOMNode::ELEMENT_NODE)
            {
                DOMElement* nodeElement = dynamic_cast<DOMElement*>(node);

                if (XMLString::equals(nodeElement->getTagName(), tagItem))
                {
                    // found an item element, now grab the attributes
                    const XMLCh* xName = nodeElement->getAttribute(attName);
                    const XMLCh* xType = nodeElement->getAttribute(attType);
                    const XMLCh* xSize = nodeElement->getAttribute(attSize);
                    const XMLCh* xOffset = nodeElement->getAttribute(attOffset);

                    // convert the numeric values to the needed types
                    size_t size;
                    off_t offset;

                    char* cSize = XMLString::transcode(xSize);
                    std::istringstream issSize(cSize);
                    issSize >> size;

                    char* cOffset = XMLString::transcode(xOffset);
                    std::istringstream issOffset(cOffset);
                    issOffset >> offset;

                    char* cName = XMLString::transcode(xName);
                    char* cType = XMLString::transcode(xType);

                    // add the item to the format
                    format->addItem(DataItem(std::string(cName), std::string(cType), size, offset));

                    // release resources
                    XMLString::release(&cName);
                    XMLString::release(&cType);
                    XMLString::release(&cSize);
                    XMLString::release(&cOffset);
                }
            }
        }

        XMLString::release(&cVersion);
        XMLString::release(&cKey);
    }
    catch (const std::runtime_error& e)
    {
        throw std::runtime_error(e.what());
    }
}

/**
 * @brief Converts a given array of DataRefItems and generates and stores its xml string into the xmlStr member
 * @param items is a vector of AbstractDataRefItem to generate from
 * @param version is a string containing the version of the data format used
 * @param key is a string containing the key from where the tap came from
 * @returns true on successful generation, false on failure
 */
bool DataFormatParser::createFromDataRefItems(const std::vector<AbstractDataRefItem*>& items, const std::string& version, const std::string& key)
{
    // temporary xml strings to use during generation
    XMLCh* xVersion = nullptr;
    XMLCh* xKey = nullptr;
    XMLCh* xName = nullptr;
    XMLCh* xType = nullptr;
    XMLCh* xSize = nullptr;
    XMLCh* xOffset = nullptr;
    XMLCh* xFormat = nullptr;

    // grab available dom implementation (nullptr = no options)
    DOMImplementation* impl = DOMImplementationRegistry::getDOMImplementation(nullptr);

    if (!impl)
    {
        return false;
    }

    // root element is format
    xFormat = XMLString::transcode("format");
    xercesc::DOMDocument* doc = impl->createDocument(nullptr, xFormat, nullptr);
    DOMElement* root = doc->getDocumentElement();

    // add the version
    xVersion = XMLString::transcode(version.c_str());
    root->setAttribute(attVersion, xVersion);

    // add the key
    xKey = XMLString::transcode(key.c_str());
    root->setAttribute(attKey, xKey);

    std::stringstream ss;

    for (auto i = items.begin(); i != items.end(); ++i)
    {
        // grab the numeric values and put them into xml strings
        size_t size = (*i)->getSize();
        off_t offset = (*i)->getOffset();

        ss.str(std::string());
        ss << size;
        xSize = XMLString::transcode(ss.str().c_str());

        ss.str(std::string());
        ss << offset;
        xOffset = XMLString::transcode(ss.str().c_str());

        // grab the remaining info
        xName = XMLString::transcode((*i)->getName().c_str());
        xType = XMLString::transcode((*i)->getType().c_str());

        // create the new element and attributes
        DOMElement* item = doc->createElement(tagItem);
        item->setAttribute(attName, xName);
        item->setAttribute(attType, xType);
        item->setAttribute(attSize, xSize);
        item->setAttribute(attOffset, xOffset);

        root->appendChild(item);

        // release resources
        XMLString::release(&xType);
        XMLString::release(&xSize);
        XMLString::release(&xName);
        XMLString::release(&xOffset);
    }

    // set the member string
    xmlStr = getStringFromDoc(doc);

    // release resources
    XMLString::release(&xFormat);
    XMLString::release(&xVersion);
    XMLString::release(&xKey);

    doc->release();

    // check validity against the schema
    if (!isValid(xmlStr, items.size()))
    {
        return false;
    }

    return true;
}

/**
 * @brief Creates the xml format string for kegs given an existing map of uuid to xml format string
 * @param map is a map of uuid to xml data format string
 * @returns true on successful generation, false on failure
 * @throws runtime_error on bad parse or bad uuid conversion
 */
// TODO throw instead of return
bool DataFormatParser::createFromUuidMap(const std::map<std::string, std::string>& uuidMap,
        const std::map<std::string, std::string>& metaMap)
{
    // temporary xml strings to use during generation
    XMLCh* xKeg = nullptr;
    XMLCh* xVersion = nullptr;
    XMLCh* xName = nullptr;
    XMLCh* xType = nullptr;
    XMLCh* xSize = nullptr;
    XMLCh* xOffset = nullptr;
    XMLCh* xUuid = nullptr;

    // grab available dom implementation (nullptr = no options)
    // note that the DOMImplementation object retains ownership of this object's
    // memory and application code should *not* delete it
    DOMImplementation* impl = DOMImplementationRegistry::getDOMImplementation(nullptr);

    if (!impl)
    {
        return false;
    }

    // root element is keg
    xKeg = XMLString::transcode("keg");
    xercesc::DOMDocument* doc = impl->createDocument(nullptr, xKeg, nullptr);
    DOMElement* root = doc->getDocumentElement();

    // all formats will go under a formats element
    DOMElement* formatsElem = doc->createElement(tagFormats);

    // set up the parser to parse the individual format strings
    parser->loadGrammar(xsdFile.c_str(), Grammar::SchemaGrammarType);
    parser->setErrorHandler(errHandler);
    parser->setValidationScheme(XercesDOMParser::Val_Always);
    parser->setDoNamespaces(true);
    parser->setDoSchema(true);
    parser->setExternalNoNamespaceSchemaLocation(xsdFile.c_str());

    for (auto i = uuidMap.begin(); i != uuidMap.end(); ++i)
    {
        // grabs the human readable string, as the 16 byte version may contain invalid xml characters
        std::string tmpUuid = lager_utils::getUuidString(i->first);

        std::string tmpXml = i->second;

        // reset the buffer each time for each new format string in the map
        MemBufInputSource xmlBuf((const XMLByte*)tmpXml.c_str(), tmpXml.size(), "unused");

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

        xercesc::DOMDocument* tmpDoc = parser->getDocument();

        // after successful parse, grab the root which is the format element
        DOMElement* tmpFormatRoot = tmpDoc->getDocumentElement();

        try
        {
            xUuid = XMLString::transcode(tmpUuid.c_str());
        }
        catch (TranscodingException& e)
        {
            std::stringstream ss;
            ss << "error transcoding uuid: " << XMLString::transcode(e.getMessage());
            throw std::runtime_error(ss.str());
        }

        // now add the new uuid attribute to the format element
        tmpFormatRoot->setAttribute(attUuid, xUuid);

        // import with the second param true makes a deep copy of the node
        DOMElement* formatElem = (DOMElement*)doc->importNode(tmpFormatRoot, true);

        // add the new element copy to our final formats element
        formatsElem->appendChild(formatElem);

        // release the memory from the temp doc
        // tmpDoc->release();
    }

    root->appendChild(formatsElem);

    // now add the meta data if it exists
    if (metaMap.size() > 0)
    {
        DOMElement* metaElem = doc->createElement(tagMetaData);

        for (auto meta = metaMap.begin(); meta != metaMap.end(); ++meta)
        {
            XMLCh* xKey;
            XMLCh* xValue;

            DOMElement* metaItem = doc->createElement(tagMeta);

            xKey = XMLString::transcode(meta->first.c_str());
            xValue = XMLString::transcode(meta->second.c_str());

            metaItem->setAttribute(attKey, xKey);
            metaItem->setAttribute(attValue, xValue);

            metaElem->appendChild(metaItem);

            XMLString::release(&xKey);
            XMLString::release(&xValue);
        }

        root->appendChild(metaElem);
    }

    // now that we have the completed doc, store the xml in the member
    xmlStr = getStringFromDoc(doc);

    // release resources
    XMLString::release(&xKeg);
    XMLString::release(&xVersion);
    XMLString::release(&xName);
    XMLString::release(&xType);
    XMLString::release(&xSize);
    XMLString::release(&xOffset);
    XMLString::release(&xUuid);
    doc->release();

    return true;
}

/**
 * @brief Gets the xml from a given DOM document
 * @param doc is a DOMDocument pointer to a valid DOM document
 * @returns a string containing the xml from the given document
 */
std::string DataFormatParser::getStringFromDoc(xercesc::DOMDocument* doc)
{
    // note that the DOMImplementation object retains ownership of this object's
    // memory and application code should *not* delete it
    DOMImplementation* pImplement;

    DOMLSSerializer* pSerializer;
    MemBufFormatTarget* pTarget;

    // grab available dom implementation (nullptr = no options)
    pImplement = DOMImplementationRegistry::getDOMImplementation(nullptr);

    // get the serializer
    pSerializer = ((DOMImplementationLS*)pImplement)->createLSSerializer();

    // set up the target which is a memory buffer
    pTarget = new MemBufFormatTarget();
    DOMLSOutput* pDomLsOutput = ((DOMImplementationLS*)pImplement)->createLSOutput();
    pDomLsOutput->setByteStream(pTarget);

    // write the xml to the buffer
    pSerializer->write(doc, pDomLsOutput);

    // copy the buffer into a real string
    std::string xmlOutput((char*)pTarget->getRawBuffer(), pTarget->getLen());

    // release the allocated memory
    pSerializer->release();
    pDomLsOutput->release();
    delete pTarget;

    return xmlOutput;
}

/**
 * @brief Checks the validity of a DataFormat xml string based on the number of items it should have
 * @param xml is a string containing the xml to check
 * @param itemCount is the number of expected items in the list
 * @returns true if the item counts match, false if not
 * @throws on a bad xml parse
 */
bool DataFormatParser::isValid(const std::string& xml, unsigned int itemCount)
{
    std::shared_ptr<DataFormat> testFormat = parseFromString(xml);

    if (testFormat->getItemCount() != itemCount)
    {
        return false;
    }

    return true;
}
