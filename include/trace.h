#pragma once
#include <wchar.h>
#include <stdarg.h>
#include <stdio.h>
#include <cstdint>
#include <iostream>

namespace siq {
	inline int32_t vsnprintf(char* _str, size_t _count, const char* _format, va_list _argList) {
		int32_t len = ::vsnprintf_s(_str, _count, size_t(-1), _format, _argList);
		return -1 == len ? ::_vscprintf(_format, _argList) : len;
	}


	void trace(const char* path, uint16_t line, const char* fmt, ...) {
		va_list args;
		va_start(args, fmt);
		char temp[8192];
		char* out = temp;
		int32_t len = siq::vsnprintf(out, sizeof(temp), fmt, args);
		out[len] = '\0';
		va_end(args);
		std::cout << out;
	}
}
#define _SIQ_TRACE(fmt, ...)                                               \
	for(;;) {                                                              \
		siq::trace(__FILE__, uint16_t(__LINE__), fmt "\n", ##__VA_ARGS__);      \
	break;}                                                                \

#define SIQ_TRACE _SIQ_TRACE