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

void queryDevices(VkInstance instance) {
	uint32_t gpuCount{ 0 };
	VkResult error;
	vkEnumeratePhysicalDevices(instance, &gpuCount, nullptr);
}

const char* vkResultToString(const VkResult r) {
#define TOSTR(r) case VK_##r: return #r;
	switch (r) {
		TOSTR(SUCCESS);
		TOSTR(NOT_READY);
		TOSTR(TIMEOUT);
		TOSTR(EVENT_SET);
		TOSTR(EVENT_RESET);
		TOSTR(INCOMPLETE);
		TOSTR(ERROR_OUT_OF_HOST_MEMORY);
		TOSTR(ERROR_OUT_OF_DEVICE_MEMORY);
		TOSTR(ERROR_INITIALIZATION_FAILED);
		TOSTR(ERROR_DEVICE_LOST);
		TOSTR(ERROR_MEMORY_MAP_FAILED);
		TOSTR(ERROR_LAYER_NOT_PRESENT);
		TOSTR(ERROR_EXTENSION_NOT_PRESENT);
		TOSTR(ERROR_FEATURE_NOT_PRESENT);
		TOSTR(ERROR_INCOMPATIBLE_DRIVER);
		TOSTR(ERROR_TOO_MANY_OBJECTS);
		TOSTR(ERROR_FORMAT_NOT_SUPPORTED);
		TOSTR(ERROR_SURFACE_LOST_KHR);
		TOSTR(ERROR_NATIVE_WINDOW_IN_USE_KHR);
		TOSTR(SUBOPTIMAL_KHR);
		TOSTR(ERROR_OUT_OF_DATE_KHR);
		TOSTR(ERROR_INCOMPATIBLE_DISPLAY_KHR);
		TOSTR(ERROR_VALIDATION_FAILED_EXT);
		TOSTR(ERROR_INVALID_SHADER_NV);
	default:
		return "UNKOWN_ERROR";
	}
#undef TOSTR
}
namespace detail {
	inline int32_t vsnprintf(char* _str, size_t _count, const char* _format, va_list _argList) {
		int32_t len = ::vsnprintf_s(_str, _count, size_t(-1), _format, _argList);
		return -1 == len ? ::_vscprintf(_format, _argList) : len;
	}
}

void trace(const char* path, uint16_t line, const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	char temp[8192];
	char* out = temp;
	int32_t len = detail::vsnprintf(out, sizeof(temp), fmt, args);
	out[len] = '\0';
	va_end(args);
	std::cout << out;
}

#define _SIQ_TRACE(fmt, ...)                                               \
	for(;;) {                                                              \
		trace(__FILE__, uint16_t(__LINE__), fmt "\n", ##__VA_ARGS__);      \
	break;}                                                                \

#define SIQ_TRACE _SIQ_TRACE

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

	VkInstance instance;
	VkResult error; 
	if ((error = createInstance(&instance))) {
		SIQ_TRACE("failed to create vulkan instance: %s", vkResultToString(error));
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