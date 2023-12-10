#pragma once

#include "Renderer.hpp"
#include "StaticMesh.hpp"

namespace VkMana::SamplesApp
{
	bool LoadGLTFModel(StaticMesh& mesh, Renderer& renderer, const std::string& filename);

} // namespace VkMana::Sample
