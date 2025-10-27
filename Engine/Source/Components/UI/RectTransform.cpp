#include "RectTransform.h"
#include "Raylib/RaylibWrapper.h"

RaylibWrapper::Rectangle RectTransform::GetRect()
{
	RectTransform* parentRt = gameObject ? gameObject->GetParent() ? gameObject->GetParent()->GetComponent<RectTransform>() : nullptr : nullptr; // Todo: What if parent doesnt have a RectTransform?
	RaylibWrapper::Rectangle parentRect = { 0.0f, 0.0f, (float)RaylibWrapper::GetScreenWidth(), (float)RaylibWrapper::GetScreenHeight() };
	if (parentRt)
		parentRect = parentRt->GetRect();

	float left = parentRect.x + parentRect.width * anchorMin.x;
	float top = parentRect.y + parentRect.height * anchorMin.y;
	float right = parentRect.x + parentRect.width * anchorMax.x;
	float bottom = parentRect.y + parentRect.height * anchorMax.y;

	float width = right - left + sizeDelta.x;
	float height = bottom - top + sizeDelta.y;

	float x = left + anchoredPosition.x;
	float y = top + anchoredPosition.y;

	return { x, y, width, height };
}

Vector2 RectTransform::GetPivotPosition()
{
	RaylibWrapper::Rectangle r = GetRect();
	return { r.x + r.width * pivot.x, r.y + r.height * pivot.y };
}

#if defined(EDITOR)
void RectTransform::EditorUpdate()
{
	anchorMin.x = exposedVariables[1][0][2][0].get<float>();
	anchorMin.y = exposedVariables[1][0][2][1].get<float>();
	anchorMax.x = exposedVariables[1][1][2][0].get<float>();
	anchorMax.y = exposedVariables[1][1][2][1].get<float>();
	pivot.x = exposedVariables[1][2][2][0].get<float>();
	pivot.y = exposedVariables[1][2][2][1].get<float>();
	sizeDelta.x = exposedVariables[1][3][2][0].get<float>();
	sizeDelta.y = exposedVariables[1][3][2][1].get<float>();
	anchoredPosition.x = exposedVariables[1][4][2][0].get<float>();
	anchoredPosition.y = exposedVariables[1][4][2][1].get<float>();
}
#endif