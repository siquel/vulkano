#include <vulkan/vulkan.h>
#include "window.h"
#include <memory>
#include <iostream>
#include "trace.h"
#include "vulkan_util.h"
#include "siq.h"
#include <vector>
#include <algorithm>
#include <array>

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

// TODO move this
VkResult createInstance(VkInstance* to) {
	VkApplicationInfo appInfo;
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = nullptr;
	appInfo.pApplicationName = "TopKek";
	appInfo.applicationVersion = 1;
	appInfo.engineVersion = 1;
	appInfo.pEngineName = "TopKek";
	appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 4); // NVIDIA drivers have 1.0.4 support

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
	VkResult error = vkEnumeratePhysicalDevices(instance, &gpuCount, nullptr);

	if (error) {
		SIQ_TRACE("vkEnumeratePhysicalDevices failed: %s", siq::vkResultToString(error));
		exit(1);
	}

	SIQ_TRACE("Found %d physical devices", gpuCount);
	// no devices
	if (!gpuCount) return;

	std::vector<VkPhysicalDevice> devices;
	devices.resize(gpuCount);
	// fill the devices vector
	vkEnumeratePhysicalDevices(instance, &gpuCount, devices.data());

	for (VkPhysicalDevice physicalDevice : devices) {
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(physicalDevice, &properties);
		SIQ_TRACE("--- Device: %s of type %s, Supports Vulkan API version: %d.%d.%d",
			properties.deviceName, 
			siq::vkDeviceTypeToString(properties.deviceType),
			VK_VERSION_MAJOR(properties.apiVersion),
			VK_VERSION_MINOR(properties.apiVersion),
			VK_VERSION_PATCH(properties.apiVersion));
	}

	VkPhysicalDevice physicalDevice = devices[0];

	// Get device queue that supports gfx operations VK_QUEUE_GRAPHICS_BIT
	uint32_t graphicsQueueIndex{ 0 };
	uint32_t queueCount{ 0 };
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, nullptr);

	if (!queueCount) {
		SIQ_TRACE("Cannot find queue!");
		exit(1);
	}

	std::vector<VkQueueFamilyProperties> queueProperties;
	queueProperties.resize(queueCount);
	// fill the vector
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, queueProperties.data());
	// get the index
	auto search = std::find_if(std::begin(queueProperties), std::end(queueProperties), [](const VkQueueFamilyProperties& prop) {
		return prop.queueFlags & VK_QUEUE_GRAPHICS_BIT;
	});
	if (search == std::end(queueProperties)) {
		SIQ_TRACE("The device doesn't have VK_QUEUE_GRAPHICS_BIT set!");
		exit(1);
	}
	graphicsQueueIndex = static_cast<uint32_t>(std::distance(std::begin(queueProperties), search));
	SIQ_TRACE("Using graphicsQueueIndex %d", graphicsQueueIndex);

	// create the queue
	std::array<float, 1> queuePriorities = { 0.f };
	VkDeviceQueueCreateInfo queueCreateInfo = {};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = graphicsQueueIndex;
	queueCreateInfo.queueCount = 1;
	queueCreateInfo.pQueuePriorities = queuePriorities.data();

	// Create the actual device now

	static const char* EnabledExtensions[] = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pNext = nullptr;
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
	deviceCreateInfo.pEnabledFeatures = nullptr;

	if (COUNTOF(EnabledExtensions) > 0) {
		deviceCreateInfo.enabledExtensionCount = COUNTOF(EnabledExtensions);
		deviceCreateInfo.ppEnabledExtensionNames = EnabledExtensions;
	}

	SIQ_TRACE("Creating VkDevice....");

	VkDevice device;

	error = vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device);

	if (error) {
		SIQ_TRACE("Failed to create device: %s", siq::vkResultToString(error));
		exit(1);
	}
	else {
		SIQ_TRACE("Success...");
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

	VkInstance instance;
	VkResult error; 
	if ((error = createInstance(&instance))) {
		SIQ_TRACE("failed to create vulkan instance: %s", siq::vkResultToString(error));
		return 1;
	}

	queryDevices(instance);
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