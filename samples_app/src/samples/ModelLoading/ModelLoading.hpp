#pragma once

#include <VkMana/Pipeline.hpp>

#include "samples/SampleBase.hpp"

#include <glm/ext/matrix_float4x4.hpp>

namespace VkMana::SamplesApp
{
	class SampleModelLoading final : public SampleBase
	{
	public:
		explicit SampleModelLoading()
			: SampleBase("Model Loading")
		{
		}
		~SampleModelLoading() override = default;

		bool Onload(SamplesApp& app, Context& ctx) override;
		void OnUnload() override;

		void Tick(float deltaTime, SamplesApp& app, Context& ctx) override;

	private:
		struct Mesh
		{
			VkMana::BufferHandle VertexBuffer;
			VkMana::BufferHandle IndexBuffer;
			uint32_t IndexCount = 0;
		};
		static bool LoadObjMesh(Mesh& outMesh, VkMana::Context& ctx, const std::string& filename);
		static bool LoadTexture(VkMana::ImageHandle& outImage, VkMana::Context& ctx, const std::string& filename);

	private:
		ImageHandle m_depthTarget = nullptr;
		SetLayoutHandle m_setLayout = nullptr;
		PipelineHandle m_pipeline = nullptr;
		ImageHandle m_texture = nullptr;
		Mesh m_mesh;

		struct PushConstants
		{
			glm::mat4 projMatrix;
			glm::mat4 viewMatrix;
			glm::mat4 modelMatrix;
		} m_pushConsts{};
	};

} // namespace VkMana::SamplesApp
