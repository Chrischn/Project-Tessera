// =============================================================================
// File:              nif_coordinate.cpp
// Author(s):         Chrischn89
// Godot Version:     4.5
// Description:
//   Coordinate conversion function implementations
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================
#include "nif_coordinate.hpp"
#include <cmath>

godot::Vector3 nif_to_godot_vec3(const Niflib::Vector3& v) {
    return godot::Vector3(v.x, v.z, -v.y);
}

godot::Quaternion nif_quat_to_godot(const Niflib::Quaternion& q) {
    return godot::Quaternion(q.x, q.z, -q.y, q.w).normalized();
}

godot::Basis nif_matrix33_to_godot(const Niflib::Matrix33& r) {
    godot::Basis b;
    b.set_column(0, godot::Vector3( r[0][0],  r[0][2], -r[0][1]));
    b.set_column(1, godot::Vector3( r[2][0],  r[2][2], -r[2][1]));
    b.set_column(2, godot::Vector3(-r[1][0], -r[1][2],  r[1][1]));
    return b;
}

void apply_uniform_scale(godot::Basis& basis, float scale) {
    if (std::abs(scale - 1.0f) > 0.0001f) {
        basis.set_column(0, basis.get_column(0) * scale);
        basis.set_column(1, basis.get_column(1) * scale);
        basis.set_column(2, basis.get_column(2) * scale);
    }
}

godot::Quaternion nif_euler_xyz_to_godot(float rx, float ry, float rz) {
    godot::Quaternion qx(godot::Vector3(1, 0, 0), rx);
    godot::Quaternion qy(godot::Vector3(0, 0, -1), ry);
    godot::Quaternion qz(godot::Vector3(0, 1, 0), rz);
    return qx * qy * qz;
}
