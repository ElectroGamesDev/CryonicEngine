#pragma once

#include "Components/Component.h"

class UIElement : public Component
{
public:
	UIElement(GameObject* obj, int id) : Component(obj, id) {}
	virtual ~UIElement() = default;
};