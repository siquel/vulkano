#pragma once
#include <windows.h>

namespace siq {
	class Window {
	private:
		int width{ 0 };
		int height{ 0 };
		bool fullscreen{ false };
		HWND window;
		HINSTANCE windowInstance;
	public:
		Window(int w, int h, HINSTANCE hInstance, WNDPROC wndproc);
		void show();
		void handleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	};
}