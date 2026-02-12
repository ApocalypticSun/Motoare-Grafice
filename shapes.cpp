#include "shapes.h"
#include <cmath>

namespace gfx
{
    // x y z  r g b  u v  nx ny nz
    static inline void PushV(std::vector<GLfloat>& v,
        GLfloat x, GLfloat y, GLfloat z,
        GLfloat r, GLfloat g, GLfloat b,
        GLfloat u, GLfloat vv,
        GLfloat nx, GLfloat ny, GLfloat nz)
    {
        v.insert(v.end(), { x,y,z, r,g,b, u,vv, nx,ny,nz });
    }

    MeshData Shapes::Triangle()
    {
        MeshData m;
        PushV(m.vertices, -0.5f, -0.5f, 0.0f, 1, 0, 0, 0, 0, 0, 0, 1);
        PushV(m.vertices, 0.5f, -0.5f, 0.0f, 0, 1, 0, 1, 0, 0, 0, 1);
        PushV(m.vertices, 0.0f, 0.5f, 0.0f, 0, 0, 1, 0.5f, 1, 0, 0, 1);
        m.indices = { 0,1,2 };
        return m;
    }

    MeshData Shapes::Cube()
    {
        MeshData m;
        m.vertices.reserve(24 * 11);

        // FRONT +Z
        PushV(m.vertices, -0.5f, -0.5f, 0.5f, 1, 0, 0, 0, 0, 0, 0, 1);
        PushV(m.vertices, 0.5f, -0.5f, 0.5f, 0, 1, 0, 1, 0, 0, 0, 1);
        PushV(m.vertices, 0.5f, 0.5f, 0.5f, 0, 0, 1, 1, 1, 0, 0, 1);
        PushV(m.vertices, -0.5f, 0.5f, 0.5f, 1, 1, 0, 0, 1, 0, 0, 1);

        // BACK -Z
        PushV(m.vertices, 0.5f, -0.5f, -0.5f, 1, 0, 1, 0, 0, 0, 0, -1);
        PushV(m.vertices, -0.5f, -0.5f, -0.5f, 0, 1, 1, 1, 0, 0, 0, -1);
        PushV(m.vertices, -0.5f, 0.5f, -0.5f, 1, 1, 1, 1, 1, 0, 0, -1);
        PushV(m.vertices, 0.5f, 0.5f, -0.5f, 0.2f, 0.2f, 0.2f, 0, 1, 0, 0, -1);

        // LEFT -X
        PushV(m.vertices, -0.5f, -0.5f, -0.5f, 1, 0, 0, 0, 0, -1, 0, 0);
        PushV(m.vertices, -0.5f, -0.5f, 0.5f, 0, 1, 0, 1, 0, -1, 0, 0);
        PushV(m.vertices, -0.5f, 0.5f, 0.5f, 0, 0, 1, 1, 1, -1, 0, 0);
        PushV(m.vertices, -0.5f, 0.5f, -0.5f, 1, 1, 0, 0, 1, -1, 0, 0);

        // RIGHT +X
        PushV(m.vertices, 0.5f, -0.5f, 0.5f, 1, 0, 1, 0, 0, 1, 0, 0);
        PushV(m.vertices, 0.5f, -0.5f, -0.5f, 0, 1, 1, 1, 0, 1, 0, 0);
        PushV(m.vertices, 0.5f, 0.5f, -0.5f, 1, 1, 1, 1, 1, 1, 0, 0);
        PushV(m.vertices, 0.5f, 0.5f, 0.5f, 0.2f, 0.2f, 0.2f, 0, 1, 1, 0, 0);

        // TOP +Y
        PushV(m.vertices, -0.5f, 0.5f, 0.5f, 1, 0, 0, 0, 0, 0, 1, 0);
        PushV(m.vertices, 0.5f, 0.5f, 0.5f, 0, 1, 0, 1, 0, 0, 1, 0);
        PushV(m.vertices, 0.5f, 0.5f, -0.5f, 0, 0, 1, 1, 1, 0, 1, 0);
        PushV(m.vertices, -0.5f, 0.5f, -0.5f, 1, 1, 0, 0, 1, 0, 1, 0);

        // BOTTOM -Y
        PushV(m.vertices, -0.5f, -0.5f, -0.5f, 1, 0, 1, 0, 0, 0, -1, 0);
        PushV(m.vertices, 0.5f, -0.5f, -0.5f, 0, 1, 1, 1, 0, 0, -1, 0);
        PushV(m.vertices, 0.5f, -0.5f, 0.5f, 1, 1, 1, 1, 1, 0, -1, 0);
        PushV(m.vertices, -0.5f, -0.5f, 0.5f, 0.2f, 0.2f, 0.2f, 0, 1, 0, -1, 0);

        m.indices = {
             0, 1, 2,  2, 3, 0,
             4, 5, 6,  6, 7, 4,
             8, 9,10, 10,11, 8,
            12,13,14, 14,15,12,
            16,17,18, 18,19,16,
            20,21,22, 22,23,20
        };
        return m;
    }

    MeshData Shapes::Circle(float radius, int segments, bool inXZ)
    {
        MeshData m;
        if (segments < 3) segments = 3;

        // normal pentru disc
        float nx = 0.0f, ny = inXZ ? 1.0f : 0.0f, nz = inXZ ? 0.0f : 1.0f;

        // centru
        PushV(m.vertices, 0, 0, 0, 1, 1, 1, 0.5f, 0.5f, nx, ny, nz);

        const float twoPi = 2.0f * 3.14159265358979323846f;

        for (int i = 0; i < segments; i++)
        {
            float a = twoPi * (float(i) / float(segments));
            float cx = radius * std::cos(a);
            float cz = radius * std::sin(a);

            float x, y, z;
            if (inXZ) { x = cx; y = 0.0f; z = cz; }
            else { x = cx; y = cz;   z = 0.0f; }

            float r = 0.6f + 0.4f * std::cos(a);
            float g = 0.6f + 0.4f * std::sin(a);
            float b = 0.8f;

            float u = 0.5f + (cx / (2.0f * radius));
            float v = 0.5f + (cz / (2.0f * radius));

            PushV(m.vertices, x, y, z, r, g, b, u, v, nx, ny, nz);
        }

        for (int i = 0; i < segments; i++)
        {
            GLuint center = 0;
            GLuint curr = 1 + i;
            GLuint next = 1 + ((i + 1) % segments);
            m.indices.insert(m.indices.end(), { center, curr, next });
        }

        return m;
    }

    MeshData Shapes::Square(float size)
    {
        MeshData m;
        float h = size * 0.5f;

        PushV(m.vertices, -h, -h, 0, 1, 0, 0, 0, 0, 0, 0, 1);
        PushV(m.vertices, h, -h, 0, 0, 1, 0, 1, 0, 0, 0, 1);
        PushV(m.vertices, h, h, 0, 0, 0, 1, 1, 1, 0, 0, 1);
        PushV(m.vertices, -h, h, 0, 1, 1, 0, 0, 1, 0, 0, 1);

        m.indices = { 0,1,2, 2,3,0 };
        return m;
    }

    MeshData Shapes::Rectangle(float width, float height)
    {
        MeshData m;
        float hw = width * 0.5f;
        float hh = height * 0.5f;

        PushV(m.vertices, -hw, -hh, 0, 0.7f, 0.2f, 0.2f, 0, 0, 0, 0, 1);
        PushV(m.vertices, hw, -hh, 0, 0.2f, 0.7f, 0.2f, 1, 0, 0, 0, 1);
        PushV(m.vertices, hw, hh, 0, 0.2f, 0.2f, 0.7f, 1, 1, 0, 0, 1);
        PushV(m.vertices, -hw, hh, 0, 0.7f, 0.7f, 0.2f, 0, 1, 0, 0, 1);

        m.indices = { 0,1,2, 2,3,0 };
        return m;
    }

    MeshData Shapes::Get(ShapeType type)
    {
        switch (type)
        {
        case ShapeType::Triangle:  return Triangle();
        case ShapeType::Cube:      return Cube();
        case ShapeType::Circle:    return Circle(0.8f, 60, false);
        case ShapeType::Square:    return Square(1);
        case ShapeType::Rectangle: return Rectangle(1.2f, 0.6f);
        default:                   return Triangle();
        }
    }
}