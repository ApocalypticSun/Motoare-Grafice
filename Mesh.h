#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "VAO.h"
#include "VBO.h"
#include "EBO.h"
#include "Texture.h"
#include "shapes.h"
#include "shaderClass.h"
#include "Camera.h"

enum class Motion { None, BobY, RotateX, RotateY, RotateXY };

struct CpuMeshData
{
    std::vector<float> vertices;
    std::vector<GLuint> indices;
};

struct GpuMesh
{
    VAO vao;
    VBO vbo;
    EBO ebo;
    GLsizei indexCount;

    GpuMesh(const void* vtx, GLsizeiptr vtxSize,
        const void* idx, GLsizeiptr idxSize,
        GLsizei count)
        : vao(), vbo(vtx, vtxSize), ebo(idx, idxSize), indexCount(count)
    {
    }
};

struct SceneObject
{
    std::string name;
    std::string meshId;
    std::string textureId;
    std::string shaderId;

    glm::vec3 pos{ 0.0f };
    glm::vec3 scale{ 1.0f };

    Motion motion = Motion::None;
    float rotSpeedDeg = 0.0f;
    float bobAmp = 0.0f;
    float bobFreq = 0.0f;
    glm::vec3 basePos{ 0.0f };
};

class MeshSystem
{
public:
    void AddMesh(const std::string& id, const CpuMeshData& data);
    void AddPrimitiveMesh(const std::string& id, gfx::ShapeType type);

    void AddTexture(const std::string& id, const std::string& filePath, GLenum format = GL_RGB);

    void RegisterShaderProgram(const std::string& id, Shader& shader);
    void SetLightParams(const glm::vec4& color, const glm::vec3& pos);

    size_t AddObjectInstance(const SceneObject& obj);
    SceneObject* FindObject(const std::string& name);

    glm::vec3 GetWorldPos(const SceneObject& o, float t) const;
    glm::vec3 GetWorldPosByName(const std::string& name, float t) const;

    void Render(Camera& camera, float timeSec);

    void Shutdown();

private:
    void LinkVertexLayout(GpuMesh& m);
    glm::mat4 BuildModelMatrix(const SceneObject& o, float t) const;

    struct ShaderUniforms
    {
        GLint model = -1;
        GLint tex0 = -1;
        GLint lightColor = -1;
        GLint lightPos = -1;
        GLint camPos = -1;
    };

    ShaderUniforms GetShaderUniforms(Shader& s);

private:
    std::vector<GpuMesh> meshes;
    std::unordered_map<std::string, size_t> meshById;

    std::vector<Texture> textures;
    std::unordered_map<std::string, size_t> textureById;

    std::vector<SceneObject> objects;

    std::unordered_map<std::string, Shader*> shaderById;
    std::unordered_map<GLuint, ShaderUniforms> uniformsByProgram;

    glm::vec4 lightColor{ 1,1,1,1 };
    glm::vec3 lightPos{ 0.5f,0.5f,0.5f };
};
