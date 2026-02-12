#pragma once
#include <vector>
#include <glad/glad.h>

namespace gfx
{
    enum class ShapeType
    {
        Triangle,
        Cube,
        Circle,
        Square,
        Rectangle
    };

    struct MeshData
    {
        std::vector<GLfloat> vertices;
        std::vector<GLuint> indices;

        size_t TriangleCount() const { return indices.size() / 3; }
        GLsizei IndexCount() const { return static_cast<GLsizei>(indices.size()); }
    };

    class Shapes
    {
    public:
        static MeshData Triangle();
        static MeshData Cube();
        static MeshData Circle(float radius, int segments, bool inXZ);
        static MeshData Square(float size);
        static MeshData Rectangle(float width, float height);
        static MeshData Get(ShapeType type);
    };
}
