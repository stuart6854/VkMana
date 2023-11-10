#pragma once

#include "Material.hpp"

#include <VkMana/Buffer.hpp>

#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/gtx/hash.hpp>

#include <string>

namespace VkMana::Sample
{
	class Renderer;

	struct StaticVertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TexCoord;
		glm::vec3 Tangent;

		bool operator==(const StaticVertex& other) const
		{
			return Position == other.Position && Normal == other.Normal && TexCoord == other.TexCoord;
		}
	};

	class StaticMesh : public IntrusivePtrEnabled<StaticMesh>
	{
	public:
		struct Submesh
		{
			uint32_t IndexOffset = 0;
			uint32_t IndexCount = 0;
			uint32_t VertexOffset = 0;
			uint32_t VertexCount = 0;
			uint32_t MaterialIndex = 0;
		};

	public:
		~StaticMesh() = default;

		bool LoadFromFile(const std::string& filename);

		void SetVertices(const std::vector<StaticVertex>& vertices);
		void SetTriangles(const std::vector<uint16_t>& triangles);
		void SetSubmeshes(const std::vector<Submesh>& submeshes);
		void SetMaterials(const std::vector<MaterialHandle>& materials);

		auto GetSubmeshes() const -> const auto& { return m_submeshes; }
		auto GetMaterials() -> auto& { return m_materials; }

	private:
		friend class Renderer;

		explicit StaticMesh(Renderer* renderer);

		auto GetVertexBuffer() const -> auto { return m_vertexBuffer.Get(); }
		auto GetIndexBuffer() const -> auto { return m_indexBuffer.Get(); }

	private:
		Renderer* m_renderer;

		BufferHandle m_vertexBuffer = nullptr;
		BufferHandle m_indexBuffer = nullptr;
		std::vector<Submesh> m_submeshes;
		std::vector<MaterialHandle> m_materials;
	};
	using StaticMeshHandle = IntrusivePtr<StaticMesh>;

} // namespace VkMana::Sample

template <>
struct std::hash<VkMana::Sample::StaticVertex>
{
	auto operator()(VkMana::Sample::StaticVertex const& vertex) const noexcept -> size_t
	{
		return ((hash<glm::vec3>()(vertex.Position) ^ (hash<glm::vec3>()(vertex.Normal) << 1)) >> 1)
			^ (hash<glm::vec2>()(vertex.TexCoord) << 1);
	}
};