#include <vulkan/vulkan.h>
#include "window.h"
#include <memory>
#include <iostream>

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
// http://cnicholson.net/2011/01/stupid-c-tricks-a-better-sizeof_array/
namespace detail {
	template< typename T, size_t N >
	char(&SIZEOF_ARRAY_REQUIRES_ARRAY_ARGUMENT(T(&)[N]))[N];
}

#define COUNTOF(x) sizeof(detail::SIZEOF_ARRAY_REQUIRES_ARRAY_ARGUMENT(x))

// TODO move this
VkResult createInstance(VkInstance* to) {
	VkApplicationInfo appInfo;
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = nullptr;
	appInfo.pApplicationName = "TopKek";
	appInfo.applicationVersion = 1;
	appInfo.engineVersion = 1;
	appInfo.pEngineName = "TopKek";
	appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 3); // looks like VK_MAKE_VERSION(1, 0, 5) fails..

	// enable extensions
	static const char* EnabledExtensions[] = {
		VK_KHR_SURFACE_EXTENSION_NAME,
#if _WIN32
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#endif
	};

	VkInstanceCreateInfo instanceInfo;
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.pNext = NULL;
	instanceInfo.pApplicationInfo = &appInfo;
	// hmm validation? 
	instanceInfo.enabledLayerCount = 0;
	instanceInfo.ppEnabledLayerNames = NULL;

	if (COUNTOF(EnabledExtensions) > 0) {
		instanceInfo.enabledExtensionCount = COUNTOF(EnabledExtensions);
		instanceInfo.ppEnabledExtensionNames = EnabledExtensions;
	}

	return vkCreateInstance(&instanceInfo, nullptr, to);
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow) {
	window = std::make_unique<siq::Window>(1280, 720, hInstance, WndProc);
	window->show();

	VkInstance instance;
	VkResult error; 
	if ((error = createInstance(&instance))) {
		std::cout << "Failed to create vulkan instance" << error << std::endl;
		return 1;
	}
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