#pragma once

#include <memory>

namespace VkMana
{
	class Texture;

	struct FramebufferAttachment
	{
		std::shared_ptr<Texture> Target;
		std::uint32_t ArrayLayer;
		std::uint32_t MipLevel;
	};

	class Framebuffer
	{
	public:
		virtual auto GetColorTargets() const -> const FramebufferAttachment* = 0;
		virtual auto GetDepthTarget() const -> const FramebufferAttachment* = 0;

//		virtual auto GetOutputDescription() -> auto = 0;

		virtual auto GetWidth() const -> std::uint32_t = 0;
		virtual auto GetHeight() const -> std::uint32_t = 0;
	};
} // namespace VkMana
