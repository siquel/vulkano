#pragma once

// http://cnicholson.net/2011/01/stupid-c-tricks-a-better-sizeof_array/
namespace siq {
	template< typename T, size_t N >
	char(&SIZEOF_ARRAY_REQUIRES_ARRAY_ARGUMENT(T(&)[N]))[N];
}

#define COUNTOF(x) sizeof(siq::SIZEOF_ARRAY_REQUIRES_ARRAY_ARGUMENT(x))