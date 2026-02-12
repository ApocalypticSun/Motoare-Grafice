#include "ObjectLoader.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>

#include <glm/glm.hpp>

// Layout: pos3 + color3 + uv2 + normal3
static constexpr int kVertexStrideFloats = 11;

// Key for unique vertex (v/vt/vn triple)
struct ObjVertexKey
{
    int v = 0;
    int vt = 0;
    int vn = 0;

    bool operator==(const ObjVertexKey& o) const
    {
        return v == o.v && vt == o.vt && vn == o.vn;
    }
};

struct ObjVertexKeyHash
{
    size_t operator()(const ObjVertexKey& k) const
    {
        size_t h1 = std::hash<int>{}(k.v);
        size_t h2 = std::hash<int>{}(k.vt);
        size_t h3 = std::hash<int>{}(k.vn);
        return (h1 * 73856093u) ^ (h2 * 19349663u) ^ (h3 * 83492791u);
    }
};

// Parses a face token: "v", "v/vt", "v//vn", "v/vt/vn"
static bool ParseFaceToken(const std::string& tok, int& vi, int& vti, int& vni)
{
    vi = vti = vni = 0;

    const size_t p1 = tok.find('/');
    if (p1 == std::string::npos)
    {
        try { vi = std::stoi(tok); }
        catch (...) { return false; }
        return true;
    }

    // v
    const std::string a = tok.substr(0, p1);
    if (!a.empty())
    {
        try { vi = std::stoi(a); }
        catch (...) { return false; }
    }

    const size_t p2 = tok.find('/', p1 + 1);
    if (p2 == std::string::npos)
    {
        // v/vt
        const std::string b = tok.substr(p1 + 1);
        if (!b.empty())
        {
            try { vti = std::stoi(b); }
            catch (...) { return false; }
        }
        return true;
    }

    // v/vt/vn or v//vn
    const std::string b = tok.substr(p1 + 1, p2 - (p1 + 1));
    const std::string c = tok.substr(p2 + 1);

    if (!b.empty())
    {
        try { vti = std::stoi(b); }
        catch (...) { return false; }
    }

    if (!c.empty())
    {
        try { vni = std::stoi(c); }
        catch (...) { return false; }
    }

    return true;
}

static inline void AppendVertex11(std::vector<float>& dst,
    const glm::vec3& p,
    const glm::vec2& uv,
    const glm::vec3& n)
{
    // pos
    dst.push_back(p.x); dst.push_back(p.y); dst.push_back(p.z);
    // color (default white)
    dst.push_back(1.0f); dst.push_back(1.0f); dst.push_back(1.0f);
    // uv
    dst.push_back(uv.x); dst.push_back(uv.y);
    // normal
    dst.push_back(n.x); dst.push_back(n.y); dst.push_back(n.z);
}

CpuMeshData ObjectLoader::LoadOBJ(const std::string& path)
{
    CpuMeshData out;

    std::ifstream file(path);
    if (!file.is_open())
    {
        std::cout << "OBJ open failed: " << path << "\n";
        return out;
    }

    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> texcoords;
    std::vector<glm::vec3> normals;

    std::unordered_map<ObjVertexKey, unsigned int, ObjVertexKeyHash> vertexRemap;

    auto GetOrCreateVertexIndex = [&](int vi, int vti, int vni) -> unsigned int
        {
            // Support negative indices (OBJ spec)
            if (vi < 0)  vi = (int)positions.size() + 1 + vi;
            if (vti < 0) vti = (int)texcoords.size() + 1 + vti;
            if (vni < 0) vni = (int)normals.size() + 1 + vni;

            ObjVertexKey key{ vi, vti, vni };
            auto it = vertexRemap.find(key);
            if (it != vertexRemap.end())
                return it->second;

            glm::vec3 p(0.0f);
            glm::vec2 uv(0.0f);
            glm::vec3 n(0.0f, 0.0f, 1.0f);

            if (vi > 0 && vi <= (int)positions.size()) p = positions[vi - 1];
            if (vti > 0 && vti <= (int)texcoords.size()) uv = texcoords[vti - 1];
            if (vni > 0 && vni <= (int)normals.size())   n = normals[vni - 1];

            const unsigned int newIndex = (unsigned int)(out.vertices.size() / kVertexStrideFloats);
            AppendVertex11(out.vertices, p, uv, n);

            vertexRemap.emplace(key, newIndex);
            return newIndex;
        };

    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty() || line[0] == '#') continue;

        std::stringstream ss(line);
        std::string tag;
        ss >> tag;

        if (tag == "v")
        {
            glm::vec3 p;
            ss >> p.x >> p.y >> p.z;
            positions.push_back(p);
        }
        else if (tag == "vt")
        {
            // Some OBJ files may have 3 values (u v w). We read u v and ignore w if present.
            glm::vec2 uv;
            ss >> uv.x >> uv.y;
            texcoords.push_back(uv);
        }
        else if (tag == "vn")
        {
            glm::vec3 n;
            ss >> n.x >> n.y >> n.z;
            normals.push_back(n);
        }
        else if (tag == "f")
        {
            std::vector<unsigned int> face;
            std::string tok;

            while (ss >> tok)
            {
                int vi, vti, vni;
                if (!ParseFaceToken(tok, vi, vti, vni))
                    continue;

                face.push_back(GetOrCreateVertexIndex(vi, vti, vni));
            }

            // Triangulate fan: (0, i, i+1)
            for (size_t i = 1; i + 1 < face.size(); i++)
            {
                out.indices.push_back(face[0]);
                out.indices.push_back(face[i]);
                out.indices.push_back(face[i + 1]);
            }
        }
    }

    std::cout << "OBJ loaded: " << path << "\n";
    std::cout << "floats: " << out.vertices.size() << " (stride " << kVertexStrideFloats << ")\n";
    std::cout << "indices: " << out.indices.size() << "\n";
    return out;
}
