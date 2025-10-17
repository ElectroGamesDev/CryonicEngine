#pragma once

#include <string>
#include <functional>
#include <vector>

namespace ImGuiPopup
{
	struct Button
	{
		std::string name;
		std::function<void()> callback;
	};

	void Render();
	void Create(std::string title, std::string content, std::vector<Button> buttons, bool progressBar = false, int progress = 0);
	void AddButton(std::string name, std::function<void()> callback);
	void ClearButtons();
	void SetActive(bool active);
	bool IsActive();
	void SetTitle(std::string title);
	void SetContent(std::string content);
	void SetProgress(int progress);
	int GetProgress();
	void Cancel();
	bool IsCancelled();
}