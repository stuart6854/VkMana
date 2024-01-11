#pragma once

#include "GPUResource.hpp"
#include "Util/IntrusivePtr.hpp"
#include "VulkanHeaders.hpp"

#include <memory>
#include <string_view>

namespace VkMana
{
    template <typename T>
    void SetObjectDebugName(vk::Device device, T handle, const char* pName)
    {
        static_assert(vk::isVulkanHandleType<T>::value == true);

        vk::DebugUtilsObjectNameInfoEXT nameInfo{};
        nameInfo.objectType = handle.objectType;
        nameInfo.objectHandle = reinterpret_cast<uint64_t>(static_cast<T::CType>(handle));
        nameInfo.pObjectName = pName;
        device.setDebugUtilsObjectNameEXT(nameInfo);
    }

    template <class T>
    inline void HashCombine(std::size_t& seed, const T& v)
    {
        std::hash<T> hasher;
        seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    inline bool FormatIsUndefined(vk::Format format) { return format == vk::Format::eUndefined; }

    inline bool FormatIsDepthOrStencil(vk::Format format)
    {
        switch(format)
        {
        case vk::Format::eD16Unorm:
        case vk::Format::eX8D24UnormPack32:
        case vk::Format::eD32Sfloat:
        case vk::Format::eS8Uint:
        case vk::Format::eD16UnormS8Uint:
        case vk::Format::eD24UnormS8Uint:
        case vk::Format::eD32SfloatS8Uint:
            return true;
        default:
            return false;
        }
    }

    inline bool FormatIsDepthAndStencil(vk::Format format)
    {
        switch(format)
        {
        case vk::Format::eD16UnormS8Uint:
        case vk::Format::eD24UnormS8Uint:
        case vk::Format::eD32SfloatS8Uint:
            return true;
        default:
            return false;
        }
    }

    inline bool FormatIsDepthOnly(vk::Format format)
    {
        switch(format)
        {
        case vk::Format::eD16Unorm:
        case vk::Format::eX8D24UnormPack32:
        case vk::Format::eD32Sfloat:
            return true;
        default:
            return false;
        }
    }

    inline bool FormatIsStencilOnly(vk::Format format)
    {
        switch(format)
        {
        case vk::Format::eS8Uint:
            return true;
        default:
            return false;
        }
    }

    inline bool FormatHasDepth(vk::Format format) { return FormatIsDepthOnly(format) || FormatIsDepthAndStencil(format); }

    inline bool FormatHasStencil(vk::Format format) { return FormatIsStencilOnly(format) || FormatIsDepthAndStencil(format); }

    inline bool FormatIsColor(vk::Format format) { return !(FormatIsUndefined(format) || FormatIsDepthOrStencil(format)); }

} // namespace VkMana

#define UNUSED(x) (void(x))