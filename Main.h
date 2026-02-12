#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "shaderClass.h"
#include "Camera.h"
#include "Mesh.h"   

GLFWwindow* InitWindow(unsigned int w, unsigned int h, const char* title);

void RegisterDefaultMeshes(MeshSystem& mesh);
void RegisterDefaultTextures(MeshSystem& mesh, Shader& shader);
void SpawnDefaultObjects(MeshSystem& mesh);