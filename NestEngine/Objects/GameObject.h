#pragma once
#include "BaseObject.h"
#include "Components/Component.h"
#include "../Math/MathAlias.h"
#include "../../DataWrappers/HashMapWrapper.h"
namespace nest
{
    enum class Keycodes;
    class Event;
    class InputEvent;
    class UIWidget;
    using Components = HashMapWrapper<Component>;
    // GameObject is a generic object in the game.
    // It is ultimately a container object for different components which control how it moves, looks, and behaves
    class GameObject : public nest::BaseObject
    {
    protected:
        // the brains and body of this object
        Components* m_pComponentWrapper = nullptr;
        const HashedId m_id = 0;
        bool m_delete = false;
        bool m_active = false;
        int m_layer = 0;
    public:
        GameObject() = default;
        GameObject(HashedId id);
        virtual ~GameObject();
        GameObject(const GameObject& copy) = delete;
        GameObject& operator=(const GameObject& copy) = delete;

        // map component with label to this object
        Components* GetComponentWrapper() { return m_pComponentWrapper; }

        template <class T>
        T* GetComponent()
        {
            return m_pComponentWrapper->GetContained<T>();
        }
        template <class T>
        void AddComponent(T* component)
        {
            m_pComponentWrapper->Emplace(component);
        }
        void UnregisterAllComponents();

        HashedId GetId() const { return m_id; }

        virtual bool Init();
        virtual void Update(float deltaTime) override; // tell all components to update
        virtual void Draw()  override; // tell all components to draw
        virtual void HandleEvent(const nest::Event& evt) override;
        virtual bool GetActive() const override { return m_active; }
        virtual void SetActive(bool status) override { m_active = status; }
        virtual bool GetDelete() const { return m_delete; }
        virtual void SetDelete(bool status) { m_delete = status; }
        virtual void ProcessInput() override; // tell all components to process input
        virtual void Align() override;
        virtual void AddObject(GameObject* object) { (void)object; }
        virtual void AddWidget(UIWidget* widget) { (void)widget; }
        virtual void UpdatePosition(Vec2 pos);
        virtual int GetLayer() const override { return m_layer; }
        virtual void SetLayer(int layer) { m_layer = layer; }
        virtual void Collided(GameObject* collider);
    };
}
