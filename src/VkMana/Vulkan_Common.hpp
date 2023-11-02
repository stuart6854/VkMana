#pragma once

#include "Vulkan_Headers.hpp"

namespace VkMana
{
	enum QueueIndex
	{
		QueueIndex_Graphics = 0,
		QueueIndex_Compute,
		QueueIndex_Transfer,
		QueueIndex_Count,
	};

} // namespace VkMana
