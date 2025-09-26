#pragma once
#include <cstdint>
namespace nest
{
	class Event;
	class BaseObject
	{
	public:
		virtual ~BaseObject() = default;
		virtual void Draw() {}
		virtual void Update([[maybe_unused]] float deltaTime) {}
		virtual void HandleEvent([[maybe_unused]] const nest::Event& evt);
		virtual void ProcessInput() {}
		virtual void Align() {};
		virtual uint64_t GetId() const = 0;
		virtual bool GetActive() const = 0;
		virtual void SetActive(bool status) = 0;
		virtual int GetLayer() const { return 0; }
	};
}