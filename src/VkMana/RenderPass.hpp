#pragma once

#include "Image.hpp"

namespace VkMana
{
    struct RenderPassTarget
    {
        const ImageView* pImage = nullptr;
        bool isDepthStencil = false;
        bool clear = true;
        bool store = true;
        std::array<float, 4> clearValue{};                        // Color=R,G,B,A, Depth/Stencil=Depth,Stencil,N/A,N/A
        vk::ImageLayout preLayout = vk::ImageLayout::eUndefined;  // Layout to transition image from before pass.
        vk::ImageLayout postLayout = vk::ImageLayout::eUndefined; // Layout to transition image from after pass.

        static auto DefaultColorTarget(const ImageView* Image)
        {
            return RenderPassTarget{
                .pImage = Image,
                .isDepthStencil = false,
                .clear = true,
                .store = true,
                .clearValue = { 0.0f, 0.0f, 0.0f, 1.0f },
                .preLayout = vk::ImageLayout::eUndefined,
                .postLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
            };
        }

        static auto DefaultDepthStencilTarget(const ImageView* Image)
        {
            return RenderPassTarget{
                .pImage = Image,
                .isDepthStencil = true,
                .clear = true,
                .store = false,
                .clearValue = { 1.0f, 0.0f, 0.0f, 0.0f },
                .preLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal,
                .postLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal,
            };
        }
    };

    struct RenderPassInfo
    {
        std::vector<RenderPassTarget> targets; // Color + Depth/Stencil
    };
} // namespace VkMana