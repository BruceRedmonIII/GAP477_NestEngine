#pragma once
#include "../../Math/MathAlias.h"
#include "../../Configuration/Macros.h"
namespace nest
{
    // Forward Declares
    //----------------------------------------------------------------------------------------------------------//
    class GameObject;
    class Event;
    //----------------------------------------------------------------------------------------------------------//

    class Component
    {
    protected:
        GameObject* m_owner = nullptr;
    public:
        SET_HASHED_ID(Component)
        // Destructor
        virtual ~Component() = default;
        // Copy Assignment
        virtual Component& operator=(const Component& copy) = delete;
        // Move Assignment
        virtual Component& operator=(const Component&& copy) = delete;
        // Basic init for classes that only need the owner set
        virtual bool Init(GameObject* owner) { (void)owner; return false; }
        // Update the component and whatever comes with that
        virtual void Update([[maybe_unused]] float deltaTime) {}
        // Gathers and processes any inputs the component needs
        virtual void ProcessInput() {}
        // Handle the event given, usually based on the event id alone
        virtual void HandleEvent(const nest::Event& evt) { (void)evt; }
        // Tell this object it has collided with the collider, while passing the collider
        virtual void Collided(GameObject* collider) { (void)collider; }
        // Returns the owner of this component
        virtual GameObject* GetOwner() { return nullptr; }
        // Tells Graphics to Draw/Render this component onto the screen
        virtual void Draw() {}
        // Unregister this component from any managers it belongs to
        virtual void Unregister() {}

        virtual void UpdatePosition([[maybe_unused]] Vec2 pos) {}
        virtual void UpdateOrientation([[maybe_unused]] float orientation) {}

        virtual void Align() {}
    };
}
