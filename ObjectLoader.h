#pragma once

#include <string>
#include "Mesh.h"   // CpuMeshData

class ObjectLoader
{
public:
    static CpuMeshData LoadOBJ(const std::string& path);
};
