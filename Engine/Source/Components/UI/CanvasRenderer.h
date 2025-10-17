#pragma once

#include "Components/Component.h"

class CanvasRenderer : public Component
{
public:
    CanvasRenderer(GameObject* obj, int id) : Component(obj, id)
	{
		runInEditor = true;
		name = "CanvasRenderer";
		iconUnicode = "\xef\x95\x9d";
	}
	CanvasRenderer* Clone() override
	{
		return new CanvasRenderer(gameObject, -1);
	}
	// Hide everything from API
	void Start() override;
};