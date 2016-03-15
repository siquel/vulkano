#include "renderer/renderer_vk.h"
#include <vulkan/vulkan.h>
#include "siq.h"
#include "vulkan_util.h"
#include "trace.h"
#include <vector>
#include <algorithm>
#include <array>

#define GET_INSTANCE_PROC_ADDR(inst, entrypoint)                                        \
{                                                                                       \
    fp##entrypoint = (PFN_vk##entrypoint) vkGetInstanceProcAddr(inst, "vk"#entrypoint); \
    if (fp##entrypoint == NULL) {                                                       \
        SIQ_TRACE("vkGetInstanceProcAddr failed to find vk" #entrypoint);               \
		return false;                                                                   \
    }                                                                                   \
}

#define GET_DEVICE_PROC_ADDR(dev, entrypoint)                           \
{                                                                                       \
    fp##entrypoint = (PFN_vk##entrypoint) vkGetDeviceProcAddr(dev, "vk"#entrypoint);    \
    if (fp##entrypoint == NULL) {                                                       \
        SIQ_TRACE("vkGetDeviceProcAddr failed to find vk" #entrypoint);                 \
		return false;                                                                   \
    }                                                                                   \
}


namespace siq {

	namespace vk {

		struct RendererContextVulkan : public RendererContext {
			VkInstance instance{ nullptr };
			VkPhysicalDevice physicalDevice{ nullptr };
			VkDevice device{ nullptr };
			VkQueue queue{ nullptr };
			VkSurfaceKHR surface{ nullptr };
			PFN_vkGetPhysicalDeviceSurfaceSupportKHR fpGetPhysicalDeviceSurfaceSupportKHR{ nullptr };
			PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR fpGetPhysicalDeviceSurfaceCapabilitiesKHR{ nullptr };
			PFN_vkGetPhysicalDeviceSurfaceFormatsKHR fpGetPhysicalDeviceSurfaceFormatsKHR{ nullptr };
			PFN_vkGetPhysicalDeviceSurfacePresentModesKHR fpGetPhysicalDeviceSurfacePresentModesKHR{ nullptr };
			PFN_vkCreateSwapchainKHR fpCreateSwapchainKHR{ nullptr };
			PFN_vkDestroySwapchainKHR fpDestroySwapchainKHR{ nullptr };
			PFN_vkGetSwapchainImagesKHR fpGetSwapchainImagesKHR{ nullptr };
			PFN_vkAcquireNextImageKHR fpAcquireNextImageKHR{ nullptr };
			PFN_vkQueuePresentKHR fpQueuePresentKHR{ nullptr };
			VkFormat surfaceFormat{ VK_FORMAT_UNDEFINED };
			VkCommandPool commandPool{ nullptr };
			VkCommandBuffer commandBuffer{ nullptr };
			const char* Name{ "TopKek" };
			
			RendererContextVulkan() {

			}
			~RendererContextVulkan() override {}

			bool init() {

				VkApplicationInfo appInfo;
				appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
				appInfo.pNext = nullptr;
				appInfo.pApplicationName = Name;
				appInfo.pEngineName = Name;
				appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 4); // NVIDIA drivers have 1.0.4 support

				// enable extensions
				static const char* InstanceEnabledExtensions[] = {
					VK_KHR_SURFACE_EXTENSION_NAME,
					VK_KHR_WIN32_SURFACE_EXTENSION_NAME
				};

				VkInstanceCreateInfo instanceInfo;
				instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
				instanceInfo.pNext = NULL;
				instanceInfo.pApplicationInfo = &appInfo;
				// hmm validation? 
				instanceInfo.enabledLayerCount = 0;
				instanceInfo.ppEnabledLayerNames = NULL;

				if (COUNTOF(InstanceEnabledExtensions) > 0) {
					instanceInfo.enabledExtensionCount = COUNTOF(InstanceEnabledExtensions);
					instanceInfo.ppEnabledExtensionNames = InstanceEnabledExtensions;
				}

				VkResult error = vkCreateInstance(&instanceInfo, nullptr, &instance);

				if (error) {
					SIQ_TRACE("failed to create Vulkan instance: %s", siq::vkResultToString(error));
					return false;
				}

				uint32_t gpuCount{ 0 };
				error = vkEnumeratePhysicalDevices(instance, &gpuCount, nullptr);

				if (error) {
					SIQ_TRACE("vkEnumeratePhysicalDevices failed: %s", siq::vkResultToString(error));
					return false;
				}

				SIQ_TRACE("Found %d physical devices", gpuCount);
				// no devices
				if (!gpuCount) return false;

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

				physicalDevice = devices[0];

				// Get device queue that supports gfx operations VK_QUEUE_GRAPHICS_BIT
				uint32_t graphicsQueueIndex{ 0 };
				uint32_t queueCount{ 0 };
				vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, nullptr);

				if (!queueCount) {
					SIQ_TRACE("Cannot find queue!");
					return false;
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
					return false;
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

				static const char* DeviceEnabledExtensions[] = {
					VK_KHR_SWAPCHAIN_EXTENSION_NAME
				};

				VkDeviceCreateInfo deviceCreateInfo = {};
				deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
				deviceCreateInfo.pNext = nullptr;
				deviceCreateInfo.queueCreateInfoCount = 1;
				deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
				deviceCreateInfo.pEnabledFeatures = nullptr;

				if (COUNTOF(DeviceEnabledExtensions) > 0) {
					deviceCreateInfo.enabledExtensionCount = COUNTOF(DeviceEnabledExtensions);
					deviceCreateInfo.ppEnabledExtensionNames = DeviceEnabledExtensions;
				}

				SIQ_TRACE("Creating VkDevice....");

				error = vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device);

				if (error) {
					SIQ_TRACE("Failed to create device: %s", siq::vkResultToString(error));
					return false;
				}
				else {
					SIQ_TRACE("Success...");
				}

				// retrieve the gfx queue
				vkGetDeviceQueue(device, graphicsQueueIndex, 0, &queue);

				// get the func pointers
				GET_INSTANCE_PROC_ADDR(instance, GetPhysicalDeviceSurfaceCapabilitiesKHR);
				GET_INSTANCE_PROC_ADDR(instance, GetPhysicalDeviceSurfaceFormatsKHR);
				GET_INSTANCE_PROC_ADDR(instance, GetPhysicalDeviceSurfacePresentModesKHR);
				GET_INSTANCE_PROC_ADDR(instance, GetPhysicalDeviceSurfaceSupportKHR);
				GET_INSTANCE_PROC_ADDR(instance, CreateSwapchainKHR);
				GET_INSTANCE_PROC_ADDR(instance, DestroySwapchainKHR);
				GET_INSTANCE_PROC_ADDR(instance, GetSwapchainImagesKHR);
				GET_INSTANCE_PROC_ADDR(instance, AcquireNextImageKHR);
				GET_INSTANCE_PROC_ADDR(instance, QueuePresentKHR);

				if (!initSwapchain()) {
					SIQ_TRACE("initSwapchain failed");
					return false;
				}

				VkCommandPoolCreateInfo cmdPoolInfo = {};
				cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
				cmdPoolInfo.pNext = nullptr;
				cmdPoolInfo.queueFamilyIndex = graphicsQueueIndex;
				cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

				error = vkCreateCommandPool(device, &cmdPoolInfo, nullptr, &commandPool);

				if (error) {
					SIQ_TRACE("vkCreateCommandPool failed: %s", siq::vkResultToString(error));
					return false;
				}

				VkCommandBufferAllocateInfo cmd;
				cmd.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
				cmd.pNext = NULL;
				cmd.commandPool = commandPool;
				cmd.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
				cmd.commandBufferCount = 1;

				error = vkAllocateCommandBuffers(device, &cmd, &commandBuffer);

				if (error) {
					SIQ_TRACE("vkAllocateCommandBuffers failed: %s", siq::vkResultToString(error));
					return false;
				}
				return true;
			}

			bool initSwapchain() {
				VkResult error;
				VkWin32SurfaceCreateInfoKHR surfaceCreateInfo;
				surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
				surfaceCreateInfo.hinstance = PlatformData::getInstance().hinstance;
				surfaceCreateInfo.hwnd = PlatformData::getInstance().hwnd;
				error = vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface);

				if (error) {
					SIQ_TRACE("Failed to create VkSurface: %s", siq::vkResultToString(error));
					return false;
				}

				// Get the list of VkFormat's that are supported:
				uint32_t formatCount{ 0 };
				error = fpGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
				if (error) {
					SIQ_TRACE("fpGetPhysicalDeviceSurfaceFormatsKHR failed: %s", siq::vkResultToString(error));
					return false;
				}

				std::vector<VkSurfaceFormatKHR> surfaceFormats;
				surfaceFormats.resize(formatCount);
				error = fpGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, surfaceFormats.data());
				if (error) {
					SIQ_TRACE("fpGetPhysicalDeviceSurfaceFormatsKHR failed: %s", siq::vkResultToString(error));
					return false;
				}

				// If the format list includes just one entry of VK_FORMAT_UNDEFINED,
				// the surface has no preferred format.  Otherwise, at least one
				// supported format will be returned.
				if (formatCount == 1 && surfaceFormats[0].format == VK_FORMAT_UNDEFINED) {
					surfaceFormat = VK_FORMAT_B8G8R8A8_UNORM;
				}
				else {
					surfaceFormat = surfaceFormats[0].format;
				}

				return true;
			}

			void shutdown() {

			}
		};



		static RendererContextVulkan* renderer{ nullptr };

		RendererContext* createRenderer() {
			renderer = new RendererContextVulkan;
			if (!renderer->init()) {
				delete renderer;
				renderer = nullptr;
			}
			return renderer;
		}

		void destroyRenderer() {
			renderer->shutdown();
			delete renderer;
			renderer = nullptr;
		}

	}
}