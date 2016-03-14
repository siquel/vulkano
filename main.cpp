#include <vulkan/vulkan.h>
#include "window.h"
#include <memory>
#include <iostream>
#include "trace.h"
#include "vulkan_util.h"
#include "siq.h"
#include <vector>
#include "renderer/renderer_context.h"
#include "renderer/renderer_vk.h"

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

void openConsole() {
	AllocConsole();
	AttachConsole(GetCurrentProcessId());
	// redirect to stdout 
	freopen("CON", "w", stdout);
	SetConsoleTitle(TEXT("topkek"));
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow) {
	openConsole();

	window = std::make_unique<siq::Window>(1280, 720, hInstance, WndProc);
	window->show();

	siq::PlatformData::getInstance().hwnd = window->getHandle();
	siq::PlatformData::getInstance().hinstance = hInstance;

	siq::RendererContext* vulkan = siq::vk::createRenderer();
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