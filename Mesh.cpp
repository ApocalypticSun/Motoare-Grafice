#include "Mesh.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

static constexpr int VERTEX_STRIDE_FLOATS = 11; // pos3 + color3 + uv2 + normal3

void MeshSystem::LinkVertexLayout(GpuMesh& m)
{
    m.vao.Bind();
    m.vbo.Bind();
    m.ebo.Bind();

    const GLsizei stride = VERTEX_STRIDE_FLOATS * sizeof(float);

    m.vao.LinkAttrib(m.vbo, 0, 3, GL_FLOAT, stride, (void*)0);                         // pos
    m.vao.LinkAttrib(m.vbo, 1, 3, GL_FLOAT, stride, (void*)(3 * sizeof(float)));       // color
    m.vao.LinkAttrib(m.vbo, 2, 2, GL_FLOAT, stride, (void*)(6 * sizeof(float)));       // uv
    m.vao.LinkAttrib(m.vbo, 3, 3, GL_FLOAT, stride, (void*)(8 * sizeof(float)));       // normal

    m.vao.Unbind();
    m.vbo.Unbind();
    m.ebo.Unbind();
}

void MeshSystem::AddMesh(const std::string& id, const CpuMeshData& data)
{
    if (meshById.count(id)) return;

    meshes.emplace_back(
        data.vertices.data(), (GLsizeiptr)(data.vertices.size() * sizeof(float)),
        data.indices.data(), (GLsizeiptr)(data.indices.size() * sizeof(GLuint)),
        (GLsizei)data.indices.size()
    );

    LinkVertexLayout(meshes.back());
    meshById.emplace(id, meshes.size() - 1);
}

void MeshSystem::AddPrimitiveMesh(const std::string& id, gfx::ShapeType type)
{
    auto m = gfx::Shapes::Get(type);
    AddMesh(id, CpuMeshData{ m.vertices, m.indices });
}

void MeshSystem::AddTexture(const std::string& id, const std::string& filePath, GLenum format)
{
    if (textureById.count(id)) return;

    textures.emplace_back(filePath.c_str(), GL_TEXTURE_2D, GL_TEXTURE0, format, GL_UNSIGNED_BYTE);
    textureById.emplace(id, textures.size() - 1);
}

void MeshSystem::RegisterShaderProgram(const std::string& id, Shader& shader)
{
    shaderById[id] = &shader;
}

void MeshSystem::SetLightParams(const glm::vec4& color, const glm::vec3& pos)
{
    lightColor = color;
    lightPos = pos;
}

size_t MeshSystem::AddObjectInstance(const SceneObject& obj)
{
    SceneObject o = obj;
    o.basePos = o.pos;
    objects.push_back(o);
    return objects.size() - 1;
}

SceneObject* MeshSystem::FindObject(const std::string& name)
{
    for (auto& o : objects)
        if (o.name == name) return &o;
    return nullptr;
}

MeshSystem::ShaderUniforms MeshSystem::GetShaderUniforms(Shader& s)
{
    auto it = uniformsByProgram.find(s.ID);
    if (it != uniformsByProgram.end()) return it->second;

    ShaderUniforms u;
    u.model = glGetUniformLocation(s.ID, "model");
    u.tex0 = glGetUniformLocation(s.ID, "tex0");
    u.lightColor = glGetUniformLocation(s.ID, "lightColor");
    u.lightPos = glGetUniformLocation(s.ID, "lightPos");
    u.camPos = glGetUniformLocation(s.ID, "camPos");

    uniformsByProgram.emplace(s.ID, u);
    return u;
}

glm::vec3 MeshSystem::GetWorldPos(const SceneObject& o, float t) const
{
    if (o.motion == Motion::BobY)
        return o.basePos + glm::vec3(0.0f, o.bobAmp * sinf(t * o.bobFreq), 0.0f);

    return o.pos;
}

glm::vec3 MeshSystem::GetWorldPosByName(const std::string& name, float t) const
{
    for (const auto& o : objects)
        if (o.name == name) return GetWorldPos(o, t);

    return glm::vec3(0.0f);
}

glm::mat4 MeshSystem::BuildModelMatrix(const SceneObject& o, float t) const
{
    glm::vec3 p = GetWorldPos(o, t);

    glm::mat4 model(1.0f);
    model = glm::translate(model, p);

    if (o.motion == Motion::RotateX)
        model = glm::rotate(model, glm::radians(t * o.rotSpeedDeg), glm::vec3(1, 0, 0));
    else if (o.motion == Motion::RotateY)
        model = glm::rotate(model, glm::radians(t * o.rotSpeedDeg), glm::vec3(0, 1, 0));
    else if (o.motion == Motion::RotateXY)
    {
        model = glm::rotate(model, glm::radians(t * o.rotSpeedDeg), glm::vec3(1, 0, 0));
        model = glm::rotate(model, glm::radians(t * o.rotSpeedDeg * 0.7f), glm::vec3(0, 1, 0));
    }

    model = glm::scale(model, o.scale);
    return model;
}

void MeshSystem::Render(Camera& camera, float t)
{
    Shader* current = nullptr;
    ShaderUniforms u;

    for (auto& o : objects)
    {
        auto mi = meshById.find(o.meshId);
        if (mi == meshById.end()) continue;

        auto si = shaderById.find(o.shaderId);
        if (si == shaderById.end() || !si->second) continue;

        Shader* s = si->second;

        if (s != current)
        {
            current = s;
            current->Activate();
            camera.Matrix(45.0f, 0.1f, 50.0f, *current, "camMatrix");
            u = GetShaderUniforms(*current);

            if (u.tex0 != -1)
                glUniform1i(u.tex0, 0);
        }

        if (u.lightColor != -1)
            glUniform4f(u.lightColor, lightColor.x, lightColor.y, lightColor.z, lightColor.w);

        if (u.lightPos != -1)
            glUniform3f(u.lightPos, lightPos.x, lightPos.y, lightPos.z);

        if (u.camPos != -1)
            glUniform3f(u.camPos, camera.Position.x, camera.Position.y, camera.Position.z);

        glm::mat4 model = BuildModelMatrix(o, t);
        if (u.model != -1)
            glUniformMatrix4fv(u.model, 1, GL_FALSE, glm::value_ptr(model));

        if (u.tex0 != -1)
        {
            auto ti = textureById.find(o.textureId);
            if (ti != textureById.end())
            {
                glActiveTexture(GL_TEXTURE0);   
                textures[ti->second].Bind();
            }
        }

        GpuMesh& m = meshes[mi->second];
        m.vao.Bind();
        glDrawElements(GL_TRIANGLES, m.indexCount, GL_UNSIGNED_INT, 0);
    }
}

void MeshSystem::Shutdown()
{
    for (auto& m : meshes) { m.vao.Delete(); m.vbo.Delete(); m.ebo.Delete(); }
    for (auto& t : textures) t.Delete();

    meshes.clear(); meshById.clear();
    textures.clear(); textureById.clear();
    objects.clear();
    shaderById.clear();
    uniformsByProgram.clear();
}
