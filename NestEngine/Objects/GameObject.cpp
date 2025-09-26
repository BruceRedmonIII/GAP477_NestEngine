#include "GameObject.h"
#include "Components/Component.h"
#include "../lib/BleachNew/BleachNew.h"
#include <ranges>
nest::GameObject::GameObject(HashedId id) : m_id(id)
{
    m_pComponentWrapper = BLEACH_NEW(Components);
}

//-----------------------------------------------------------------------------------------------
// GameObject Destructor
//-----------------------------------------------------------------------------------------------
nest::GameObject::~GameObject()
{
    BLEACH_DELETE(m_pComponentWrapper);
}

//-----------------------------------------------------------------------------------------------
// Calls ProcessInput on all contained components
//-----------------------------------------------------------------------------------------------
void nest::GameObject::ProcessInput()
{
    auto& componentWrapper = m_pComponentWrapper->GetAllComponents();
    for (auto const& value : std::views::values(componentWrapper))
    {
        value->ProcessInput();
    }
}

void nest::GameObject::Align()
{
    auto& componentWrapper = m_pComponentWrapper->GetAllComponents();
    for (auto const& value : std::views::values(componentWrapper))
    {
        value->Align();
    }
}


bool nest::GameObject::Init()
{
	m_pComponentWrapper = BLEACH_NEW(Components);
    return true;
}

//-----------------------------------------------------------------------------------------------
// Calls Update on all contained components
//-----------------------------------------------------------------------------------------------
void nest::GameObject::Update(float deltaTime)
{
    auto& componentWrapper = m_pComponentWrapper->GetAllComponents();
    for (auto const& value : std::views::values(componentWrapper))
    {
        value->Update(deltaTime);
    }
}

//-----------------------------------------------------------------------------------------------
// Calls Draw on all contained components
//-----------------------------------------------------------------------------------------------
void nest::GameObject::Draw() 
{
    if (m_active)
    {
        auto& componentWrapper = m_pComponentWrapper->GetAllComponents();
        for (auto const& value : std::views::values(componentWrapper))
        {
            value->Draw();
        }
    }
}

//-----------------------------------------------------------------------------------------------
// Calls HandleMessage on all contained components
//-----------------------------------------------------------------------------------------------
void nest::GameObject::HandleEvent(const nest::Event& evt)
{
    auto& componentWrapper = m_pComponentWrapper->GetAllComponents();
    for (auto const& value : std::views::values(componentWrapper))
    {
        value->HandleEvent(evt);
    }
}


void nest::GameObject::UpdatePosition(Vec2 pos)
{
    auto& componentWrapper = m_pComponentWrapper->GetAllComponents();
    for (auto const& value : std::views::values(componentWrapper))
    {
        value->UpdatePosition(pos);
    }
}

//-----------------------------------------------------------------------------------------------
// Calls Collided on all contained components
//-----------------------------------------------------------------------------------------------
void nest::GameObject::Collided(GameObject* collider)
{
    auto& componentWrapper = m_pComponentWrapper->GetAllComponents();
    for (auto const& value : std::views::values(componentWrapper))
    {
        value->Collided(collider);
    }
}

//-----------------------------------------------------------------------------------------------
// Calls Unregister on all contained components
//-----------------------------------------------------------------------------------------------
void nest::GameObject::UnregisterAllComponents()
{
    auto& componentWrapper = m_pComponentWrapper->GetAllComponents();
    for (auto const& value : std::views::values(componentWrapper))
    {
        value->Unregister();
    }
}