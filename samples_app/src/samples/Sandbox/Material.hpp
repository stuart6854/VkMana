#pragma once

#include <VkMana/Image.hpp>

namespace VkMana::SamplesApp
{
	class Renderer;

	class Material : public IntrusivePtrEnabled<Material>
	{
	public:
		~Material() = default;

		void SetAlbedoTexture(const ImageHandle& image) { m_albedoTexture = image; }
		void SetNormalTexture(const ImageHandle& image) { m_normalTexture = image; }

		auto GetAlbedoTexture() -> auto { return m_albedoTexture.Get(); }
		auto GetNormalTexture() -> auto { return m_normalTexture.Get(); }

	private:
		friend class Renderer;

		explicit Material(Renderer* renderer)
			: m_renderer(renderer)
		{
		}

		auto GetTextures() const -> std::vector<ImageHandle>
		{
			std::vector<ImageHandle> textures;
			if (m_albedoTexture)
				textures.push_back(m_albedoTexture);
			if (m_normalTexture)
				textures.push_back(m_normalTexture);

			return textures;
		}

	private:
		Renderer* m_renderer;

		ImageHandle m_albedoTexture = nullptr;
		ImageHandle m_normalTexture = nullptr;
	};
	using MaterialHandle = IntrusivePtr<Material>;

} // namespace VkMana::SamplesApp