#pragma once

#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/gtx/hash.hpp>

namespace VkMana::SamplesApp
{
	struct Vertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TexCoord;

		bool operator==(const Vertex& other) const
		{
			return Position == other.Position && Normal == other.Normal && TexCoord == other.TexCoord;
		}
	};

} // namespace VkMana::SamplesApp

namespace std
{
	template <>
	struct hash<VkMana::SamplesApp::Vertex>
	{
		size_t operator()(VkMana::SamplesApp::Vertex const& vertex) const noexcept
		{
			return ((hash<glm::vec3>()(vertex.Position) ^ (hash<glm::vec3>()(vertex.Normal) << 1)) >> 1)
				^ (hash<glm::vec2>()(vertex.TexCoord) << 1);
		}
	};
} // namespace std
