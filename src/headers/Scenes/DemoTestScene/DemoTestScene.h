#pragma once

#include "headers/SceneManager.h"

class DemoTestScene : public Scene
{
	void SetupScene();
	void RenderScene(unsigned int deferredQuadFrameBuffer);

	std::string skyboxShaderProgramName;
	std::string blendingShaderProgramName;
	std::string tessShaderProgramName;
	std::string tessNormalVisualizationShaderProgramName;

	std::string containerDiffuseMap;
	std::string wallDiffuseMap;
	std::string grassTexture;
	std::string transparentWindowTexture;
	std::string skyboxTexture;
	std::string heightMap;
	std::string normalMap;

	std::unordered_map<std::string, std::vector<glm::vec3>> sceneAttributes;
	std::shared_ptr<PatchInfo> patchInfo;
};