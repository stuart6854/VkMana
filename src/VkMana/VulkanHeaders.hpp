#pragma once

#if defined(VKMANA_EXPORTS)
    #define VM_API __declspec(dllexport)
#else
    #define VM_API __declspec(dllimport)
#endif

#if defined(_WIN32) && !defined(VK_USE_PLATFORM_WIN32)
    #define VK_USE_PLATFORM_WIN32_KHR
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
#elif defined(__linux__) && !defined(VK_USE_PLATFORM_WAYLAND_KHR)
    #define VK_USE_PLATFORM_WAYLAND_KHR
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
#define VULKAN_HPP_STORAGE_SHARED
#if defined(VKMANA_EXPORTS)
    #define VULKAN_HPP_STORAGE_SHARED_EXPORT
#endif
#include <vk_mem_alloc.hpp>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_hash.hpp>

#include "Logging.hpp"

#ifdef VULKAN_DEBUG
    #define VK_ASSERT(x)                                                                                                                                       \
        do                                                                                                                                                     \
        {                                                                                                                                                      \
            if(!bool(x))                                                                                                                                       \
            {                                                                                                                                                  \
                LOG_ERR("Vulkan error at {}:{}." m __FILE__, __LINE__);                                                                                        \
                abort();                                                                                                                                       \
            }                                                                                                                                                  \
        } while(false)
#else
    #define VK_ASSERT(x) ((void)0)
#endif