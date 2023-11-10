#pragma once

#include <VkMana/Image.hpp>

namespace VkMana::Sample
{
	class Renderer;

	class Material : public IntrusivePtrEnabled<Material>
	{
	public:
		~Material() = default;

		void SetAlbedoTexture(const ImageHandle& image) { m_albedoTexture = image; }
		void SetNormalTexture(const ImageHandle& image) { m_normalTexture = image; }

	private:
		friend class Renderer;

		Material(Renderer* renderer)
			: m_renderer(renderer)
		{
		}

	private:
		Renderer* m_renderer;

		ImageHandle m_albedoTexture = nullptr;
		ImageHandle m_normalTexture = nullptr;
	};
	using MaterialHandle = IntrusivePtr<Material>;

} // namespace VkMana::Sample