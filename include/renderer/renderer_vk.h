#pragma once

#include "renderer_context.h"

namespace siq {
	namespace vk {

		RendererContext* createRenderer();
		void destroyRenderer();
	}
}