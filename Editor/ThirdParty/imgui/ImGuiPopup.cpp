#pragma once

#include "ImGuiPopup.h"
#include "imgui.h"

namespace ImGuiPopup
{
	static bool _active = false;
	static std::string _title = "";
	static std::string _content = "";
	static bool _progressBar = false;
	static int _progress = 0;
	static std::vector<Button> _buttons;
	static bool _cancel = false;

	void Render()
	{
		if (!_active)
			return;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.10f, 0.10f, 0.10f, 1.00f));

		ImGuiIO& io = ImGui::GetIO();
		float ySize = 45;
		if (_progressBar) ySize += 60;
		if (_buttons.size() > 0) ySize += 20;
		ImGui::SetNextWindowPos({ io.DisplaySize.x / 2 - 200, io.DisplaySize.y / 2 - ySize });
		ImGui::SetNextWindowSize({ 350, ySize });
		ImGui::SetNextWindowFocus();
		ImGui::Begin(_title.c_str(), nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

		ImGui::SetCursorPosX(5);
		if (_progressBar)
		{
			ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
			ImGui::SetCursorPosY(40);
			ImGui::ProgressBar(_progress / 100.0f, { ImGui::GetWindowWidth() - 10, 20 });
			ImGui::PopStyleColor();

			ImGui::SetCursorPosY(70);
			ImGui::Text(_content.c_str());
		}
		else
			ImGui::Text(_content.c_str());

		ImGui::NewLine();
		if (_buttons.size() > 0)
		{
			bool cancelled = _cancel; // A local variable must be used since it's possible for it to be cancelled after BeginDisabled but before EndDisabled which triggers an ASSERT
			if (cancelled) ImGui::BeginDisabled();

			float buttonWidth = 75;
			float xPos = ImGui::GetWindowWidth() - buttonWidth - 5;
			for (Button& button : _buttons)
			{
				ImGui::SetCursorPos({ xPos, ImGui::GetWindowHeight() - 28});
				if (ImGui::Button(button.name.c_str(), { buttonWidth, 20 }))
					button.callback();
				xPos -= buttonWidth;
			}

			if (cancelled) ImGui::EndDisabled();
		}

		ImGui::End();

		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor();
	}

	void Create(std::string title, std::string content, std::vector<Button> buttons, bool progressBar, int progress)
	{
		_title = title;
		_content = content;
		_progressBar = progressBar;
		_progress = progress;
		_buttons = buttons;
		_cancel = false;
		_active = true;
	}

	void AddButton(std::string name, std::function<void()> callback)
	{
		_buttons.push_back({ name, callback });
	}

	void ClearButtons()
	{
		_buttons.clear();
	}

	void SetActive(bool active) { _active = active; }
	bool IsActive() { return _active; }
	void SetTitle(std::string title) { _title = title; }
	void SetContent(std::string content) { _content = content; }
	void SetProgress(int progress) { _progress = progress; };
	int GetProgress() { return _progress; }
	void Cancel() { _cancel = true; };
	bool IsCancelled() { return _cancel; }
}