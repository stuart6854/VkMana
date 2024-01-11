#pragma once

#include "Image.hpp"

namespace VkMana
{
    struct RenderPassTarget
    {
        const ImageView* Image = nullptr;
        bool IsDepthStencil = false;
        bool Clear = true;
        bool Store = true;
        std::array<float, 4> ClearValue{};                        // Color=R,G,B,A, Depth/Stencil=Depth,Stencil,N/A,N/A
        vk::ImageLayout PreLayout = vk::ImageLayout::eUndefined;  // Layout to transition image from before pass.
        vk::ImageLayout PostLayout = vk::ImageLayout::eUndefined; // Layout to transition image from after pass.

        static auto DefaultColorTarget(const ImageView* Image)
        {
            return RenderPassTarget{
                .Image = Image,
                .IsDepthStencil = false,
                .Clear = true,
                .Store = true,
                .ClearValue = {0.0f, 0.0f, 0.0f, 1.0f},
                .PreLayout = vk::ImageLayout::eUndefined,
                .PostLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
            };
        }

        static auto DefaultDepthStencilTarget(const ImageView* Image)
        {
            return RenderPassTarget{
                .Image = Image,
                .IsDepthStencil = true,
                .Clear = true,
                .Store = false,
                .ClearValue = {1.0f, 0.0f, 0.0f, 0.0f},
                .PreLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal,
                .PostLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal,
            };
        }
    };

    struct RenderPassInfo
    {
        std::vector<RenderPassTarget> Targets; // Color + Depth/Stencil
    };
} // namespace VkMana