#pragma once

#include "Material.hpp"
#include "StaticMesh.hpp"

#include <VkMana/Context.hpp>

#include <glm/ext/matrix_float4x4.hpp>

#include <unordered_map>

namespace VkMana::SamplesApp
{
	class Renderer
	{
	public:
		Renderer() = default;
		~Renderer() = default;

		/* State */

		bool Init(WSI* mainWindow);

		void SetSceneCamera(const glm::mat4& projMatrix, const glm::mat4& viewMatrix);

		void Submit(StaticMesh* mesh, glm::mat4 transform);

		void Flush();

		/* Resources */

		auto CreateMaterial() -> MaterialHandle;
		auto CreateStaticMesh() -> StaticMeshHandle;

		/* Getters */

		auto GetContext() const -> auto { return m_ctx; }

	private:
		void SetupGBufferPass();
		void SetupCompositionPass();
		void SetupScreenPass();

		void GBufferPass(CmdBuffer& cmd);
		void CompositionPass(CmdBuffer& cmd);
		void ScreenPass(CmdBuffer& cmd);

		auto GetImageIndex(Image* image) -> uint32_t;
		auto GetMaterialIndex(const Material* material) -> uint32_t;

	private:
		ContextHandle m_ctx = nullptr;
		WSI* m_mainWindow = nullptr;
		glm::uvec2 m_mainWindowSize;

		ImageHandle m_whiteImage = nullptr;
		ImageHandle m_blackImage = nullptr;

		/* Pass Resources */

		SetLayoutHandle m_bindlessSetLayout = nullptr;
		DescriptorSetHandle m_bindlessSet = nullptr;

		ImageHandle m_depthTargetImage = nullptr;
		ImageHandle m_positionTargetImage = nullptr;
		ImageHandle m_normalTargetImage = nullptr;
		ImageHandle m_albedoTargetImage = nullptr;
		RenderPassInfo m_gBufferPass;

		SetLayoutHandle m_cameraSetLayout = nullptr;
		PipelineLayoutHandle m_gBufferPipelineLayout = nullptr;
		PipelineHandle m_gBufferStaticPipeline = nullptr;

		ImageHandle m_compositionTargetImage;
		RenderPassInfo m_compositionPass;

		SetLayoutHandle m_compositionSetLayout = nullptr;
		PipelineLayoutHandle m_compositionPipelineLayout = nullptr;
		PipelineHandle m_compositionPipeline = nullptr;

		SetLayoutHandle m_screenSetLayout = nullptr;
		PipelineLayoutHandle m_screenPipelineLayout = nullptr;
		PipelineHandle m_screenPipeline = nullptr;

		/* Uniform Data */

		struct CameraUniformData
		{
			glm::mat4 projMatrix = glm::mat4(1.0f);
			glm::mat4 viewMatrix = glm::mat4(1.0f);
		} m_cameraUniformData;
		BufferHandle m_cameraUniformBuffer = nullptr;

		struct PushConstantData
		{
			glm::mat4 modelMatrix = glm::mat4(1.0f);
			uint32_t albedoMapIndex = 0;
			uint32_t normalMapIndex = 0;
		} m_pushConstantData;

		/* Renderables */

		template <typename T>
		struct Instance
		{
			T* Mesh = nullptr;
			// uint32_t submesh = 0;
			// uint32_t MaterialIdx = 0;
			glm::mat4 Transform = glm::mat4(1.0f);
		};

		std::vector<Instance<StaticMesh>> m_staticInstances;

		std::unordered_map<const Image*, uint32_t> m_knownBindlessImages;
		std::vector<const ImageView*> m_bindlessImages;

		std::unordered_map<const Material*, uint32_t> m_knownMaterials;
		std::vector<const Material*> m_materials;
	};

} // namespace VkMana::Sample
