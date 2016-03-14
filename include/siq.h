#pragma once

// http://cnicholson.net/2011/01/stupid-c-tricks-a-better-sizeof_array/
namespace siq {
	template< typename T, size_t N >
	char(&SIZEOF_ARRAY_REQUIRES_ARRAY_ARGUMENT(T(&)[N]))[N];

	struct PlatformData {
		HWND hwnd;
		HINSTANCE hinstance;
		static PlatformData& getInstance() {
			static PlatformData instance{ 0 };
			return instance;
		}
	};
}

#define COUNTOF(x) sizeof(siq::SIZEOF_ARRAY_REQUIRES_ARRAY_ARGUMENT(x))