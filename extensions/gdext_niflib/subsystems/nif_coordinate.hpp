#pragma once
// =============================================================================
// File:              nif_coordinate.hpp
// Author(s):         Chrischn89
// Godot Version:     4.5
// Description:
//   Shared coordinate conversion helpers: NIF (Z-up) <-> Godot (Y-up).
//   Used by all nif_*.cpp subsystem files.
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================

#include <niflib.h>
#include <nif_math.h>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/quaternion.hpp>
#include <godot_cpp/variant/basis.hpp>

// NIF -> Godot coordinate mapping: Godot(x, y, z) = NIF(x, z, -y)
godot::Vector3 nif_to_godot_vec3(const Niflib::Vector3& v);

// NIF quaternion -> Godot quaternion with axis permutation.
godot::Quaternion nif_quat_to_godot(const Niflib::Quaternion& q);

// NIF Matrix33 rotation -> Godot Basis via R_godot = P * R_nif * P^T.
// niflib stores row-vector matrices (v*M), so reads r[j][i] (transposing).
godot::Basis nif_matrix33_to_godot(const Niflib::Matrix33& r);

// Applies uniform scale to all three Basis column vectors.
void apply_uniform_scale(godot::Basis& basis, float scale);

// NIF XYZ Euler angles (radians) -> Godot quaternion.
// Rotation order: Qx * Qy * Qz (verified from OpenMW).
// Axis mapping: NIF-X->Godot-X, NIF-Y->Godot(-Z), NIF-Z->Godot-Y.
godot::Quaternion nif_euler_xyz_to_godot(float rx, float ry, float rz);
