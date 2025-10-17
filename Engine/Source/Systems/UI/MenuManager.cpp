#include "MenuManager.h"

namespace MenuManager
{
	static std::stack<Menu> openMenus;
	static Event globalOpenEvent;
	static Event globalCloseEvent;

	void Init()
	{
#if !defined(EDITOR)
		EventSystem::Subscribe("ActiveSceneChanged", CloseAllMenus);
#endif
	}

	void OpenMenu(GameObject* menu, std::function<void()> closeCallback)
	{
		menu->SetActive(true);
		openMenus.push({ menu, closeCallback });

		if (GetOpenMenusCount() == 1)
			globalOpenEvent.Invoke();
	}

	void CloseTopMenu()
	{
		if (GetOpenMenusCount() == 0)
			return;

		Menu& menu = openMenus.top();
		menu.gameObject->SetActive(false);

		if (menu.closeCallback)
			menu.closeCallback();

		openMenus.pop();

		if (GetOpenMenusCount() == 0)
			globalCloseEvent.Invoke();
	}

	void CloseAllMenus()
	{
		if (GetOpenMenusCount() == 0)
			return;

		while (GetOpenMenusCount() > 0)
		{
			Menu& menu = openMenus.top();
			menu.gameObject->SetActive(false);

			if (menu.closeCallback)
				menu.closeCallback();

			openMenus.pop();
		}

		globalCloseEvent.Invoke();
	}

	int GetOpenMenusCount()
	{
		return openMenus.size();
	}

	void OnFirstMenuOpened(std::function<void()>& callback)
	{
		globalOpenEvent.Subscribe(callback);
	}

	void RemoveOnFirstOpen(std::function<void()>& callback)
	{
		globalOpenEvent.Unsubscribe(callback);
	}

	void OnLastMenuClosed(std::function<void()>& callback)
	{
		globalCloseEvent.Subscribe(callback);
	}

	void RemoveOnLastClose(std::function<void()>& callback)
	{
		globalCloseEvent.Unsubscribe(callback);
	}
}