#include "BaseObject.h"
#include "../Events/Event.h"

// this is here so I can avoid the include in the game object file
void nest::BaseObject::HandleEvent([[maybe_unused]] const nest::Event& evt)
{}