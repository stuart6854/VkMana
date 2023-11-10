#include "ModelLoader.hpp"

#include <VkMana/Context.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_IMPLEMENTATION
#include <tiny_gltf.h>
#include <glm/gtc/type_ptr.hpp>

namespace VkMana::Sample
{
	namespace Utils
	{
		auto LoadGLTFImage(tinygltf::Image& gltfImage, Context& context) -> ImageHandle
		{
			std::vector<uint8_t> pixels;
			if (gltfImage.component == 3)
			{
				// Convert to RGBA (most devices don't support RGB formats)
				const size_t bufferSize = gltfImage.width * gltfImage.height * 4;
				pixels.resize(bufferSize);
				auto* rgba = pixels.data();
				const auto* rgb = &gltfImage.image[0];
				for (auto i = 0; i < gltfImage.width * gltfImage.height; ++i)
				{
					for (auto j = 0; j < 3; ++j)
					{
						rgba[j] = rgb[j];
					}
					rgba += 4;
					rgb += 3;
				}
			}
			else
			{
				pixels.assign(gltfImage.image.begin(), gltfImage.image.end());
			}

			const auto imageInfo = ImageCreateInfo::Texture(gltfImage.width, gltfImage.height);
			const ImageDataSource imageDataSrc{ .Size = pixels.size(), .Data = pixels.data() };
			return context.CreateImage(imageInfo, &imageDataSrc);
		}

		auto LoadGLTFImages(tinygltf::Model& gltfModel, Context& context) -> std::vector<ImageHandle>
		{
			std::vector<ImageHandle> images;
			for (auto& image : gltfModel.images)
			{
				images.push_back(LoadGLTFImage(image, context));
			}
			return images;
		}

		auto LoadGLTFMaterials(tinygltf::Model& gltfModel, Renderer& renderer, const std::vector<ImageHandle>& images)
			-> std::vector<MaterialHandle>
		{
			std::vector<MaterialHandle> materials;
			for (auto& mat : gltfModel.materials)
			{
				auto material = renderer.CreateMaterial();
				if (mat.values.contains("baseColorTexture"))
				{
					const auto imageIndex = gltfModel.textures[mat.values["baseColorTexture"].TextureIndex()].source;
					material->SetAlbedoTexture(images[imageIndex]);
				}
				if (mat.additionalValues.contains("normalTexture"))
				{
					const auto imageIndex = gltfModel.textures[mat.additionalValues["normalTexture"].TextureIndex()].source;
					material->SetNormalTexture(images[imageIndex]);
				}

				materials.push_back(material);
			}
			return materials;
		}

		void LoadGLTFNode(const tinygltf::Node* parent,
			const tinygltf::Node& node,
			const uint32_t nodeIndex,
			const tinygltf::Model& model,
			glm::mat4 globalTransform,
			std::vector<StaticVertex>& vertices,
			std::vector<uint16_t>& triangles,
			std::vector<StaticMesh::Submesh>& submeshes)
		{
			if (node.matrix.size() == 16)
			{
				globalTransform = globalTransform * glm::mat4(glm::make_mat4x4(node.matrix.data()));
			}

			if (!node.children.empty())
			{
				// Nodes children
				for (const auto& childNodeIdx : node.children)
				{
					LoadGLTFNode(&node, model.nodes[childNodeIdx], childNodeIdx, model, globalTransform, vertices, triangles, submeshes);
				}
			}

			if (node.mesh > -1)
			{
				// Node has mesh data
				const auto& mesh = model.meshes[node.mesh];
				for (auto i = 0; i < mesh.primitives.size(); ++i)
				{
					const auto& primitive = mesh.primitives[i];
					if (primitive.indices < 0)
						continue;

					auto& submesh = submeshes.emplace_back();
					submesh.IndexOffset = triangles.size();
					submesh.VertexOffset = vertices.size();
					submesh.MaterialIndex = primitive.material;

					{ // Vertices
						const float* bufferPos = nullptr;
						const float* bufferNormals = nullptr;
						const float* bufferTexCoords = nullptr;
						const float* bufferColors = nullptr;
						const float* bufferTangents = nullptr;

						const auto& posAccessor = model.accessors[primitive.attributes.find("POSITION")->second];
						const auto& posView = model.bufferViews[posAccessor.bufferView];
						bufferPos = reinterpret_cast<const float*>(
							&(model.buffers[posView.buffer].data[posAccessor.byteOffset + posView.byteOffset]));

						if (primitive.attributes.contains("NORMAL"))
						{
							const auto& normAccessor = model.accessors[primitive.attributes.find("NORMAL")->second];
							const auto& normView = model.bufferViews[normAccessor.bufferView];
							bufferNormals = reinterpret_cast<const float*>(
								&(model.buffers[normView.buffer].data[normAccessor.byteOffset + normView.byteOffset]));
						}
						if (primitive.attributes.contains("TEXCOORD_0"))
						{
							const auto& uvAccessor = model.accessors[primitive.attributes.find("TEXCOORD_0")->second];
							const auto& uvView = model.bufferViews[uvAccessor.bufferView];
							bufferTexCoords = reinterpret_cast<const float*>(
								&(model.buffers[uvView.buffer].data[uvAccessor.byteOffset + uvView.byteOffset]));
						}
						if (primitive.attributes.contains("TANGENT"))
						{
							const tinygltf::Accessor& tangentAccessor = model.accessors[primitive.attributes.find("TANGENT")->second];
							const tinygltf::BufferView& tangentView = model.bufferViews[tangentAccessor.bufferView];
							bufferTangents = reinterpret_cast<const float*>(
								&(model.buffers[tangentView.buffer].data[tangentAccessor.byteOffset + tangentView.byteOffset]));
						}

						submesh.VertexCount = posAccessor.count;
						for (auto v = 0; v < submesh.VertexCount; ++v)
						{
							auto& vertex = vertices.emplace_back();
							vertex.Position = glm::make_vec3(&bufferPos[v * 3]);
							vertex.Normal =
								glm::normalize(glm::vec3(bufferNormals ? glm::make_vec3(&bufferNormals[v * 3]) : glm::vec3(0.0f)));
							vertex.TexCoord = bufferTexCoords ? glm::make_vec2(&bufferTexCoords[v * 2]) : glm::vec2(0.0f);
							vertex.Tangent = bufferTangents ? glm::vec4(glm::make_vec4(&bufferTangents[v * 4])) : glm::vec4(0.0f);

							// Pre-Transform vertices
							vertex.Position = glm::vec3(globalTransform * glm::vec4(vertex.Position, 1.0f));
							vertex.Normal = glm::normalize(glm::mat3(globalTransform) * vertex.Normal);
						}
					}
					{ // Indices
						const auto& accessor = model.accessors[primitive.indices];
						const auto& bufferView = model.bufferViews[accessor.bufferView];
						const auto& buffer = model.buffers[bufferView.buffer];

						submesh.IndexCount = accessor.count;

						switch (accessor.componentType)
						{
							case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
							{
								std::vector<uint32_t> buf(accessor.count);
								memcpy(buf.data(),
									&buffer.data[accessor.byteOffset + bufferView.byteOffset],
									accessor.count * sizeof(uint32_t));
								for (size_t index = 0; index < accessor.count; index++)
								{
									triangles.push_back(buf[index] /* + submesh.VertexOffset*/);
								}
								break;
							}
							case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
							{
								std::vector<uint16_t> buf(accessor.count);
								memcpy(buf.data(),
									&buffer.data[accessor.byteOffset + bufferView.byteOffset],
									accessor.count * sizeof(uint16_t));
								for (size_t index = 0; index < accessor.count; index++)
								{
									triangles.push_back(buf[index] /* + submesh.VertexOffset*/);
								}
								break;
							}
							case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
							{
								std::vector<uint8_t> buf(accessor.count);
								memcpy(buf.data(),
									&buffer.data[accessor.byteOffset + bufferView.byteOffset],
									accessor.count * sizeof(uint8_t));
								for (size_t index = 0; index < accessor.count; index++)
								{
									triangles.push_back(buf[index] /* + submesh.VertexOffset*/);
								}
								break;
							}
							default:
								std::cerr << "Index component type " << accessor.componentType << " not supported!" << std::endl;
								return;
						}
					}
				}
			}
		}

	} // namespace Utils

	bool LoadGLTFModel(StaticMesh& mesh, Renderer& renderer, const std::string& filename)
	{
		tinygltf::Model gltfModel;
		tinygltf::TinyGLTF gltfCtx;

		std::string warning;
		std::string error;

		bool fileLoaded = gltfCtx.LoadASCIIFromFile(&gltfModel, &error, &warning, filename);

		if (!fileLoaded)
		{
			LOG_ERR("Failed to load GLTF model <{}>: {}", filename, error);
			return false;
		}

		const auto images = Utils::LoadGLTFImages(gltfModel, *renderer.GetContext());
		const auto materials = Utils::LoadGLTFMaterials(gltfModel, renderer, images);

		std::vector<StaticVertex> vertices;
		std::vector<uint16_t> triangles;
		std::vector<StaticMesh::Submesh> submeshes;

		const auto& scene = gltfModel.scenes[gltfModel.defaultScene > -1 ? gltfModel.defaultScene : 0];
		for (auto i = 0; i < scene.nodes.size(); ++i)
		{
			const auto node = gltfModel.nodes[scene.nodes[i]];
			Utils::LoadGLTFNode(nullptr, node, scene.nodes[i], gltfModel, glm::mat4(1.0f), vertices, triangles, submeshes);
		}

		mesh.SetVertices(vertices);
		mesh.SetTriangles(triangles);
		mesh.SetMaterials(materials);
		mesh.SetSubmeshes(submeshes);

		return true;
	}

} // namespace VkMana::Sample