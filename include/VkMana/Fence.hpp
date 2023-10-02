#pragma once

#include <memory>

namespace VkMana
{
	class GraphicsDevice;

	class Fence
	{
	public:
		Fence(GraphicsDevice& graphicsDevice);
		~Fence();

		auto GetImpl() const -> auto* { return m_impl.get(); }

	private:
		class Impl;
		std::unique_ptr<Impl> m_impl;
	};
} // namespace VkMana
