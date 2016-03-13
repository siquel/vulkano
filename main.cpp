#include <vulkan/vulkan.h>
#include "window.h"
#include <memory>

std::unique_ptr<siq::Window> window;

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (window != nullptr) {
		window->handleMessages(hWnd, uMsg, wParam, lParam);
	}
	return (DefWindowProc(hWnd, uMsg, wParam, lParam));
}

void loop() {
	MSG msg;
	while (true) {
		PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
		if (msg.message == WM_QUIT) {
			break;
		}
		else {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow) {
	window = std::make_unique<siq::Window>(1280, 720, hInstance, WndProc);
	window->show();

	VkApplicationInfo appInfo;
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = NULL;
	appInfo.pApplicationName = "TopKek";
	appInfo.applicationVersion = 1;
	appInfo.engineVersion = 1;
	appInfo.pEngineName = "TopKek";
	appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 2); // looks like VK_MAKE_VERSION(1, 0, 5) fails..

	VkInstanceCreateInfo instanceInfo;
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.pNext = NULL;
	instanceInfo.flags = 0;
	instanceInfo.pApplicationInfo = &appInfo;
	instanceInfo.enabledExtensionCount = 0;
	instanceInfo.ppEnabledExtensionNames = NULL;
	instanceInfo.enabledLayerCount = 0;
	instanceInfo.ppEnabledLayerNames = NULL;
	
	VkResult error;
	VkInstance instance;
	error = vkCreateInstance(&instanceInfo, NULL, &instance);
	/*
	

	// create swap chain
	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo;
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.hinstance = hInstance;
	surfaceCreateInfo.hwnd = window->getHandle();
	error = vkCreateWin32SurfaceKHR()*/
	loop();
	return 0;
}