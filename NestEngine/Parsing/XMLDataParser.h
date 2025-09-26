#pragma once
namespace tinyxml2
{
    class XMLNode;
    class XMLDocument;
    enum XMLError : int;
}
namespace nest
{
    class XMLFile;
    class GameObject;
    class XMLDoc;
    class XMLNode;
    class XMLDataParser
    {
    public:
        XMLDataParser() = default;
        ~XMLDataParser() = default;
        XMLDataParser(const XMLDataParser& copy) = delete;
        XMLDataParser& operator=(const XMLDataParser& copy) = delete;
        static void FillObjectFromNode(XMLNode& dataFile, GameObject* object);
        static const char* XMLErrorToSTring(tinyxml2::XMLError error);
    };
}
