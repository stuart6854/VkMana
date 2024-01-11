#pragma once

#include "VulkanCommon.hpp"

namespace VkMana
{
    class Context;

    class CommandPool : public IntrusivePtrEnabled<CommandPool>
    {
    public:
        ~CommandPool();

        auto RequestCmd() -> vk::CommandBuffer;
        void ResetPool();

    private:
        friend class Context;

        CommandPool(Context* context, uint32_t queueFamilyIndex);

    private:
        Context* m_ctx;
        vk::CommandPool m_pool;
        std::vector<vk::CommandBuffer> m_cmds;
        uint32_t m_index = 0;
    };

} // namespace VkMana
