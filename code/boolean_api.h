#pragma once

#include <array>
#include <bitset>
#include <cstdint>
#include <vector>

// DLL Export/Import macros
#ifdef _WIN32
    #ifdef MESH_BOOLEANS_EXPORTS
        #define MESH_BOOLEANS_API __declspec(dllexport)
    #else
        #define MESH_BOOLEANS_API __declspec(dllimport)
    #endif
#else
    #define MESH_BOOLEANS_API __attribute__((visibility("default")))
#endif

namespace mesh_booleans {

    enum BoolOp
    {
        UNITE = 0,
        INTERSECT = 1,
        SUBTRACT = 2,
        XOR = 3,
        NONREG_UNITE = 4
    };

    struct Mesh
    {
        std::vector<std::array<double, 3>> vertices;
        std::vector<std::array<std::uint32_t, 3>> triangles;
    };

    MESH_BOOLEANS_API void BooleanOperation(
        const std::vector<Mesh>& meshes, 
        const BoolOp& operation, 
        Mesh& output, 
        std::vector<std::bitset<32>>& labels);

    MESH_BOOLEANS_API void ResolveIntersections(const Mesh& mesh, Mesh& output);
}
