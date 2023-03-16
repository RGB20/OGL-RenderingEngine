#pragma once

#include "SceneManager.h"

class StensilTestScene : public Scene
{
	void SetupScene();
	void RenderScene();

	std::string objectShaderProgramName;
    std::string stensilShaderProgramName;
    std::string lightShaderProgramName;

	std::string containerDiffuseMap;
	std::string containerSpecularMap;
	std::string wallDiffuseMap;
};
