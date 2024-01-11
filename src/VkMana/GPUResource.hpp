#pragma once

#include "Util/IntrusivePtr.hpp"
#include "VulkanHeaders.hpp"

#include <fmt/format.h>

#include <string>

namespace VkMana
{
    class Context;

    template <typename T>
    class GPUResource : public IntrusivePtrEnabled<T>
    {
    public:
        explicit GPUResource(Context* pContext)
            : m_pContext(pContext)
        {
        }
        virtual ~GPUResource() = default;

        virtual void SetDebugName(const std::string& name) = 0;

        auto GetContext() const -> auto { return m_pContext; }

    private:
        Context* m_pContext;
    };

} // namespace VkMana