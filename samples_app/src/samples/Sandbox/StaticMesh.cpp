#include "StaticMesh.hpp"

#include "Renderer.hpp"

#include <tiny_obj_loader.h>

namespace VkMana::SamplesApp
{
	bool StaticMesh::LoadFromFile(const std::string& filename)
	{
		std::vector<StaticVertex> vertices;
		std::vector<uint16_t> indices;

		tinyobj::ObjReader objReader;
		if (!objReader.ParseFromFile(filename))
		{
			VM_ERR("{}", objReader.Error());
			if (!objReader.Warning().empty())
				VM_WARN("{}", objReader.Warning());
			return false;
		}

		std::unordered_map<StaticVertex, uint16_t> uniqueVertices{};

		const auto& shapes = objReader.GetShapes();
		const auto& attrib = objReader.GetAttrib();
		for (const auto& shape : shapes)
		{
			for (const auto& index : shape.mesh.indices)
			{
				StaticVertex vertex{};
				vertex.Position = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2],
				};
				vertex.Normal = {
					attrib.normals[3 * index.normal_index + 0],
					attrib.normals[3 * index.normal_index + 1],
					attrib.normals[3 * index.normal_index + 2],
				};
				vertex.TexCoord = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					attrib.texcoords[2 * index.texcoord_index + 1],
				};
				if (!uniqueVertices.contains(vertex))
				{
					uniqueVertices[vertex] = uint16_t(vertices.size());
					vertices.push_back(vertex);
				}

				indices.push_back(uniqueVertices[vertex]);
			}
		}

		SetVertices(vertices);
		SetTriangles(indices);

		return true;
	}

	void StaticMesh::SetVertices(const std::vector<StaticVertex>& vertices)
	{
		const auto bufferInfo = BufferCreateInfo::Vertex(sizeof(StaticVertex) * vertices.size());
		const BufferDataSource initialData{
			.Size = bufferInfo.Size,
			.Data = vertices.data(),
		};
		m_vertexBuffer = m_renderer->GetContext()->CreateBuffer(bufferInfo, &initialData);
	}

	void StaticMesh::SetTriangles(const std::vector<uint16_t>& triangles)
	{
		const auto bufferInfo = BufferCreateInfo::Index(sizeof(uint16_t) * triangles.size());
		const BufferDataSource initialData{
			.Size = bufferInfo.Size,
			.Data = triangles.data(),
		};
		m_indexBuffer = m_renderer->GetContext()->CreateBuffer(bufferInfo, &initialData);
	}

	void StaticMesh::SetSubmeshes(const std::vector<Submesh>& submeshes)
	{
		m_submeshes = submeshes;
	}

	void StaticMesh::SetMaterials(const std::vector<MaterialHandle>& materials)
	{
		m_materials = materials;
	}

	StaticMesh::StaticMesh(Renderer* renderer)
		: m_renderer(renderer)
	{
	}

} // namespace VkMana::Sample