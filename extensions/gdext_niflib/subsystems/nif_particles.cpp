// =============================================================================
// File:              nif_particles.cpp
// Author(s):         Chrischn89
// Godot Version:     4.5
// Description:
//   Particle system stub with NiPSys type documentation
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================
#include "gdext_niflib.hpp"

#include <obj/NiParticles.h>
#include <obj/NiParticleSystem.h>

#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;
using namespace Niflib;

// =============================================================================
// NiParticleSystem / NiParticles stub
// =============================================================================
// Civ4 uses particles extensively for fire, smoke, spell effects, explosions,
// weather, and unit death animations. The NIF particle system maps to Godot's
// GPUParticles3D with ParticleProcessMaterial.
//
// NiPSys* modifier/emitter/controller types and their Godot equivalents:
//   NiPSysGravityModifier      -> GPUParticlesAttractorBox3D
//   NiPSysGrowFadeModifier     -> scale curve in ParticleProcessMaterial
//   NiPSysColorModifier        -> color_ramp in ParticleProcessMaterial
//   NiPSysRotationModifier     -> angular_velocity in ParticleProcessMaterial
//   NiPSysBoundUpdateModifier  -> automatic in Godot (visibility_aabb)
//   NiPSysAgeDeathModifier     -> lifetime in ParticleProcessMaterial
//   NiPSysSphereEmitter        -> EMISSION_SHAPE_SPHERE
//   NiPSysCylinderEmitter      -> custom emission shape
//   NiPSysBoxEmitter           -> EMISSION_SHAPE_BOX
//   NiPSysMeshEmitter          -> EMISSION_SHAPE_POINTS with mesh vertices
//   NiPSysColliderManager      -> GPUParticlesCollision*3D nodes
//   NiPSysPlanarCollider       -> GPUParticlesCollisionHeightField3D
//   NiPSysSphericalCollider    -> GPUParticlesCollisionSphere3D
//   NiPSysSpawnModifier        -> sub_emitter in ParticleProcessMaterial
//   NiPSysDragModifier         -> damping in ParticleProcessMaterial
//   NiPSysBombModifier         -> GPUParticlesAttractor*3D with impulse
//   NiPSysTrailEmitter         -> trail mesh or Ribbon3D
//   NiPSysUpdateCtlr           -> automatic in Godot (process callback)
//   NiPSysEmitterCtlr          -> emitting property toggle
//   NiPSysModifierActiveCtlr   -> modifier enable/disable
//   NiPSysEmitterLifeSpanCtlr  -> AnimationPlayer lifetime track
//   NiPSysEmitterSpeedCtlr     -> AnimationPlayer initial_velocity track
//   NiPSysEmitterDeclinationCtlr    -> AnimationPlayer spread track
//   NiPSysEmitterDeclinationVarCtlr -> AnimationPlayer spread variance track
//   NiPSysEmitterPlanarAngleCtlr    -> AnimationPlayer flatness track
//   NiPSysEmitterPlanarAngleVarCtlr -> AnimationPlayer flatness variance track
//   NiPSysEmitterInitialRadiusCtlr  -> AnimationPlayer emission radius track
//   NiPSysInitialRotSpeedCtlr       -> AnimationPlayer angular_velocity track
//   NiPSysInitialRotSpeedVarCtlr    -> AnimationPlayer angular_velocity variance track
//   NiPSysInitialRotAngleCtlr       -> AnimationPlayer initial angle track
//   NiPSysInitialRotAngleVarCtlr    -> AnimationPlayer initial angle variance track
//   NiPSysGravityStrengthCtlr       -> AnimationPlayer attractor strength track
//   NiPSysFieldAttenuationCtlr      -> AnimationPlayer field attenuation track
//   NiPSysFieldMagnitudeCtlr        -> AnimationPlayer field magnitude track
//   NiPSysFieldMaxDistanceCtlr      -> AnimationPlayer field distance track
// =============================================================================
void GdextNiflib::process_ni_particle_system(NiParticlesRef particles,
    Node3D* parent, const String& base_path) {
    if (!particles || !parent) return;

    std::string name = nif_display_name(StaticCast<NiObject>(particles));
    std::string type = particles->GetType().GetTypeName();

    UtilityFunctions::print("[STUB] ", String::utf8(type.c_str()),
        " skipped: '", String::utf8(name.c_str()),
        "' — Godot equivalent: GPUParticles3D + ParticleProcessMaterial");
}
