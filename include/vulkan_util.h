#pragma once
#include "vulkan\vulkan.h"

#define TOSTR(r) case VK_##r: return #r;
namespace siq {
	const char* vkResultToString(const VkResult r) {
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
	}
}
#undef TOSTR