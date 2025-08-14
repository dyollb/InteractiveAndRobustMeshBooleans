#include "boolean_api.h"

#ifndef NOMINMAX
    #define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif

#include "booleans.h"
#include "solve_intersections.h"

namespace mesh_booleans {

void BooleanOperation(
    const std::vector<Mesh>& meshes, 
    const BoolOp& operation, 
    Mesh& output, 
    std::vector<std::bitset<32>>& bool_labels)
{
    std::vector<double> in_coords;
    std::vector<uint> in_tris;
    std::vector<uint> in_labels;

    for(uint f_id = 0; f_id < meshes.size(); f_id++)
    {
        const Mesh& mesh = meshes[f_id];
        const uint off = static_cast<uint>(in_coords.size() / 3);

        in_coords.reserve(in_coords.size() + mesh.vertices.size() * 3);
        in_tris.reserve(in_tris.size() + mesh.triangles.size() * 3);
        in_labels.reserve(in_labels.size() + 1);

        for(const auto& vertex : mesh.vertices)
        {
            in_coords.insert(in_coords.end(), vertex.begin(), vertex.end());
        }
        for(const auto& triangle : mesh.triangles)
        {
            in_tris.push_back(triangle[0] + off);
            in_tris.push_back(triangle[1] + off);
            in_tris.push_back(triangle[2] + off);
        }
        in_labels.push_back(f_id);
    }

    std::vector<double> bool_coords;
    std::vector<uint> bool_tris;

    const std::array<::BoolOp, 5> op_map = {
        ::BoolOp::UNION,
        ::BoolOp::INTERSECTION,
        ::BoolOp::SUBTRACTION,
        ::BoolOp::XOR,
        ::BoolOp::NONREG
    };

    booleanPipeline(in_coords, in_tris, in_labels, op_map.at(operation), bool_coords, bool_tris, bool_labels);

    output.vertices.reserve(bool_coords.size() / 3);
    output.triangles.reserve(bool_tris.size() / 3);

    for (size_t i = 0; i < bool_coords.size(); i += 3) {
        output.vertices.push_back({ bool_coords[i], bool_coords[i + 1], bool_coords[i + 2] });
    }
    for (size_t i = 0; i < bool_tris.size(); i += 3) {
        output.triangles.push_back({ bool_tris[i], bool_tris[i + 1], bool_tris[i + 2] });
    }
}

void ResolveIntersections(const Mesh& mesh, Mesh& output)
{
    std::vector<double> in_coords, out_coords;
    for (const auto& vertex : mesh.vertices) {
        in_coords.insert(in_coords.end(), vertex.begin(), vertex.end());
    }
    std::vector<uint> in_tris, out_tris;
    for (const auto& triangle : mesh.triangles) {
        in_tris.insert(in_tris.end(), triangle.begin(), triangle.end());
    }

    point_arena arean;
    solveIntersections(in_coords, in_tris, arean, out_coords, out_tris);

    output.vertices.reserve(out_coords.size() / 3);
    output.triangles.reserve(out_tris.size() / 3);

    for (size_t i = 0; i < out_coords.size(); i += 3) {
        output.vertices.push_back({ out_coords[i], out_coords[i + 1], out_coords[i + 2] });
    }
    for (size_t i = 0; i < out_tris.size(); i += 3) {
        output.triangles.push_back({ out_tris[i], out_tris[i + 1], out_tris[i + 2] });
    }
}

}
