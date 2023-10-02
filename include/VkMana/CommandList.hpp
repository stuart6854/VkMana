#pragma once

#include "Structs.hpp"
#include "Enums.hpp"

#include <memory>
#include <string>
#include <vector>

namespace VkMana
{
	class GraphicsDevice;
	class Framebuffer;
	class Pipeline;
	class DeviceBuffer;
	class Texture;
	class ResourceSet;

	class CommandList
	{
	public:
		explicit CommandList(GraphicsDevice& graphicsDevice);
		~CommandList();

		void Reset();

		void Begin();
		void End();

		void SetFramebuffer(Framebuffer& framebuffer);

		void ClearColorTarget(std::uint32_t index, RgbaFloat clearColor);
		void ClearDepthTarget(float depth);
		void ClearDepthStencilTarget(float depth, std::int32_t stencil = 0);

		void SetFullViewports();
		void SetFullViewport(std::uint32_t index);
		void SetViewport(std::uint32_t index, Viewport viewport);

		void SetFullScissorRects();
		void SetFullScissorRect(std::uint32_t index);
		void SetScissorRect(std::uint32_t index, std::uint32_t x, std::uint32_t y, std::uint32_t width, std::uint32_t height);

		void SetPipeline(Pipeline& pipeline);
		void SetVertexBuffer(std::uint32_t index, DeviceBuffer& buffer, std::uint32_t offsetBytes = 0);
		void SetIndexBuffer(DeviceBuffer& buffer, IndexFormat format, std::uint32_t offsetBytes = 0);

		void SetGraphicsResourceSet(std::uint32_t slot, ResourceSet& set, const std::vector<std::uint32_t>& dynamicOffsets);
		void SetComputeResourceSet(std::uint32_t slot, ResourceSet& set, const std::vector<std::uint32_t>& dynamicOffsets);

		void SetConstants(std::uint32_t offsetBytes, std::uint32_t sizeBytes, const void* data);

		void Draw(std::uint32_t vertexCount, std::uint32_t instanceCount, std::uint32_t vertexStart, std::uint32_t instanceStart);
		void DrawIndexed(std::uint32_t indexCount,
			std::uint32_t instanceCount = 1,
			std::uint32_t indexStart = 0,
			std::uint32_t vertexOffset = 0,
			std::uint32_t instanceStart = 0);
		// void DrawIndirect();
		// void DrawIndexedIndirect();

		void Dispatch(std::uint32_t groupCountX, std::uint32_t groupCountY, std::uint32_t groupCountZ);
		// void DispatchIndirect();

		// void ResolveTexture(Texture& source, Texture& destination);

		void UpdateBuffer(DeviceBuffer& buffer, std::uint32_t offsetBytes, std::uint32_t sizeBytes, const void* data);

		void CopyBuffer(DeviceBuffer& soruce,
			std::uint32_t sourceOffset,
			DeviceBuffer& destination,
			std::uint32_t destinationOffset,
			std::uint32_t sizeBytes);
		void CopyTexture(Texture& source, Texture& destination, std::uint32_t mipLevel = 0, std::uint32_t arrayLayer = 0);
		void CopyTexture(Texture& source,
			std::uint32_t srcX,
			std::uint32_t srcY,
			std::uint32_t srcZ,
			std::uint32_t srcMipLevel,
			std::uint32_t srcArrayLayer,
			Texture& destination,
			std::uint32_t dstX,
			std::uint32_t dstY,
			std::uint32_t dstZ,
			std::uint32_t dstMipLevel,
			std::uint32_t dstArrayLayer,
			std::uint32_t width,
			std::uint32_t height,
			std::uint32_t depth,
			std::uint32_t layerCount);

		void GenerateMipMaps(Texture& texture);

		void PushDebugGroup(const std::string& name);
		void PopDebugGroup();

		void InsertDebugMarker(const std::string& name);

		auto GetImpl() const -> auto* { return m_impl.get(); }

	private:
		class Impl;
		std::unique_ptr<Impl> m_impl;
	};
} // namespace VkMana
