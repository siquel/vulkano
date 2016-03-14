#include "renderer/renderer_vk.h"
#include <vulkan/vulkan.h>
#include "siq.h"
#include "vulkan_util.h"
#include "trace.h"
#include <vector>
#include <algorithm>
#include <array>


namespace siq {
	namespace vk {
		
		struct RendererContextVulkan : public RendererContext {
			VkInstance instance{ nullptr };
			VkPhysicalDevice physicalDevice{ nullptr };
			VkDevice device{ nullptr };
			VkQueue queue{ nullptr };
			
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