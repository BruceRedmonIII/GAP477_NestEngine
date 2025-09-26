#include "XMLNode.h"
#include "XMLFile.h"
#include "../Configuration/LogMacro.hint"
#include "../lib/tinyxml2/tinyxml2.h"
//----------------------------------------------------------------
// will check if node given is nullptr, if not, will set as m_node
//----------------------------------------------------------------
nest::XMLNode::XMLNode(tinyxml2::XMLNode* node)
{
    if (node != nullptr)
    {
        m_node = node;
        m_pRoot = m_node->ToElement();
        m_pChild = m_pRoot->FirstChildElement();
        m_isValid = true;
    }
    else
        _LOG(LogType::kError, "XMLNode was passed a null node!");
}

tinyxml2::XMLNode* nest::XMLNode::GetNode()
{
    return m_node;
}

void nest::XMLNode::SetNode(tinyxml2::XMLNode* node)
{
    if (node == nullptr)
    {
        m_isValid = false;
        m_node = nullptr;
        m_pRoot = nullptr;
        m_pChild = nullptr;
        return;
    }
    m_node = node;
    m_pRoot = m_node->ToElement();
    m_pChild = m_pRoot->FirstChildElement();
    m_isValid = true;
}

nest::XMLNode nest::XMLNode::GetChildNode() const
{
    return m_pChild;
}

//-----------------------------------------------------------------------------------------------
// Gathers the current roots name that were pointing at
//-----------------------------------------------------------------------------------------------
const char* nest::XMLNode::GetCurrentRoot() const
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
const char* nest::XMLNode::GetCurrentChild() const
{
    if (m_pChild != nullptr)
        return m_pChild->Name();
    return nullptr;
}

//-----------------------------------------------------------------------------------------------
// Querys the attribute located at the current root
// Will return true if an error occured and log the error if logs enabled
//-----------------------------------------------------------------------------------------------
bool nest::XMLNode::GetConstCharFromRoot(const char* attribute, const char*& value) const
{
    if (m_pRoot != nullptr)
    {
        auto error = m_pRoot->QueryAttribute(attribute, &value);
        if (error != tinyxml2::XMLError::XML_SUCCESS)
        {
            // this function is used to verify if statements, and I don't want logging for everytime it fails
            if (error != tinyxml2::XML_NO_ATTRIBUTE)
            {
                _LOG(LogType::kError, XMLFile::XMLErrorToString(error));
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
// Will return true if an error occured and log the error if logs enabled
//-----------------------------------------------------------------------------------------------
bool nest::XMLNode::GetBoolFromRoot(const char* attribute, bool& value) const
{
    if (m_pRoot != nullptr)
    {
        auto error = m_pRoot->QueryBoolAttribute(attribute, &value);
        if (error != tinyxml2::XMLError::XML_SUCCESS)
        {
            // this function is used to verify if statements, and I don't want logging for everytime it fails
            if (error != tinyxml2::XML_NO_ATTRIBUTE)
            {
                _LOG(LogType::kError, XMLFile::XMLErrorToString(error));
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
// Will return true if an error occured and log the error if logs enabled
//-----------------------------------------------------------------------------------------------
bool nest::XMLNode::GetIntFromRoot(const char* attribute, int& value) const
{
    if (m_pRoot != nullptr)
    {
        auto error = m_pRoot->QueryIntAttribute(attribute, &value);
        if (error != tinyxml2::XMLError::XML_SUCCESS)
        {
            // this function is used to verify if statements, and I don't want logging for everytime it fails
            if (error != tinyxml2::XML_NO_ATTRIBUTE)
            {
                _LOG(LogType::kError, XMLFile::XMLErrorToString(error));
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
// Will return true if an error occured and log the error if logs enabled
//-----------------------------------------------------------------------------------------------
bool nest::XMLNode::GetFloatFromRoot(const char* attribute, float& value) const
{
    if (m_pRoot != nullptr)
    {
        auto error = m_pRoot->QueryFloatAttribute(attribute, &value);
        if (error != tinyxml2::XMLError::XML_SUCCESS)
        {
            // this function is used to verify if statements, and I don't want logging for everytime it fails
            if (error != tinyxml2::XML_NO_ATTRIBUTE)
            {
                _LOG(LogType::kError, XMLFile::XMLErrorToString(error));
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
// Will return true if an error occured and log the error if logs enabled
//-----------------------------------------------------------------------------------------------
bool nest::XMLNode::GetConstCharFromChild(const char* attribute, const char*& value) const
{
    if (m_pChild != nullptr)
    {
        auto error = m_pChild->QueryAttribute(attribute, &value);
        if (error != tinyxml2::XMLError::XML_SUCCESS)
        {
            // this function is used to verify if statements, and I don't want logging for everytime it fails
            if (error != tinyxml2::XML_NO_ATTRIBUTE)
            {
                _LOG(LogType::kError, XMLFile::XMLErrorToString(error));
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
// Will return true if an error occured and log the error if logs enabled
//-----------------------------------------------------------------------------------------------
bool nest::XMLNode::GetBoolFromChild(const char* attribute, bool& value) const
{
    if (m_pChild != nullptr)
    {
        auto error = m_pChild->QueryBoolAttribute(attribute, &value);
        if (error != tinyxml2::XMLError::XML_SUCCESS)
        {
            // this function is used to verify if statements, and I don't want logging for everytime it fails
            if (error != tinyxml2::XML_NO_ATTRIBUTE)
            {
                _LOG(LogType::kError, XMLFile::XMLErrorToString(error));
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
// Will return true if an error occured and log the error if logs enabled
//-----------------------------------------------------------------------------------------------
bool nest::XMLNode::GetIntFromChild(const char* attribute, int& value) const
{
    if (m_pChild != nullptr)
    {
        auto error = m_pChild->QueryIntAttribute(attribute, &value);
        if (error != tinyxml2::XMLError::XML_SUCCESS)
        {
            // this function is used to verify if statements, and I don't want logging for everytime it fails
            if (error != tinyxml2::XML_NO_ATTRIBUTE)
            {
                _LOG(LogType::kError, XMLFile::XMLErrorToString(error));
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
// Will return true if an error occured and log the error if logs enabled
//-----------------------------------------------------------------------------------------------
bool nest::XMLNode::GetFloatFromChild(const char* attribute, float& value) const
{
    if (m_pChild != nullptr)
    {
        auto error = m_pChild->QueryFloatAttribute(attribute, &value);
        if (error != tinyxml2::XMLError::XML_SUCCESS)
        {
            // this function is used to verify if statements, and I don't want logging for everytime it fails
            if (error != tinyxml2::XML_NO_ATTRIBUTE)
            {
                _LOG(LogType::kError, XMLFile::XMLErrorToString(error));
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
const char* nest::XMLNode::GetTextFromRoot() const
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
const char* nest::XMLNode::GetTextFromChild() const
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
bool nest::XMLNode::ChildHasAttribute(const char* attribute) const
{
    // BoolAttribute will check if the attribute is present, if not it will return the default value which is initially set to false unless specified
    // https://leethomason.github.io/tinyxml2/classtinyxml2_1_1_x_m_l_element.html#a95a89b13bb14a2d4655e2b5b406c00d4
    return m_pChild->FindAttribute(attribute) != 0;
}

bool nest::XMLNode::RootHasAttribute(const char* attribute) const
{
    // BoolAttribute will check if the attribute is present, if not it will return the default value which is initially set to false unless specified
    // https://leethomason.github.io/tinyxml2/classtinyxml2_1_1_x_m_l_element.html#a95a89b13bb14a2d4655e2b5b406c00d4
    return m_pRoot->FindAttribute(attribute) != 0;
}

//---------------------------------------------------------------------
// Resets current child to first element in root
// Does nothing if root is null
//---------------------------------------------------------------------
void nest::XMLNode::ResetChild()
{
    if (m_pRoot != nullptr)
    {
        m_pRoot = m_node->ToElement();
        m_pChild = m_pRoot->FirstChildElement();
    }
}

//-----------------------------------------------------------------------------------------------
// Returns true if valid file, logs error if invalid
//-----------------------------------------------------------------------------------------------
bool nest::XMLNode::Check() const
{
    return m_isValid;
}

//-----------------------------------------------------------------------------------------------
// Moves to the next root if possible
// returns true if another root is found, or else returns false
//-----------------------------------------------------------------------------------------------
bool nest::XMLNode::MoveToNextRoot(const char* name)
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
bool nest::XMLNode::MoveToNextChild(const char* name)
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

//-------------------------------------------------------------------------
// Replaces the current child with the childs first element
//-------------------------------------------------------------------------
bool nest::XMLNode::MoveToChildsFirstElement(const char* name)
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
