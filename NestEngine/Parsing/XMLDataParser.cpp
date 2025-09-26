#include "XMLDataParser.h"
#include "../Engine/Engine.h"
#include "../Managers/FactoryManager.h"
#include "../Objects/GameObject.h"
#include "../../Configuration/Macros.h"
#include "../Events/Event.h"
#include "../Parsing/XMLNode.h"
#include "XMLFile.h"
#include "../lib/tinyxml2/tinyxml2.h"
#include <cstring>

void nest::XMLDataParser::FillObjectFromNode(XMLNode& dataFile, GameObject* object)
{
    if (!dataFile.Check())
    {
        _LOG(LogType::kError, "node null");
        return;
    }
    if (!dataFile.HasChild())
    {
        _LOG(LogType::kError, "Node has no child!");
        return;
    }
    if (std::strcmp(dataFile.GetCurrentChild(), "Components") == 0)
    {
        dataFile.MoveToChildsFirstElement();
        bool loop = true;
        while (loop)
        {
            const HashedId childNameHash = nest::Hash(dataFile.GetCurrentChild());
            nest::Engine::GetManager<FactoryManager>()->CreateComponentById(childNameHash, object, dataFile);
            loop = dataFile.MoveToNextChild();
        }
    }
    else
    {
        dataFile.ResetChild();
        return;
    }
    dataFile.ResetChild();
    return;
}

const char* nest::XMLDataParser::XMLErrorToSTring(tinyxml2::XMLError error)
{
    switch (error)
    {
    case tinyxml2::XML_SUCCESS: return "XML_SUCCESS";
    case tinyxml2::XML_NO_ATTRIBUTE: return "XML_NO_ATTRIBUTE";
    case tinyxml2::XML_WRONG_ATTRIBUTE_TYPE: return "XML_WRONG_ATTRIBUTE_TYPE";
    case tinyxml2::XML_ERROR_FILE_NOT_FOUND: return "XML_ERROR_FILE_NOT_FOUND";
    case tinyxml2::XML_ERROR_FILE_COULD_NOT_BE_OPENED: return "XML_ERROR_FILE_COULD_NOT_BE_OPENED";
    case tinyxml2::XML_ERROR_FILE_READ_ERROR: return "XML_ERROR_FILE_READ_ERROR";
    case tinyxml2::XML_ERROR_PARSING_ELEMENT: return "XML_ERROR_PARSING_ELEMENT";
    case tinyxml2::XML_ERROR_PARSING_ATTRIBUTE: return "XML_ERROR_PARSING_ATTRIBUTE";
    case tinyxml2::XML_ERROR_PARSING_TEXT: return "XML_ERROR_PARSING_TEXT";
    case tinyxml2::XML_ERROR_PARSING_CDATA: return "XML_ERROR_PARSING_CDATA";
    case tinyxml2::XML_ERROR_PARSING_COMMENT: return "XML_ERROR_PARSING_COMMENT";
    case tinyxml2::XML_ERROR_PARSING_DECLARATION: return "XML_ERROR_PARSING_DECLARATION";
    case tinyxml2::XML_ERROR_PARSING_UNKNOWN: return "XML_ERROR_PARSING_UNKNOWN";
    case tinyxml2::XML_ERROR_EMPTY_DOCUMENT: return "XML_ERROR_EMPTY_DOCUMENT";
    case tinyxml2::XML_ERROR_MISMATCHED_ELEMENT: return "XML_ERROR_MISMATCHED_ELEMENT";
    case tinyxml2::XML_ERROR_PARSING: return "XML_ERROR_PARSING";
    case tinyxml2::XML_CAN_NOT_CONVERT_TEXT: return "XML_CAN_NOT_CONVERT_TEXT";
    case tinyxml2::XML_NO_TEXT_NODE: return "XML_NO_TEXT_NODE";
    default: return "Unknown XML error";
    }
}
