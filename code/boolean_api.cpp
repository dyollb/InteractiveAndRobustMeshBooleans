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

void fromEigen(
    const Eigen::MatrixXd& input_coords,
    const Eigen::MatrixXi& input_tris,
    std::vector<double>& out_coords,
    std::vector<uint>& out_tris)
{
    const uint offset = static_cast<uint>(out_coords.size() / 3);

    for (int i = 0; i < input_coords.rows(); ++i) {
        out_coords.push_back(input_coords(i, 0));
        out_coords.push_back(input_coords(i, 1));
        out_coords.push_back(input_coords(i, 2));
    }

    for (int i = 0; i < input_tris.rows(); ++i) {
        out_tris.push_back(input_tris(i, 0) + offset);
        out_tris.push_back(input_tris(i, 1) + offset);
        out_tris.push_back(input_tris(i, 2) + offset);
    }
}

void toEigen(
    const std::vector<double>& in_coords,
    const std::vector<uint>& in_tris,
    Eigen::MatrixXd& out_coords,
    Eigen::MatrixXi& out_tris)
{
    const size_t num_points = in_coords.size() / 3;
    const size_t num_tris = in_tris.size() / 3;

    out_coords.resize(num_points, 3);
    for (size_t i = 0, k=0; i < num_points; i++, k+=3) {
        out_coords(i, 0) = in_coords[k + 0];
        out_coords(i, 1) = in_coords[k + 1];
        out_coords(i, 2) = in_coords[k + 2];
    }

    out_tris.resize(num_tris, 3);
    for (size_t i = 0, k=0; i < num_tris; i++, k+=3) {
        out_tris(i, 0) = in_tris[k + 0];
        out_tris(i, 1) = in_tris[k + 1];
        out_tris(i, 2) = in_tris[k + 2];
    }
}

void BooleanOperation(
    const std::vector<Eigen::MatrixXd>& input_coords,
    const std::vector<Eigen::MatrixXi>& input_tris,
    const BoolOp& operation,
    Eigen::MatrixXd& output_coords,
    Eigen::MatrixXi& output_tris,
    std::vector<std::bitset<32>>& bool_labels)
{
    const uint num_input_points = std::accumulate(input_coords.begin(), input_coords.end(), 0u,
                    [](uint sum, const Eigen::MatrixXd& m) { return sum + m.rows(); });
    const uint num_input_tris = std::accumulate(input_tris.begin(), input_tris.end(), 0u,
                    [](uint sum, const Eigen::MatrixXi& m) { return sum + m.rows(); });

    std::vector<double> in_coords;
    in_coords.reserve(num_input_points * 3);

    std::vector<uint> in_tris, in_labels;
    in_tris.reserve(num_input_tris * 3);
    in_labels.reserve(num_input_tris);

    for(uint f_id = 0; f_id < input_coords.size(); f_id++) {

        const Eigen::MatrixXd& mesh_coords = input_coords[f_id];
        const Eigen::MatrixXi& mesh_tris = input_tris[f_id];
        fromEigen(mesh_coords, mesh_tris, in_coords, in_tris);

        for (uint i = 0, iN = mesh_coords.rows(); i < iN; i++) {
            in_labels.push_back(f_id);
        }
    }

    std::vector<double> bool_coords;
    std::vector<uint> bool_tris;
    const ::BoolOp op = static_cast<::BoolOp>(operation);

    booleanPipeline(in_coords, in_tris, in_labels, op, bool_coords, bool_tris, bool_labels);

    toEigen(bool_coords, bool_tris, output_coords, output_tris);
}

void ResolveIntersections(
    const Eigen::MatrixXd& input_points,
    const Eigen::MatrixXi& input_tris,
    Eigen::MatrixXd& output_coords,
    Eigen::MatrixXi& output_tris)
{
    std::vector<double> in_coords;
    in_coords.reserve(input_points.rows() * 3);
    
    std::vector<uint> in_tris;
    in_tris.reserve(input_tris.rows() * 3);

    fromEigen(input_points, input_tris, in_coords, in_tris);

    std::vector<double> out_coords;
    std::vector<uint> out_tris;
    point_arena arean;
    solveIntersections(in_coords, in_tris, arean, out_coords, out_tris);

    toEigen(out_coords, out_tris, output_coords, output_tris);
}

}
