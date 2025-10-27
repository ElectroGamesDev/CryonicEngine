#pragma once

#include "Components/Component.h"
#include "Raylib/RaylibWrapper.h"
#include "ThirdParty/Misc/json.hpp"

class RectTransform : public Component
{
public:
	RectTransform(GameObject* obj, int id) : Component(obj, id)
	{
		runInEditor = true;
		name = "RectTransform";
		iconUnicode = "\xef\x81\x87";

#if defined(EDITOR)
		std::string variables = R"(
        [
            0,
            [
                [
                    "Vector2",
                    "anchorMin",
                    [ 0, 0 ],
                    "Anchor Min"
                ],
                [
                    "Vector2",
                    "anchorMax",
                    [ 0, 0 ],
                    "Anchor Max"
                ],
                [
                    "Vector2",
                    "pivot",
                    [ 0.5, 0.5 ],
                    "Pivot"
                ],
                [
                    "Vector2",
                    "sizeDelta",
                    [ 100, 100 ],
                    "Size Delta"
                ],
                [
                    "Vector2",
                    "anchoredPosition",
                    [ 0, 0 ],
                    "Anchored Position"
                ]
            ]
        ]
    )";
		exposedVariables = nlohmann::json::parse(variables);
#endif
	}
	RectTransform* Clone() override
	{
		RectTransform* rt = new RectTransform(gameObject, -1);
		rt->anchorMin = anchorMin;
		rt->anchorMax = anchorMax;
		rt->pivot = pivot;
		rt->sizeDelta = sizeDelta;
		rt->anchoredPosition = anchoredPosition;
		return rt;
	}
#if defined(EDITOR)
	void EditorUpdate() override;
#endif

	RaylibWrapper::Rectangle GetRect();
	Vector2 GetPivotPosition();

	void SetAnchorMin(Vector2 min) { anchorMin = min; }
	void SetAnchorMax(Vector2 max) { anchorMax = max; }
	void SetPivot(Vector2 p) { pivot = p; }
	void SetSizeDelta(Vector2 sd) { sizeDelta = sd; }
	void SetAnchoredPosition(Vector2 ap) { anchoredPosition = ap; }

private:
	Vector2 anchorMin = { 0.0f, 0.0f };
	Vector2 anchorMax = { 0.0f, 0.0f };
	Vector2 pivot = { 0.5f, 0.5f };
	Vector2 sizeDelta = { 100.0f, 100.0f };
	Vector2 anchoredPosition = { 0.0f, 0.0f };
};