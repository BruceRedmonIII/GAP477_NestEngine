#pragma once
namespace tinyxml2
{
    class XMLNode;
    class XMLElement;
}
namespace nest
{
    // OSTXMLNode exists to navigate at a lower level than the entire document
    // it also allows the storage of subtrees into data containers to quickly make new objects
    class XMLNode
    {
        tinyxml2::XMLNode* m_node = nullptr;
        tinyxml2::XMLElement* m_pRoot = nullptr;
        tinyxml2::XMLElement* m_pChild = nullptr;
        bool m_isValid = false;
    public:
        XMLNode() = default;
        XMLNode(tinyxml2::XMLNode* node);
        tinyxml2::XMLNode* GetNode();
        void SetNode(tinyxml2::XMLNode* node);
        XMLNode GetChildNode() const;

        tinyxml2::XMLElement* GetCurrentChildElement() const { return m_pChild; }


        const char* GetCurrentRoot() const; // get the current root being pointed at
        const char* GetCurrentChild() const; // get the current child being pointed at

        bool GetConstCharFromRoot(const char* attribute, const char*& value) const;
        bool GetBoolFromRoot(const char* attribute, bool& value) const;
        bool GetIntFromRoot(const char* attribute, int& value) const;
        bool GetFloatFromRoot(const char* attribute, float& value) const;
        bool GetConstCharFromChild(const char* attribute, const char*& value) const; // get the attribute associated with the child
        bool GetBoolFromChild(const char* attribute, bool& value) const;
        bool GetIntFromChild(const char* attribute, int& value) const;
        bool GetFloatFromChild(const char* attribute, float& value) const;

        const char* GetTextFromRoot() const; // get the text listed within the root
        const char* GetTextFromChild() const; // get the test listed within the child

        bool MoveToNextRoot(const char* name = nullptr); // iterate the root
        bool MoveToNextChild(const char* name = nullptr); // iterate the child
        bool MoveToChildsFirstElement(const char* name = nullptr); // iterate to the childs first element, replacing the current child
        bool HasChild() const { return m_pChild != nullptr; }

        bool RootHasAttribute(const char* attribute) const; // checks if an attribute exists within the root
        bool ChildHasAttribute(const char* attribute) const; // checks if an attribute exists within the child

        void ResetChild();

        bool Check() const;
    };
}