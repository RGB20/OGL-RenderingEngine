#pragma once
#include <iostream>
#include <string.h>
#include <direct.h>
#include <vector>
#include <Memory.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

std::string GetCurrentDir();

std::string GetDirFromPath(std::string path);