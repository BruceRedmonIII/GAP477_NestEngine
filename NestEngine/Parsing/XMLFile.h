#pragma once
#include "XMLNode.h"
struct SDL_RWops;
namespace tinyxml2
{
    class XMLDocument;
    class XMLElement;
    enum XMLError : int;
}
namespace nest
{
    class XMLDoc;
    // XMLFile stores the tinyxml document and contains pointers to the current root and the current child of that root
    class XMLFile
    {
        tinyxml2::XMLDocument* m_pFile;
        tinyxml2::XMLElement* m_pRoot;
        tinyxml2::XMLElement* m_pChild;
        bool m_isValid = false;
    public:
        XMLFile(const char* xmlFilePath);
        XMLFile(SDL_RWops* ops);
        XMLFile(tinyxml2::XMLDocument* pFile);
        XMLFile(tinyxml2::XMLElement* element);
        XMLFile();
        ~XMLFile();
        XMLFile(const XMLFile& copy) = delete;
        XMLFile& operator=(const XMLFile& copy) = delete;

        XMLNode GetRootNode();

        const char* GetRootName() const; // get the current root being pointed at
        const char* GetCurrentChild() const; // get the current child being pointed at

        const char* GetTextFromRoot() const; // get the text listed within the root
        const char* GetTextFromChild() const; // get the test listed within the child

        bool MoveToNextRoot(const char* name = nullptr); // iterate the root
        bool MoveToNextChild(const char* name = nullptr); // iterate the child
        bool MoveToNextChild(tinyxml2::XMLElement* element, const char* name = nullptr); // iterate the child
        bool MoveToFirstChildElement(tinyxml2::XMLElement* element, const char* name = nullptr);
        bool MoveToFirstChildElement(const char* name = nullptr);

        bool GetConstCharFromRoot(const char* attribute, const char*& value) const;
        bool GetBoolFromRoot(const char* attribute, bool& value) const;
        bool GetIntFromRoot(const char* attribute, int& value) const;
        bool GetFloatFromRoot(const char* attribute, float& value) const;
        bool GetConstCharFromChild(const char* attribute, const char*& value) const; // get the attribute associated with the child
        bool GetBoolFromChild(const char* attribute, bool& value) const;
        bool GetIntFromChild(const char* attribute, int& value) const;
        bool GetFloatFromChild(const char* attribute, float& value) const;

        bool RootHasAttribute(const char* attribute) const; // checks if an attribute exists within the root
        bool ChildHasAttribute(const char* attribute) const; // checks if an attribute exists within the child

        bool CompareRootName(const char* comparison) const;

        void ResetChild();

        tinyxml2::XMLNode* GetChildNode();
        XMLFile GetFileFromChildNode() const;

        tinyxml2::XMLDocument* GetDocument() const { return m_pFile; }

        static const char* XMLErrorToString(tinyxml2::XMLError error);
        const char* GetErrorName() const;
        bool Check() const;
    private:

        void LoadFile(const char* xmlFilePath);
    };
}
