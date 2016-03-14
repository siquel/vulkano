#pragma once

namespace siq {
	struct __declspec(novtable) RendererContext {
		virtual ~RendererContext() = 0;
	};
	inline RendererContext::~RendererContext() {}

}