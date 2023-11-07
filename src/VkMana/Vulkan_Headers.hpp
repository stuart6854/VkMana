#pragma once

#if defined(_WIN32) && !defined(VK_USE_PLATFORM_WIN32)
	#define VK_USE_PLATFORM_WIN32_KHR
#endif

#if defined(VULKAN_H_) || defined(VULKAN_CORE_H_)
	#error "Must include vulkan_headers.hpp before Vulkan headers"
#endif

#ifndef VULKAN_DEBUG
	#ifdef _DEBUG
		#define VULKAN_DEBUG
	#endif
#endif

#include <vulkan/vulkan.h>
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>
#include <vma/vk_mem_alloc.h>
#include <vk_mem_alloc.hpp>
#include "Logging.hpp"
#include <vulkan/utility/vk_format_utils.h>

#ifdef VULKAN_DEBUG
	#define VK_ASSERT(x)                                                \
		do                                                              \
		{                                                               \
			if (!bool(x))                                               \
			{                                                           \
				LOG_ERR("Vulkan error at {}:{}." m __FILE__, __LINE__); \
				abort();                                                \
			}                                                           \
		}                                                               \
		while (false)
#else
	#define VK_ASSERT(x) ((void)0)
#endif