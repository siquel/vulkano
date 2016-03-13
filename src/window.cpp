#include "window.h"
#include <iostream>

namespace siq {
	static const char* WndName = "window";
	static const char* Title = "TopKek";

	Window::Window(int w, int h, HINSTANCE hInstance, WNDPROC wndproc)
		: width(w), height(h), windowInstance(hInstance) {
		WNDCLASSEX wndClass;

		wndClass.cbSize = sizeof(WNDCLASSEX);
		wndClass.style = CS_HREDRAW | CS_VREDRAW;
		wndClass.lpfnWndProc = wndproc;
		wndClass.cbClsExtra = 0;
		wndClass.cbWndExtra = 0;
		wndClass.hInstance = hInstance;
		wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		wndClass.lpszMenuName = NULL;
		wndClass.lpszClassName = WndName;
		wndClass.hIconSm = LoadIcon(NULL, IDI_WINLOGO);

		if (!RegisterClassEx(&wndClass)) {
			std::cout << "Could not register window" << std::endl;
			exit(1);
		}
	}

	void Window::show() {
		DWORD dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;;
		DWORD dwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

		RECT windowRect;

		int screenWidth = GetSystemMetrics(SM_CXSCREEN);
		int screenHeight = GetSystemMetrics(SM_CYSCREEN);

		// center the window
		windowRect.left = (long)screenWidth / 2 - width / 2;
		windowRect.right = (long)width;
		windowRect.top = (long)screenHeight / 2 - height / 2;
		windowRect.bottom = (long)height;

		AdjustWindowRectEx(&windowRect, dwStyle, FALSE, dwExStyle);

		window = CreateWindowEx(0,
			WndName,
			Title,
			dwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
			windowRect.left,
			windowRect.top,
			windowRect.right,
			windowRect.bottom,
			NULL,
			NULL,
			windowInstance,
			NULL);

		if (!window) {
			std::cout << "Could not create window" << std::endl;
			exit(1);
		}

		ShowWindow(window, SW_SHOW);
		SetForegroundWindow(window);
		SetFocus(window);
	}

	void Window::handleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		switch (uMsg) {
		case WM_CLOSE:
			DestroyWindow(hWnd);
			PostQuitMessage(0);
			break;
		}
	}

}