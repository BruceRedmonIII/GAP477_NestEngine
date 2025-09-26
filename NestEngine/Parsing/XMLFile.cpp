#include "XMLFile.h"
#include "../Configuration/LogMacro.hint"
#include "../lib/tinyxml2/tinyxml2.h"
#include "../lib/BleachNew/BleachNew.h"
#include "../GlobalConstants/constants.h"
#include "../Managers/FileManager.h"
#include <SDL_rwops.h>
#include <iostream>
//-----------------------------------------------------------------------------------------------
// XMLFile constructor
// loads the file
//-----------------------------------------------------------------------------------------------
nest::XMLFile::XMLFile(const char* xmlFilePath)
    : m_pFile(BLEACH_NEW(tinyxml2::XMLDocument))
    , m_pRoot(nullptr)
    , m_pChild(nullptr)
{
    LoadFile(nest::FileManager::ResolvePath(s_kXMLFilesKey, xmlFilePath).c_str());
}

nest::XMLFile::XMLFile(SDL_RWops* ops)
    : m_pFile(BLEACH_NEW(tinyxml2::XMLDocument))
    , m_pRoot(nullptr)
    , m_pChild(nullptr)
{
    // Check if ops is not null
    if (ops != nullptr)
    {
        // Determine the size of the file
        Sint64 fileSize = SDL_RWsize(ops);
        if (fileSize < 0)
        {
            _LOG(LogType::kError, "Failed to determine the size of the file");
            return;
        }

        // Allocate memory to read the file
        char* fileData = BLEACH_NEW(char[fileSize + 1]); // +1 for null terminator
        if (!fileData)
        {
            _LOG(LogType::kError, "Failed to allocate memory for reading file data");
            return;
        }

        // Read the file data into the allocated memory
        Sint64 bytesRead = static_cast<Sint64>(SDL_RWread(ops, fileData, 1, fileSize));
        if (bytesRead != fileSize)
        {
            _LOG(LogType::kError, "Failed to read file data");
            BLEACH_DELETE_ARRAY(fileData); // Clean up allocated memory
            return;
        }

        // Null-terminate the file data
        fileData[fileSize] = '\0';

        // Load the XML data from the file data
        m_pFile->Parse(fileData);
        m_pRoot = m_pFile->FirstChildElement();
        m_pChild = m_pRoot->FirstChildElement();

        // Clean up allocated memory
        BLEACH_DELETE_ARRAY(fileData);
        m_isValid = true;
    }
    else
    {
        _LOG(LogType::kError, "SDL_RWops object is null");
    }
}

nest::XMLFile::XMLFile(tinyxml2::XMLDocument* pFile)
{
    m_pFile = pFile;
    m_pRoot = pFile->RootElement();
    m_pChild = m_pRoot->FirstChildElement();
    m_isValid = true;
}

nest::XMLFile::XMLFile(tinyxml2::XMLElement* element)
{

    m_pFile = element->GetDocument();
    m_pRoot = m_pFile->FirstChildElement();
    m_pChild = m_pRoot->FirstChildElement();
    m_isValid = true;
}

nest::XMLFile::XMLFile()
    : m_pFile(nullptr)
    , m_pRoot(nullptr)
    , m_pChild(nullptr)
{
}

//-----------------------------------------------------------------------------------------------
// XMLFile Destructor
// destroys the file
//-----------------------------------------------------------------------------------------------
nest::XMLFile::~XMLFile()
{
    if (!m_pFile->Error())
    {
        //m_pFile->DeleteChildren();
        //m_pFile->Clear();
        BLEACH_DELETE(m_pFile);
    }
}

//-----------------------------------------------------------------------------------------------
// Loads the file
//-----------------------------------------------------------------------------------------------
void nest::XMLFile::LoadFile(const char* xmlFilePath)
{
    m_pFile->LoadFile(xmlFilePath);

    if (m_pFile->Error())
    {
        m_isValid = false;
        _LOG_V(LogType::kError, m_pFile->ErrorStr(), " | Path = ", std::filesystem::current_path());
    }
    else
    {
        m_pRoot = m_pFile->FirstChildElement();
        m_pChild = m_pRoot->FirstChildElement();
        m_isValid = true;
    }
}

nest::XMLNode nest::XMLFile::GetRootNode()
{
    return XMLNode(m_pRoot);
}

//-----------------------------------------------------------------------------------------------
// Gathers the current roots name that were pointing at
//-----------------------------------------------------------------------------------------------
const char* nest::XMLFile::GetRootName() const
{
    if (m_pRoot)
    {
        const char* name = m_pRoot->Name();
        return name; // gets the name of the tag
    }
    _LOG(LogType::kError, "COULD NOT FIND ROOT ELEMENT!");
    return nullptr;
}

//-----------------------------------------------------------------------------------------------
// Gathers the current childs name that were pointing at
//-----------------------------------------------------------------------------------------------
const char* nest::XMLFile::GetCurrentChild() const
{
    if (m_pChild != nullptr)
        return m_pChild->Name();
    return nullptr;
}

//-----------------------------------------------------------------------------------------------
// Querys the attribute located at the current root
// Will return true if an error occured and log the error if logs enabled
//-----------------------------------------------------------------------------------------------
bool nest::XMLFile::GetConstCharFromRoot(const char* attribute, const char*& value) const
{
    if (m_pRoot != nullptr)
    {
        auto error = m_pRoot->QueryAttribute(attribute, &value);
        if (error != tinyxml2::XMLError::XML_SUCCESS)
        {
            // this function is used to verify if statements, and I don't want logging for every time it fails
            if (error != tinyxml2::XML_NO_ATTRIBUTE)
            {
                _LOG(LogType::kError, XMLErrorToString(error));
            }
            return false;
        }
        return true;
    }
    _LOG(LogType::kError, "Root element is nullptr!");
    return false;
}

//-----------------------------------------------------------------------------------------------
// Querys the attribute located at the current root
// Will return true if an error occurred and log the error if logs enabled
//-----------------------------------------------------------------------------------------------
bool nest::XMLFile::GetBoolFromRoot(const char* attribute, bool& value) const
{
    if (m_pRoot != nullptr)
    {
        auto error = m_pRoot->QueryBoolAttribute(attribute, &value);
        if (error != tinyxml2::XMLError::XML_SUCCESS)
        {
            // this function is used to verify if statements, and I don't want logging for every time it fails
            if (error != tinyxml2::XML_NO_ATTRIBUTE)
            {
                _LOG(LogType::kError, XMLErrorToString(error));
            }
            return false;
        }
        return true;
    }
    _LOG(LogType::kError, "Root element is nullptr!");
    return false;
}

//-----------------------------------------------------------------------------------------------
// Querys the attribute located at the current root
// Will return true if an error occurred and log the error if logs enabled
//-----------------------------------------------------------------------------------------------
bool nest::XMLFile::GetIntFromRoot(const char* attribute, int& value) const
{
    if (m_pRoot != nullptr)
    {
        auto error = m_pRoot->QueryIntAttribute(attribute, &value);
        if (error != tinyxml2::XMLError::XML_SUCCESS)
        {
            // this function is used to verify if statements, and I don't want logging for every time it fails
            if (error != tinyxml2::XML_NO_ATTRIBUTE)
            {
                _LOG(LogType::kError, XMLErrorToString(error));
            }
            return false;
        }
        return true;
    }
    _LOG(LogType::kError, "Root element is nullptr!");
    return false;
}

//-----------------------------------------------------------------------------------------------
// Querys the attribute located at the current root
// Will return true if an error occurred and log the error if logs enabled
//-----------------------------------------------------------------------------------------------
bool nest::XMLFile::GetFloatFromRoot(const char* attribute, float& value) const
{
    if (m_pRoot != nullptr)
    {
        auto error = m_pRoot->QueryFloatAttribute(attribute, &value);
        if (error != tinyxml2::XMLError::XML_SUCCESS)
        {
            // this function is used to verify if statements, and I don't want logging for every time it fails
            if (error != tinyxml2::XML_NO_ATTRIBUTE)
            {
                _LOG(LogType::kError, XMLErrorToString(error));
            }
            return false;
        }
        return true;
    }
    _LOG(LogType::kError, "Root element is nullptr!");
    return false;
}

//-----------------------------------------------------------------------------------------------
// Querys the attribute located at the current child
// Will return true if an error occurred and log the error if logs enabled
//-----------------------------------------------------------------------------------------------
bool nest::XMLFile::GetConstCharFromChild(const char* attribute, const char*& value) const
{
    if (m_pChild != nullptr)
    {
        auto error = m_pChild->QueryAttribute(attribute, &value);
        if (error != tinyxml2::XMLError::XML_SUCCESS)
        {
            // this function is used to verify if statements, and I don't want logging for every time it fails
            if (error != tinyxml2::XML_NO_ATTRIBUTE)
            {
                _LOG(LogType::kError, XMLErrorToString(error));
            }
            return false;
        }
        return true;
    }
    _LOG(LogType::kError, "Child element is nullptr!");
    return false;
}

//-----------------------------------------------------------------------------------------------
// Querys the attribute located at the current child
// Will return true if an error occurred and log the error if logs enabled
//-----------------------------------------------------------------------------------------------
bool nest::XMLFile::GetBoolFromChild(const char* attribute, bool& value) const
{
    if (m_pChild != nullptr)
    {
        auto error = m_pChild->QueryBoolAttribute(attribute, &value);
        if (error != tinyxml2::XMLError::XML_SUCCESS)
        {
            // this function is used to verify if statements, and I don't want logging for every time it fails
            if (error != tinyxml2::XML_NO_ATTRIBUTE)
            {
                _LOG(LogType::kError, XMLErrorToString(error));
            }
            return false;
        }
        return true;
    }
    _LOG(LogType::kError, "Child element is nullptr!");
    return false;
}

//-----------------------------------------------------------------------------------------------
// Querys the attribute located at the current child
// Will return true if an error occurred and log the error if logs enabled
//-----------------------------------------------------------------------------------------------
bool nest::XMLFile::GetIntFromChild(const char* attribute, int& value) const
{
    if (m_pChild != nullptr)
    {
        auto error = m_pChild->QueryIntAttribute(attribute, &value);
        if (error != tinyxml2::XMLError::XML_SUCCESS)
        {
            // this function is used to verify if statements, and I don't want logging for every time it fails
            if (error != tinyxml2::XML_NO_ATTRIBUTE)
            {
                _LOG(LogType::kError, XMLErrorToString(error));
            }
            return false;
        }
        return true;
    }
    _LOG(LogType::kError, "Child element is nullptr!");
    return false;
}

//-----------------------------------------------------------------------------------------------
// Querys the attribute located at the current child
// Will return true if an error occurred and log the error if logs enabled
//-----------------------------------------------------------------------------------------------
bool nest::XMLFile::GetFloatFromChild(const char* attribute, float& value) const
{
    if (m_pChild != nullptr)
    {
        auto error = m_pChild->QueryFloatAttribute(attribute, &value);
        if (error != tinyxml2::XMLError::XML_SUCCESS)
        {
            // this function is used to verify if statements, and I don't want logging for every time it fails
            if (error != tinyxml2::XML_NO_ATTRIBUTE)
            {
                _LOG(LogType::kError, XMLErrorToString(error));
            }
            return false;
        }
        return true;
    }
    _LOG(LogType::kError, "Child element is nullptr!");
    return false;
}

//-----------------------------------------------------------------------------------------------
// Gathers the text located at the current Root
//-----------------------------------------------------------------------------------------------
const char* nest::XMLFile::GetTextFromRoot() const
{
    if (m_pRoot != nullptr)
    {
        return m_pRoot->GetText();
    }
    _LOG(LogType::kError, "Root element is nullptr!");
    return nullptr;
}

//-----------------------------------------------------------------------------------------------
// Gathers the text located at the current child
//-----------------------------------------------------------------------------------------------
const char* nest::XMLFile::GetTextFromChild() const
{
    if (m_pChild != nullptr)
    {
        return m_pChild->GetText();
    }
    _LOG(LogType::kError, "Child element is nullptr!");
    return nullptr;
}

//-----------------------------------------------------------------------------------------------
// Returns whether or not the current child has the attribute given
//-----------------------------------------------------------------------------------------------
bool nest::XMLFile::ChildHasAttribute(const char* attribute) const
{
    // FindAttribute will check if the attribute is present, if not it will return the default value which is initially set to false unless specified
    // https://leethomason.github.io/tinyxml2/classtinyxml2_1_1_x_m_l_element.html#a95a89b13bb14a2d4655e2b5b406c00d4
    return m_pChild->FindAttribute(attribute) != nullptr;
}

bool nest::XMLFile::CompareRootName(const char* comparison) const
{
    if (strcmp(GetRootName(), comparison) == 0)
        return true;
    return false;
}

bool nest::XMLFile::RootHasAttribute(const char* attribute) const
{
    // FindAttribute will check if the attribute is present, if not it will return the default value which is initially set to false unless specified
    // https://leethomason.github.io/tinyxml2/classtinyxml2_1_1_x_m_l_element.html#a95a89b13bb14a2d4655e2b5b406c00d4
    return m_pRoot->FindAttribute(attribute) != nullptr;
}

//---------------------------------------------------------------------
// Resets current child to first element in root
// Does nothing if root is null
//---------------------------------------------------------------------
void nest::XMLFile::ResetChild()
{
    if (m_pRoot != nullptr)
    {
        m_pChild = m_pRoot->FirstChildElement();
    }
}

tinyxml2::XMLNode* nest::XMLFile::GetChildNode()
{
    return static_cast<tinyxml2::XMLNode*>(m_pChild);
}

nest::XMLFile nest::XMLFile::GetFileFromChildNode() const
{
    return { m_pChild};
}

const char* nest::XMLFile::GetErrorName() const
{
    return m_pFile->ErrorName();
}

//-----------------------------------------------------------------------------------------------
// Returns true if valid file, logs error if invalid
//-----------------------------------------------------------------------------------------------
bool nest::XMLFile::Check() const
{
    return m_isValid;
}

const char* nest::XMLFile::XMLErrorToString(tinyxml2::XMLError error)
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

//-----------------------------------------------------------------------------------------------
// Moves to the next root if possible
// returns true if another root is found, or else returns false
//-----------------------------------------------------------------------------------------------
bool nest::XMLFile::MoveToNextRoot(const char* name)
{
    if (name)
        m_pRoot = m_pRoot->NextSiblingElement(name);
    else
        m_pRoot = m_pRoot->NextSiblingElement();
    if (m_pRoot)
    {
        m_pChild = m_pRoot->FirstChildElement();
        return true;
    }
    return false;
}

//-----------------------------------------------------------------------------------------------
// Moves to the next child if possible
// returns true if another child is found, or else returns false
//-----------------------------------------------------------------------------------------------
bool nest::XMLFile::MoveToNextChild(const char* name)
{
    if (m_pChild != nullptr)
    {
        if (name)
            m_pChild = m_pChild->NextSiblingElement(name);
        else
            m_pChild = m_pChild->NextSiblingElement();
        if (m_pChild)
            return true;
        return false;
    }
    return false;
}

bool nest::XMLFile::MoveToNextChild(
    tinyxml2::XMLElement* element, const char* name)
{
    if (element != nullptr)
    {
        if (name)
            m_pChild = element->NextSiblingElement(name);
        else
            m_pChild = element->NextSiblingElement();
        if (m_pChild)
            return true;
        return false;
    }
    return false;
}

bool nest::XMLFile::MoveToFirstChildElement(tinyxml2::XMLElement* element, const char* name)
{
    if (element != nullptr)
    {
        if (name)
            m_pChild = element->FirstChildElement(name);
        else
            m_pChild = element->FirstChildElement();
        if (m_pChild)
            return true;
        return false;
    }
    return false;
}

bool nest::XMLFile::MoveToFirstChildElement(const char* name)
{
    if (m_pChild != nullptr)
    {
        if (name)
            m_pChild = m_pChild->FirstChildElement(name);
        else
            m_pChild = m_pChild->FirstChildElement();
        if (m_pChild)
            return true;
        return false;
    }
    return false;
}

