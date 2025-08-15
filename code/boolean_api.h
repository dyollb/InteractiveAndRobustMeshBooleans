#pragma once

#include <bitset>
#include <vector>

#include <Eigen/Dense>

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


    MESH_BOOLEANS_API void BooleanOperation(
        const std::vector<Eigen::MatrixXd>& input_points,
        const std::vector<Eigen::MatrixXi>& input_tris,
        const BoolOp& operation,
        Eigen::MatrixXd& output_coords,
        Eigen::MatrixXi& output_tris,
        std::vector<std::bitset<32>>& output_labels);

    MESH_BOOLEANS_API void ResolveIntersections(
        const Eigen::MatrixXd& input_points,
        const Eigen::MatrixXi& input_tris,
        Eigen::MatrixXd& output_coords,
        Eigen::MatrixXi& output_tris);
}
