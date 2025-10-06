//GDExtension Header
#include "gdext_niflib.hpp"

//niflib Headers
#include <NIF_IO.h>
#include <gen/Header.h>
#include <nif_basic_types.h>
#include <obj/NiNode.h>
#include <obj/NiObjectNET.h>
#include <obj/NiSourceTexture.h>
#include <obj/NiTriShape.h>
#include <obj/NiTriShapeData.h>
#include <obj/NiTriStrips.h>
#include <obj/NiTriStripsData.h>
#include <obj/NiTexturingProperty.h>

//godot-cpp Headers
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/surface_tool.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/texture2d.hpp> 

//Standard Library
#include <string_view>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <cctype>
#include <exception>


using namespace godot;
using namespace Niflib;

//Binds functions for use from inside Godot Editor. Hint: not every function needs to be bound 
void GdextNiflib::_bind_methods() {
    ClassDB::bind_method(D_METHOD("ping"), &GdextNiflib::ping);
    ClassDB::bind_method(D_METHOD("get_nif_version", "file_path"), &GdextNiflib::get_nif_version_as_string);
    ClassDB::bind_method(D_METHOD("get_nif_header_info", "file_path"), &GdextNiflib::get_nif_header_info);
    ClassDB::bind_method(D_METHOD("get_nif_header", "file_path"), &GdextNiflib::get_nif_header);
    ClassDB::bind_method(D_METHOD("check_nif_version_code", "version_code"), &GdextNiflib::isValidNIFVersion);
    ClassDB::bind_method(D_METHOD("load_nif_scene", "file_path", "root"), &GdextNiflib::load_nif_scene);
}

// Check if GDExtension is working correctly
String GdextNiflib::ping() const {
    return "gdext_niflib OK";
}

// Receives a file path to a .nif file and reads version information
// Returns a String  with all the version number
// See also "get_nif_version_as_uint"
String GdextNiflib::get_nif_version_as_string(const String& file_path) const {
    try {
        unsigned int version = Niflib::GetNifVersion(file_path.utf8().get_data());
        return Niflib::FormatVersionString(version).c_str();
    } catch(const std::exception& e) {
        return "Error: " + String(e.what());
    }
}

// Receives a file path to a .nif file and reads version information
// Returns a unsinged int with version number
// See also "get_nif_version_as_string"
unsigned int GdextNiflib::get_nif_version_as_uint(const String& file_path) const {
    try {
        unsigned int version = Niflib::GetNifVersion(file_path.utf8().get_data());
        return version;
    }
    catch (const std::exception& e) {
        UtilityFunctions::print("Error: ", e.what());
        return 0;
    }
}

// Receives a file path to a .nif file and reads the header information
// Returns a Godot Dictionary with all the information
// See also "get_nif_header"
Dictionary GdextNiflib::get_nif_header_info(const String& file_path) const {
    Dictionary result;
    try {
        Niflib::NifInfo info = Niflib::ReadHeaderInfo(file_path.utf8().get_data());
        result["version"] = Niflib::FormatVersionString(info.version).c_str();
        result["user_version"] = (int)info.userVersion;
        result["user_version2"] = (int)info.userVersion2;
        result["endian"] = (int)info.endian;
        result["creator"] = info.creator.c_str();
        result["export_info1"] = info.exportInfo1.c_str();
        result["export_info2"] = info.exportInfo2.c_str();
        result["success"] = true;
    } catch (const std::exception& e) {
        result["success"] = false;
        result["error"] = e.what();
    }
    return result;
}


// Receives a file path to a .nif file and reads the header information
// Returns a Godot Dictionary with all the information
// See also "get_nif_header_info"
Dictionary GdextNiflib::get_nif_header(const String& file_path) const {
    Dictionary result;
    try {
        Niflib::Header header = Niflib::ReadHeader(file_path.utf8().get_data());
        
        result["version"] = Niflib::FormatVersionString(header.version).c_str();
        result["user_version"] = (int)header.userVersion;
        result["user_version2"] = (int)header.userVersion2;
        result["endian_type"] = (int)header.endianType;
        result["num_blocks"] = (int)header.numBlocks;
        result["num_block_types"] = (int)header.blockTypes.size();
        
        // Add block types array
        Array block_types;
        for (const auto& type : header.blockTypes) {
            block_types.push_back(type.c_str());
        }
        result["block_types"] = block_types;
        
        // Creator and export info
        result["creator"] = header.exportInfo.creator.str.c_str();
        result["export_info1"] = header.exportInfo.exportInfo1.str.c_str();
        result["export_info2"] = header.exportInfo.exportInfo2.str.c_str();
        
        // Copyright lines (first few)
        Array copyright;
        for (size_t i = 0; i < 3; ++i) {
            copyright.push_back(header.copyright[i].line.c_str());
        }
        result["copyright"] = copyright;
        
        result["success"] = true;
    } catch (const std::exception& e) {
        result["success"] = false;
        result["error"] = e.what();
    }
    return result;
}


// Receives a .nif file version number and checks whether it is a valid version or not
// Returns true or false
bool GdextNiflib::isValidNIFVersion(unsigned int version_code) const {
    bool result;
    unsigned int version = version_code;
    
    if (version == Niflib::VER_INVALID) {
        result = false;
        godot::print_line("VER_INVALID");
        UtilityFunctions::print("Provided version: " + String(Niflib::FormatVersionString(version).c_str()));
        godot::print_line("Not a NIF file");
        return result;
    }

    if (version == Niflib::VER_UNSUPPORTED) {
        result = false;
        godot::print_line("VER_UNSUPPORTED");
        UtilityFunctions::print("Provided version: " + String(Niflib::FormatVersionString(version).c_str()));
        godot::print_line("Unsupported NIF version");
        return result;
    }
    
    if (Niflib::IsSupportedVersion(version)) {
        result = true;
        godot::print_line("SUPPORTED_VERSION");
        UtilityFunctions::print("Supported version: " + String(Niflib::FormatVersionString(version).c_str()));
        return result;
    }
    
    result = true;
    godot::print_line("UNKNOWN_VERSION");
    UtilityFunctions::print("Unknown version: " + String(Niflib::FormatVersionString(version).c_str()));
    return result;
}

// Receives a file path to a .nif file that needs importing into Godot.
// Receives a Godot Node* pointer to an already existing Godot Node to enable the retrieval of data from Godot or to attach new Nodes to it.
void GdextNiflib::load_nif_scene(const String& file_path, Node3D* godotnode) {        
    try {
        if (isValidNIFVersion(get_nif_version_as_uint(file_path))) {
            Node3D* custom_nif_node3d_root = memnew(Node3D);

            NiObjectRef ref_root = ReadNifTree(file_path.utf8().get_data());

            if (ref_root != NULL) {
                rebuild_nif_tree_in_godot(ref_root, godotnode);
            }
        }
        else {
            throw std::invalid_argument("Err loading NIF: file invalid!");
        }

    } catch (const std::exception& e) {
        UtilityFunctions::print("Error loading NIF: ", e.what());
    }
}

// Receives a NiObjectRef (basically a pointer) for an NiObject that needs conversion to a Godot Node.
// Receives a Godot Node* pointer to an already existing Godot Node to enable the retrieval of data from Godot or to attach new Nodes to it.
// Returns a Godot Node* pointer to a newly created Godot Node or returns a nullptr if no new Node is generated.
Node* GdextNiflib::convert_nif_node_to_godot(Niflib::NiObjectRef current_node, Node* parent) {
    try {
            Node3D* converted_NiObject = memnew(Node3D); //TODO: create an empty Godot Node object for every type of Node in Godot so that the different switch cases can grab whatever they need. Delete the rest one step before returning the converted Node.
            Node3D* converted_NiNode = memnew(Node3D);
            MeshInstance3D* converted_NiTriShape = memnew(MeshInstance3D);

            converted_NiObject->set_name(String::utf8((current_node->GetType().GetTypeName()).c_str()));

            NiType type = stringToNiType(current_node->GetType().GetTypeName());
            switch(type) {      //TODO: Find a better solution to determine and convert different NiObject types than enum + stringToNiType + switch
                case NiType::NiNode:
                    UtilityFunctions::print("You found a NiNode");
                    UtilityFunctions::print("--------------------------------------");
                    return converted_NiObject;


                case NiType::NiTriShape:
                    UtilityFunctions::print("You found a NiTriShape");
                    return converted_NiObject;


                case NiType::NiTriShapeData:
                    UtilityFunctions::print("You found a NiTriShapeData");
                    try {
                        int mesh_count = 0;

                            if (current_node != nullptr) {

                                if (Niflib::NiTriShapeDataRef data = DynamicCast<NiTriShapeData>(current_node)) {
                                        
                                    if (data != nullptr) {
                                        // Create SurfaceTool for Godot mesh
                                        Ref<SurfaceTool> st = memnew(SurfaceTool);
                                        st->begin(Mesh::PRIMITIVE_TRIANGLES);

                                        // Extract vertices
                                        Niflib::vector<Niflib::Vector3> vertices = data->GetVertices();
                                        UtilityFunctions::print("Mesh " + String::num_int64(mesh_count) + ": " + String::num_int64(vertices.size()) + " vertices");

                                        for (const auto& v : vertices) {
                                            // Basic coordinate conversion: NIF Y-up to Godot Z-up
                                            st->add_vertex(Vector3(v.x, v.z, -v.y));  // Swap Y/Z, negate Z for handedness
                                        }

                                        // Extract triangles (indices)
                                        Niflib::vector<Niflib::Triangle> triangles = data->GetTriangles();
                                        for (const auto& tri : triangles) {
                                            st->add_index(tri.v1);
                                            st->add_index(tri.v2);
                                            st->add_index(tri.v3);
                                        }

                                        // Generate normals for basic lighting
                                        st->generate_normals();

                                        // Commit to ArrayMesh
                                        Ref<ArrayMesh> godot_mesh = st->commit();

                                        if (godot_mesh.is_valid()) {
                                            // Create MeshInstance3D
                                            godot::MeshInstance3D* mesh_instance = memnew(MeshInstance3D);
                                            mesh_instance->set_name("Mesh_" + String::num_int64(mesh_count));
                                            mesh_instance->set_mesh(godot_mesh);

                                            // Simple material for visibility
                                            godot::StandardMaterial3D* material = memnew(StandardMaterial3D);
                                            material->set_albedo(Color(0.8, 0.2, 0.2));  // Red tint for debugging
                                            material->set_cull_mode(StandardMaterial3D::CULL_DISABLED);  // Show both sides
                                            mesh_instance->set_surface_override_material(0, material);

                                            // Position meshes side by side for visibility
                                            mesh_instance->set_position(Vector3(mesh_count * 2.0f, 0, 0));

                                            // Add meshes to new Godot Node
                                            converted_NiObject->add_child(mesh_instance);
                                            mesh_count++;
                                        }
                                    }
                                }
                            }

                        UtilityFunctions::print("Created " + String::num_int64(mesh_count) + " MeshInstance3D nodes");
                        UtilityFunctions::print("--------------------------------------");
                        return converted_NiObject;
                    }
                    catch (const std::exception& e) {
                        UtilityFunctions::print("Error converting NifTriShapeData: " + String(e.what()));
                        return nullptr;
                    }
                        
                default:
                    UtilityFunctions::print("You found a Default");
                    UtilityFunctions::print("--------------------------------------");
                    return converted_NiObject;
                }
            
    } catch (const std::exception& e) {
        UtilityFunctions::print("Error loading NIF: ", e.what());
        return nullptr;
    }
    catch (...) {
        UtilityFunctions::push_error("Undefined error loading NIF!");
        return nullptr;
    }
}

// Receives a NiObjectRef (basically a pointer) for an NiObject that needs conversion to a Godot Node.
// Iterates recursively through all of the NiObject's children and their children
// Receives a Godot Node* pointer to an already existing Godot Node to enable the retrieval of data from Godot or to attach new Nodes to it.
void GdextNiflib::rebuild_nif_tree_in_godot(Niflib::NiObjectRef current_node, Node* parent) {
    try {
            UtilityFunctions::print("Name of the current node : ", String::utf8(nif_display_name(current_node).c_str()));
            list<NiObjectRef> children = current_node->GetRefs();
            Node* custom_nif_node3d_root = convert_nif_node_to_godot(current_node, parent);
            if ((custom_nif_node3d_root != nullptr) && (custom_nif_node3d_root != parent)) {
                parent->add_child(custom_nif_node3d_root);
                for (list<NiObjectRef>::iterator child = children.begin(); child != children.end(); ++child) {
                        rebuild_nif_tree_in_godot(*child, custom_nif_node3d_root);
                }
            }
            else
            {
                for (list<NiObjectRef>::iterator child = children.begin(); child != children.end(); ++child) {
                        rebuild_nif_tree_in_godot(*child, parent);
                }
            }
    } catch (const std::exception& e) {
        //UtilityFunctions::print("Error loading NIF: ", e.what());
        UtilityFunctions::push_error("Error loading NIF: ", e.what());        
    }
}


// Returns the name/value of a provided NiObject if it has one otherwise it returns the type name of the provided NiObject
std::string GdextNiflib::nif_display_name(const Niflib::NiObjectRef& obj) {
    if (!obj) return "<null>";
    if (auto net = Niflib::DynamicCast<Niflib::NiObjectNET>(obj)) {
        const std::string& n = net->GetName();
        if (!n.empty()) return n;
    }
    return obj->GetType().GetTypeName(); // Fallback
}


// Returns the corresponding enum NiType to a provided string if it exists
GdextNiflib::NiType GdextNiflib::stringToNiType(const std::string& str) {
    static const std::unordered_map<std::string, NiType> map = {
        {"NiObject", NiType::NiObject},
        // {"Ni3dsAlphaAnimator", NiType::Ni3dsAlphaAnimator},
        // {"Ni3dsAnimationNode", NiType::Ni3dsAnimationNode},
        // {"Ni3dsColorAnimator", NiType::Ni3dsColorAnimator},
        // {"Ni3dsMorphShape", NiType::Ni3dsMorphShape},
        // {"Ni3dsParticleSystem", NiType::Ni3dsParticleSystem},
        // {"Ni3dsPathController", NiType::Ni3dsPathController},
        // {"NiParticleModifier", NiType::NiParticleModifier},
        // {"NiPSysCollider", NiType::NiPSysCollider},
        // {"bhkRefObject", NiType::bhkRefObject},
        // {"bhkSerializable", NiType::bhkSerializable},
        // {"bhkWorldObject", NiType::bhkWorldObject},
        // {"bhkEntity", NiType::bhkEntity},
        // {"bhkPhantom", NiType::bhkPhantom},
        // {"bhkShapePhantom", NiType::bhkShapePhantom},
        // {"bhkSimpleShapePhantom", NiType::bhkSimpleShapePhantom},
        // {"bhkAabbPhantom", NiType::bhkAabbPhantom},
        // {"bhkEntityPhantom", NiType::bhkEntityPhantom},
        // {"bhkRigidBody", NiType::bhkRigidBody},
        // {"bhkRigidBodyT", NiType::bhkRigidBodyT},
        // {"bhkCollisionObject", NiType::bhkCollisionObject},
        // {"bhkBlendCollisionObject", NiType::bhkBlendCollisionObject},
        // {"bhkPCollisionObject", NiType::bhkPCollisionObject},
        // {"bhkSPCollisionObject", NiType::bhkSPCollisionObject},
        // {"bhkMouseSpringAction", NiType::bhkMouseSpringAction},
        // {"bhkSpringAction", NiType::bhkSpringAction},
        // {"bhkMotorAction", NiType::bhkMotorAction},
        // {"bhkOrientHingedBodyAction", NiType::bhkOrientHingedBodyAction},
        // {"bhkReorientAction", NiType::bhkReorientAction},
        // {"bhkWorld", NiType::bhkWorld},
        // {"bhkWorldCinfo", NiType::bhkWorldCinfo},
        // {"bhkConstraint", NiType::bhkConstraint},
        // {"bhkBallAndSocketConstraint", NiType::bhkBallAndSocketConstraint},
        // {"bhkLimitedHingeConstraint", NiType::bhkLimitedHingeConstraint},
        // {"bhkHingeConstraint", NiType::bhkHingeConstraint},
        // {"bhkStiffSpringConstraint", NiType::bhkStiffSpringConstraint},
        // {"bhkMalleableConstraint", NiType::bhkMalleableConstraint},
        // {"bhkPrismaticConstraint", NiType::bhkPrismaticConstraint},
        // {"bhkRagdollConstraint", NiType::bhkRagdollConstraint},
        // {"bhkWheelConstraint", NiType::bhkWheelConstraint},
        // {"bhkGenericConstraint", NiType::bhkGenericConstraint},
        // {"bhkFixedConstraint", NiType::bhkFixedConstraint},
        // {"bhkPointToPathConstraint", NiType::bhkPointToPathConstraint},
        // {"bhkCartesianConstraint", NiType::bhkCartesianConstraint},
        // {"bhkConeTwistConstraint", NiType::bhkConeTwistConstraint},
        // {"bhkBallSocketConstraintChain", NiType::bhkBallSocketConstraintChain},
        // {"bhkShapeCollection", NiType::bhkShapeCollection},
        // {"bhkListShape", NiType::bhkListShape},
        // {"bhkTransformShape", NiType::bhkTransformShape},
        // {"bhkMoppBvTreeShape", NiType::bhkMoppBvTreeShape},
        // {"bhkBvTreeShape", NiType::bhkBvTreeShape},
        // {"bhkTriSampledHeightFieldBvTreeShape", NiType::bhkTriSampledHeightFieldBvTreeShape},
        // {"bhkHeightFieldShape", NiType::bhkHeightFieldShape},
        // {"bhkPackedNiTriStripsShape", NiType::bhkPackedNiTriStripsShape},
        // {"bhkNiTriStripsShape", NiType::bhkNiTriStripsShape},
        // {"bhkCompressedMeshShape", NiType::bhkCompressedMeshShape},
        // {"bhkCompressedMeshShapeData", NiType::bhkCompressedMeshShapeData},
        // {"bhkCopyFilter", NiType::bhkCopyFilter},
        // {"bhkConvexListShape", NiType::bhkConvexListShape},
        // {"bhkConvexShape", NiType::bhkConvexShape},
        // {"bhkSphereRepShape", NiType::bhkSphereRepShape},
        // {"bhkCapsuleShape", NiType::bhkCapsuleShape},
        // {"bhkMultiSphereShape", NiType::bhkMultiSphereShape},
        // {"bhkConvexVerticesShape", NiType::bhkConvexVerticesShape},
        // {"bhkBoxShape", NiType::bhkBoxShape},
        // {"bhkTriangleShape", NiType::bhkTriangleShape},
        // {"bhkCylinderShape", NiType::bhkCylinderShape},
        // {"bhkConvexTranslateShape", NiType::bhkConvexTranslateShape},
        // {"bhkConvexTransformShape", NiType::bhkConvexTransformShape},
        // {"bhkPlaneShape", NiType::bhkPlaneShape},
        // {"bhkSimplexShape", NiType::bhkSimplexShape},
        // {"bhkSphereShape", NiType::bhkSphereShape},
        // {"bhkRigidBodyCinfo", NiType::bhkRigidBodyCinfo},
        // {"bhkConstraintCinfo", NiType::bhkConstraintCinfo},
        // {"bhkBallAndSocketConstraintCinfo", NiType::bhkBallAndSocketConstraintCinfo},
        // {"bhkLimitedHingeConstraintCinfo", NiType::bhkLimitedHingeConstraintCinfo},
        // {"bhkHingeConstraintCinfo", NiType::bhkHingeConstraintCinfo},
        // {"bhkStiffSpringConstraintCinfo", NiType::bhkStiffSpringConstraintCinfo},
        // {"bhkMalleableConstraintCinfo", NiType::bhkMalleableConstraintCinfo},
        // {"bhkPrismaticConstraintCinfo", NiType::bhkPrismaticConstraintCinfo},
        // {"bhkRagdollConstraintCinfo", NiType::bhkRagdollConstraintCinfo},
        // {"bhkWheelConstraintCinfo", NiType::bhkWheelConstraintCinfo},
        // {"bhkGenericConstraintCinfo", NiType::bhkGenericConstraintCinfo},
        // {"bhkFixedConstraintCinfo", NiType::bhkFixedConstraintCinfo},
        // {"bhkPointToPathConstraintCinfo", NiType::bhkPointToPathConstraintCinfo},
        // {"bhkConeTwistConstraintCinfo", NiType::bhkConeTwistConstraintCinfo},
        // {"bhkBallSocketChainConstraintCinfo", NiType::bhkBallSocketChainConstraintCinfo},
        // {"bhkBreakableConstraint", NiType::bhkBreakableConstraint},
        // {"bhkRagdollTemplate", NiType::bhkRagdollTemplate},
        // {"bhkCMSDMaterial", NiType::bhkCMSDMaterial},
        // {"bhkCMSDChunk", NiType::bhkCMSDChunk},
        // {"bhkCMSDBigTris", NiType::bhkCMSDBigTris},
        // {"bhkBvTreeShapeData", NiType::bhkBvTreeShapeData},
        // {"bhkTriangleShapeBase", NiType::bhkTriangleShapeBase},
        // {"bhkPlane", NiType::bhkPlane},
        // {"bhkAabb", NiType::bhkAabb},
        // {"bhkCMSDTransform", NiType::bhkCMSDTransform},
        // {"bhkWorldObjectCinfo", NiType::bhkWorldObjectCinfo},
        // {"bhkBlendCollisionObjectCinfo", NiType::bhkBlendCollisionObjectCinfo},
        // {"bhkCollisionObjectCinfo", NiType::bhkCollisionObjectCinfo},
        // {"bhkNiTriStripsData", NiType::bhkNiTriStripsData},
        // {"bhkPositionConstraintMotor", NiType::bhkPositionConstraintMotor},
        // {"bhkVelocityConstraintMotor", NiType::bhkVelocityConstraintMotor},
        // {"bhkSpringDamperConstraintMotor", NiType::bhkSpringDamperConstraintMotor},
        // {"bhkCharacterProxy", NiType::bhkCharacterProxy},
        // {"bhkCharacterRigidBody", NiType::bhkCharacterRigidBody},
        // {"bhkRagdollLimitsConstraint", NiType::bhkRagdollLimitsConstraint},
        // {"bhkRagdollLimitsConstraintCinfo", NiType::bhkRagdollLimitsConstraintCinfo},
        // {"bhkBlendController", NiType::bhkBlendController},
        // {"bhkExtraData", NiType::bhkExtraData},
        // {"bhkAngularDashpotAction", NiType::bhkAngularDashpotAction},
        // {"bhkDashpotAction", NiType::bhkDashpotAction},
        // {"bhkLiquidAction", NiType::bhkLiquidAction},
        // {"bhkMotorActionCinfo", NiType::bhkMotorActionCinfo},
        // {"bhkOrientHingedBodyActionCinfo", NiType::bhkOrientHingedBodyActionCinfo},
        // {"bhkReorientActionCinfo", NiType::bhkReorientActionCinfo},
        // {"bhkSpringActionCinfo", NiType::bhkSpringActionCinfo},
        // {"bhkBinaryAction", NiType::bhkBinaryAction},
        // {"bhkAngularConstraintMotor", NiType::bhkAngularConstraintMotor},
        // {"bhkLinearDashpotAction", NiType::bhkLinearDashpotAction},
        // {"bhkWheelAction", NiType::bhkWheelAction},
        // {"bhkPoweredHingeConstraint", NiType::bhkPoweredHingeConstraint},
        // {"bhkPoweredHingeConstraintCinfo", NiType::bhkPoweredHingeConstraintCinfo},
        // {"bhkGroupConstraint", NiType::bhkGroupConstraint},
        // {"bhkHingeLimitsConstraint", NiType::bhkHingeLimitsConstraint},
        // {"bhkPositionConstraintMotor_NextGen", NiType::bhkPositionConstraintMotor_NextGen},
        // {"bhkRotationConstraintMotor", NiType::bhkRotationConstraintMotor},
        // {"bhkMeshShape", NiType::bhkMeshShape},
        // {"bhkNetConstraintsAction", NiType::bhkNetConstraintsAction},
        // {"bhkPoseArray", NiType::bhkPoseArray},
        // {"bhkRagdollConstraint_NextGen", NiType::bhkRagdollConstraint_NextGen},
        // {"bhkRagdollConstraintCinfo_NextGen", NiType::bhkRagdollConstraintCinfo_NextGen},
        // {"bhkRotationalConstraint", NiType::bhkRotationalConstraint},
        // {"bhkRotationalConstraintCinfo", NiType::bhkRotationalConstraintCinfo},
        // {"bhkSweptTransformShape", NiType::bhkSweptTransformShape},
        // {"bhkPcDofBehavior", NiType::bhkPcDofBehavior},
        // {"bhkRagdollConstraintChain", NiType::bhkRagdollConstraintChain},
        // {"hkPackedNiTriStripsData", NiType::hkPackedNiTriStripsData},
        // {"NiExtraData", NiType::NiExtraData},
        // {"NiInterpolator", NiType::NiInterpolator},
        // {"NiKeyBasedInterpolator", NiType::NiKeyBasedInterpolator},
        // {"NiFloatInterpolator", NiType::NiFloatInterpolator},
        // {"NiTransformInterpolator", NiType::NiTransformInterpolator},
        // {"NiPoint3Interpolator", NiType::NiPoint3Interpolator},
        // {"NiPathInterpolator", NiType::NiPathInterpolator},
        // {"NiBoolInterpolator", NiType::NiBoolInterpolator},
        // {"NiBoolTimelineInterpolator", NiType::NiBoolTimelineInterpolator},
        // {"NiBlendInterpolator", NiType::NiBlendInterpolator},
        // {"NiBSplineInterpolator", NiType::NiBSplineInterpolator},
        // {"NiBSplineCompTransformInterpolator", NiType::NiBSplineCompTransformInterpolator},
        // {"NiBSplineTransformInterpolator", NiType::NiBSplineTransformInterpolator},
        // {"NiBSplinePoint3Interpolator", NiType::NiBSplinePoint3Interpolator},
        // {"NiBSplineFloatInterpolator", NiType::NiBSplineFloatInterpolator},
        // {"NiBSplineCompPoint3Interpolator", NiType::NiBSplineCompPoint3Interpolator},
        // {"NiBSplineCompFloatInterpolator", NiType::NiBSplineCompFloatInterpolator},
        // {"NiColorInterpolator", NiType::NiColorInterpolator},
        // {"NiBSplineColorInterpolator", NiType::NiBSplineColorInterpolator},
        // {"NiBSplineCompColorInterpolator", NiType::NiBSplineCompColorInterpolator},
        // {"NiBlendBoolInterpolator", NiType::NiBlendBoolInterpolator},
        // {"NiBlendFloatInterpolator", NiType::NiBlendFloatInterpolator},
        // {"NiBlendPoint3Interpolator", NiType::NiBlendPoint3Interpolator},
        // {"NiBlendTransformInterpolator", NiType::NiBlendTransformInterpolator},
        // {"NiFloatExtraData", NiType::NiFloatExtraData},
        // {"NiBinaryExtraData", NiType::NiBinaryExtraData},
        // {"NiColorExtraData", NiType::NiColorExtraData},
        // {"NiIntegerExtraData", NiType::NiIntegerExtraData},
        // {"NiFloatsExtraData", NiType::NiFloatsExtraData},
        // {"NiIntegersExtraData", NiType::NiIntegersExtraData},
        // {"NiStringsExtraData", NiType::NiStringsExtraData},
        // {"NiTextKeyExtraData", NiType::NiTextKeyExtraData},
        // {"NiVertWeightsExtraData", NiType::NiVertWeightsExtraData},
        // {"NiAlphaController", NiType::NiAlphaController},
        // {"NiRollController", NiType::NiRollController},
        // {"NiUVController", NiType::NiUVController},
        // {"NiUVData", NiType::NiUVData},
        // {"NiPosData", NiType::NiPosData},
        // {"NiPathController", NiType::NiPathController},
        // {"NiLookAtController", NiType::NiLookAtController},
        // {"NiKeyframeData", NiType::NiKeyframeData},
        // {"NiTransformData", NiType::NiTransformData},
        // {"NiMorphData", NiType::NiMorphData},
        // {"NiFloatData", NiType::NiFloatData},
        // {"NiVisController", NiType::NiVisController},
        // {"NiVisData", NiType::NiVisData},
        // {"NiBillboardNode", NiType::NiBillboardNode},
        {"NiTriShape", NiType::NiTriShape},
        {"NiTriShapeData", NiType::NiTriShapeData},
        // {"NiTriStrips", NiType::NiTriStrips},
        // {"NiTriStripsData", NiType::NiTriStripsData},
        // {"NiParticles", NiType::NiParticles},
        // {"NiRotatingParticles", NiType::NiRotatingParticles},
        // {"NiParticlesData", NiType::NiParticlesData},
        // {"NiAutoNormalParticles", NiType::NiAutoNormalParticles},
        // {"NiAutoNormalParticlesData", NiType::NiAutoNormalParticlesData},
        // {"NiPSysData", NiType::NiPSysData},
        // {"NiMesh", NiType::NiMesh},
        // {"NiLines", NiType::NiLines},
        // {"NiLinesData", NiType::NiLinesData},
        // {"NiTriBasedGeom", NiType::NiTriBasedGeom},
        // {"NiGeometryData", NiType::NiGeometryData},
        // {"NiTriBasedGeomData", NiType::NiTriBasedGeomData},
        // {"NiClod", NiType::NiClod},
        // {"NiClodData", NiType::NiClodData},
        // {"NiParticleMeshes", NiType::NiParticleMeshes},
        // {"NiParticleMeshesData", NiType::NiParticleMeshesData},
        // {"NiSkinData", NiType::NiSkinData},
        // {"NiSkinInstance", NiType::NiSkinInstance},
        // {"NiSkinPartition", NiType::NiSkinPartition},
        // {"NiTexture", NiType::NiTexture},
        {"NiSourceTexture", NiType::NiSourceTexture},
        // {"NiSourceCubeMap", NiType::NiSourceCubeMap},
        // {"NiPixelData", NiType::NiPixelData},
        // {"NiPalette", NiType::NiPalette},
        // {"NiDitherProperty", NiType::NiDitherProperty},
        // {"NiAlphaProperty", NiType::NiAlphaProperty},
        // {"NiMaterialProperty", NiType::NiMaterialProperty},
        {"NiTexturingProperty", NiType::NiTexturingProperty},
        // {"NiTextureProperty", NiType::NiTextureProperty},
        // {"NiWireframeProperty", NiType::NiWireframeProperty},
        {"NiZBufferProperty", NiType::NiZBufferProperty},
        // {"NiRendererSpecificProperty", NiType::NiRendererSpecificProperty},
        // {"NiShadeProperty", NiType::NiShadeProperty},
        // {"NiSpecularProperty", NiType::NiSpecularProperty},
        // {"NiStencilProperty", NiType::NiStencilProperty},
        {"NiVertexColorProperty", NiType::NiVertexColorProperty},
        // {"NiFogProperty", NiType::NiFogProperty},
        // {"NiLODNode", NiType::NiLODNode},
        // {"NiRangeLODData", NiType::NiRangeLODData},
        // {"NiScreenLODData", NiType::NiScreenLODData},
        // {"NiBSPArrayController", NiType::NiBSPArrayController},
        // {"NiBSplineBasisData", NiType::NiBSplineBasisData},
        // {"NiBSplineData", NiType::NiBSplineData},
        // {"NiBSplineFloatData", NiType::NiBSplineFloatData},
        // {"NiBSplineCompFloatData", NiType::NiBSplineCompFloatData},
        // {"NiBSplinePoint3Data", NiType::NiBSplinePoint3Data},
        // {"NiBSplineCompPoint3Data", NiType::NiBSplineCompPoint3Data},
        // {"NiBSplineColorData", NiType::NiBSplineColorData},
        // {"NiBSplineCompColorData", NiType::NiBSplineCompColorData},
        // {"NiBSplineTransformData", NiType::NiBSplineTransformData},
        // {"NiBSplineCompTransformData", NiType::NiBSplineCompTransformData},
        // {"NiParticlesGravity", NiType::NiParticlesGravity},
        // {"NiParticlesGrowFade", NiType::NiParticlesGrowFade},
        // {"NiParticleColorModifier", NiType::NiParticleColorModifier},
        // {"NiParticleGrowFade", NiType::NiParticleGrowFade},
        // {"NiPSysCollider", NiType::NiPSysCollider},
        // {"NiPSysGravityModifier", NiType::NiPSysGravityModifier},
        // {"NiPSysSpawnModifier", NiType::NiPSysSpawnModifier},
        // {"NiPlanarCollider", NiType::NiPlanarCollider},
        // {"NiSphericalCollider", NiType::NiSphericalCollider},
        // {"NiPSysPositionModifier", NiType::NiPSysPositionModifier},
        // {"NiPSysRotationModifier", NiType::NiPSysRotationModifier},
        // {"NiPSysResetOnLoopCtlr", NiType::NiPSysResetOnLoopCtlr},
        // {"NiPSysUpdateCtlr", NiType::NiPSysUpdateCtlr},
        // {"NiPSysBombModifier", NiType::NiPSysBombModifier},
        // {"NiPSysDragModifier", NiType::NiPSysDragModifier},
        // {"NiPSysTurbulenceFieldModifier", NiType::NiPSysTurbulenceFieldModifier},
        // {"NiPSysVortexFieldModifier", NiType::NiPSysVortexFieldModifier},
        // {"NiParticleSystem", NiType::NiParticleSystem},
        // {"NiPSysVolumeEmitter", NiType::NiPSysVolumeEmitter},
        // {"NiPSysBoxEmitter", NiType::NiPSysBoxEmitter},
        // {"NiPSysCylinderEmitter", NiType::NiPSysCylinderEmitter},
        // {"NiPSysMeshEmitter", NiType::NiPSysMeshEmitter},
        // {"NiPSysSphereEmitter", NiType::NiPSysSphereEmitter},
        // {"NiPSysCircleEmitter", NiType::NiPSysCircleEmitter},
        // {"NiPSParticleSystem", NiType::NiPSParticleSystem},
        // {"NiPSSimulator", NiType::NiPSSimulator},
        // {"NiPSSimulatorGeneralStep", NiType::NiPSSimulatorGeneralStep},
        // {"NiPSSimulatorForcesStep", NiType::NiPSSimulatorForcesStep},
        // {"NiPSSimulatorCollidersStep", NiType::NiPSSimulatorCollidersStep},
        // {"NiPSSimulatorFinalStep", NiType::NiPSSimulatorFinalStep},
        // {"NiPSFacingQuadGenerator", NiType::NiPSFacingQuadGenerator},
        // {"NiPSAlignedQuadGenerator", NiType::NiPSAlignedQuadGenerator},
        // {"NiPSMeshParticleSystem", NiType::NiPSMeshParticleSystem},
        // {"NiPSForce", NiType::NiPSForce},
        // {"NiPSSpawner", NiType::NiPSSpawner},
        // {"NiPSBombForce", NiType::NiPSBombForce},
        // {"NiPSDragForce", NiType::NiPSDragForce},
        // {"NiPSGravityForce", NiType::NiPSGravityForce},
        // {"NiPSCollider", NiType::NiPSCollider},
        // {"NiPSSphericalCollider", NiType::NiPSSphericalCollider},
        // {"NiPSPlanarCollider", NiType::NiPSPlanarCollider},
        // {"NiPSEmitter", NiType::NiPSEmitter},
        // {"NiPSEmitterCtlr", NiType::NiPSEmitterCtlr},
        // {"NiPSEmitterDeclinationCtlr", NiType::NiPSEmitterDeclinationCtlr},
        // {"NiPSEmitterDeclinationVarCtlr", NiType::NiPSEmitterDeclinationVarCtlr},
        // {"NiPSEmitterLifeSpanCtlr", NiType::NiPSEmitterLifeSpanCtlr},
        // {"NiPSEmitterSpeedCtlr", NiType::NiPSEmitterSpeedCtlr},
        // {"NiPSEmitterRadiusCtlr", NiType::NiPSEmitterRadiusCtlr},
        // {"NiPSEmitterRotAngleCtlr", NiType::NiPSEmitterRotAngleCtlr},
        // {"NiPSEmitterRotAngleVarCtlr", NiType::NiPSEmitterRotAngleVarCtlr},
        // {"NiPSEmitterRotSpeedCtlr", NiType::NiPSEmitterRotSpeedCtlr},
        // {"NiPSEmitterRotSpeedVarCtlr", NiType::NiPSEmitterRotSpeedVarCtlr},
        // {"NiPSEmitterPlanarAngleCtlr", NiType::NiPSEmitterPlanarAngleCtlr},
        // {"NiPSEmitterPlanarAngleVarCtlr", NiType::NiPSEmitterPlanarAngleVarCtlr},
        // {"NiPSEmitterFloatCtlr", NiType::NiPSEmitterFloatCtlr},
        // {"NiPSForceCtlr", NiType::NiPSForceCtlr},
        // {"NiPSForceBoolCtlr", NiType::NiPSForceBoolCtlr},
        // {"NiPSForceFloatCtlr", NiType::NiPSForceFloatCtlr},
        // {"NiPSBoxEmitter", NiType::NiPSBoxEmitter},
        // {"NiPSCylinderEmitter", NiType::NiPSCylinderEmitter},
        // {"NiPSMeshEmitter", NiType::NiPSMeshEmitter},
        // {"NiPSSphereEmitter", NiType::NiPSSphereEmitter},
        // {"NiMeshPSysData", NiType::NiMeshPSysData},
        // {"NiAVObject", NiType::NiAVObject},
        // {"NiAlphaAccumulator", NiType::NiAlphaAccumulator},
        // {"NiBackToFrontAccumulator", NiType::NiBackToFrontAccumulator},
        // {"NiTimeController", NiType::NiTimeController},
        // {"NiBSPNode", NiType::NiBSPNode},
        // {"NiBone", NiType::NiBone},
        // {"NiCamera", NiType::NiCamera},
        // {"NiCollisionData", NiType::NiCollisionData},
        // {"NiCollisionObject", NiType::NiCollisionObject},
        // {"NiGeometry", NiType::NiGeometry},
        // {"NiLight", NiType::NiLight},
        // {"NiAmbientLight", NiType::NiAmbientLight},
        // {"NiDirectionalLight", NiType::NiDirectionalLight},
        // {"NiPointLight", NiType::NiPointLight},
        // {"NiSpotLight", NiType::NiSpotLight},
        {"NiNode", NiType::NiNode},
        // {"NiDynamicEffect", NiType::NiDynamicEffect},
        // {"NiSwitchNode", NiType::NiSwitchNode},
        // {"NiScreenElements", NiType::NiScreenElements},
        // {"NiScreenElementsData", NiType::NiScreenElementsData},
        // {"NiScreenTexture", NiType::NiScreenTexture},
        // {"NiTextureEffect", NiType::NiTextureEffect},
        // {"NiCullingProcess", NiType::NiCullingProcess},
        // {"NiIntegersExtraDataPoint3Controller", NiType::NiIntegersExtraDataPoint3Controller},
        // {"NiFloatsExtraDataPoint3Controller", NiType::NiFloatsExtraDataPoint3Controller},
        // {"NiFloatsExtraDataController", NiType::NiFloatsExtraDataController},
        // {"NiFloatsExtraDataInterpController", NiType::NiFloatsExtraDataInterpController},
        // {"NiFloatsExtraDataMorphController", NiType::NiFloatsExtraDataMorphController},
        // {"NiIntegersExtraDataController", NiType::NiIntegersExtraDataController},
        // {"NiLightColorController", NiType::NiLightColorController},
        // {"NiTextureTransformController", NiType::NiTextureTransformController},
        // {"NiLightDimmerController", NiType::NiLightDimmerController},
        // {"NiAmbientLightColorController", NiType::NiAmbientLightColorController},
        // {"NiPointLightColorController", NiType::NiPointLightColorController},
        // {"NiDirectionalLightColorController", NiType::NiDirectionalLightColorController},
        // {"NiSpotLightColorController", NiType::NiSpotLightColorController},
        // {"NiLightRadiusController", NiType::NiLightRadiusController},
        // {"NiParticleSystemController", NiType::NiParticleSystemController},
        // {"NiPSysEmitterCtlr", NiType::NiPSysEmitterCtlr},
        // {"NiPSysModifierCtlr", NiType::NiPSysModifierCtlr},
        // {"NiPSysModifierBoolCtlr", NiType::NiPSysModifierBoolCtlr},
        // {"NiPSysModifierFloatCtlr", NiType::NiPSysModifierFloatCtlr},
        // {"NiRGBALightColorController", NiType::NiRGBALightColorController},
        // {"NiDefaultAVObjectPalette", NiType::NiDefaultAVObjectPalette},
        // {"NiBones", NiType::NiBones},
        // {"NiScreenLODNode", NiType::NiScreenLODNode},
        {"NiStringExtraData", NiType::NiStringExtraData},
        // {"NiSwitchStringExtraData", NiType::NiSwitchStringExtraData},
        // {"NiAdditionalGeometryData", NiType::NiAdditionalGeometryData},
        // {"NiArkTextureExtraData", NiType::NiArkTextureExtraData},
        // {"NiArkAnimationExtraData", NiType::NiArkAnimationExtraData},
        // {"NiBinaryVoxelData", NiType::NiBinaryVoxelData},
        // {"NiBooleanExtraData", NiType::NiBooleanExtraData},
        // {"NiCollisionSwitch", NiType::NiCollisionSwitch},
        // {"NiColorExtraDataController", NiType::NiColorExtraDataController},
        // {"NiGeometryMorpherController", NiType::NiGeometryMorpherController},
        // {"NiKeyframeController", NiType::NiKeyframeController},
        // {"NiFlipController", NiType::NiFlipController},
        // {"NiTransformController", NiType::NiTransformController},
        // {"NiFurSpringController", NiType::NiFurSpringController},
        // {"NiLookAtInterpolator", NiType::NiLookAtInterpolator},
        // {"NiBSplineBasisData_NiAVObjectPalette", NiType::NiBSplineBasisData_NiAVObjectPalette},
        // {"NiBSplineInterpolator", NiType::NiBSplineInterpolator},
        // {"NiEvaluator", NiType::NiEvaluator},
        // {"NiFloatEvaluator", NiType::NiFloatEvaluator},
        // {"NiPoint3Evaluator", NiType::NiPoint3Evaluator},
        // {"NiTransformEvaluator", NiType::NiTransformEvaluator},
        // {"NiBSplineEvaluator", NiType::NiBSplineEvaluator},
        // {"NiBSplineFloatEvaluator", NiType::NiBSplineFloatEvaluator},
        // {"NiBSplinePoint3Evaluator", NiType::NiBSplinePoint3Evaluator},
        // {"NiBSplineTransformEvaluator", NiType::NiBSplineTransformEvaluator},
        // {"NiColorExtraDataControllerDeprecated", NiType::NiColorExtraDataControllerDeprecated},
        // {"NiArkControllerTree", NiType::NiArkControllerTree},
        // {"NiArkAnimationInfo", NiType::NiArkAnimationInfo},
        // {"NiArkKeyframeInfo", NiType::NiArkKeyframeInfo},
        // {"NiArkSequenceManager", NiType::NiArkSequenceManager},
        // {"NiArkTextureExtraDataDeprecated", NiType::NiArkTextureExtraDataDeprecated},
        // {"NiArkShaderExtraData", NiType::NiArkShaderExtraData},
        // {"NiArkShaderExtraDataController", NiType::NiArkShaderExtraDataController},
        // {"NiArkTextureFlipController", NiType::NiArkTextureFlipController},
        // {"NiArkTextureSRTController", NiType::NiArkTextureSRTController},
        // {"NiArkParameterController", NiType::NiArkParameterController},
        // {"NiArkMaterialExtraData", NiType::NiArkMaterialExtraData},
        // {"NiArkIOExtraData", NiType::NiArkIOExtraData},
        // {"NiArkAnimationInstance", NiType::NiArkAnimationInstance},
        // {"NiArkAnimationSequence", NiType::NiArkAnimationSequence},
        // {"NiArkBsplineData", NiType::NiArkBsplineData},
        // {"NiArkBsplineBasisData", NiType::NiArkBsplineBasisData},
        // {"NiArkAnimationSequenceData", NiType::NiArkAnimationSequenceData},
        // {"NiArkMorphWeightController", NiType::NiArkMorphWeightController},
        // {"NiArkMaterialColorController", NiType::NiArkMaterialColorController},
        // {"NiArkMaterialAlphaController", NiType::NiArkMaterialAlphaController},
        // {"NiArkTextureRepeatController", NiType::NiArkTextureRepeatController},
        // {"NiArkBSplineInterpolator", NiType::NiArkBSplineInterpolator},
        // {"NiArkFloatExtraDataController", NiType::NiArkFloatExtraDataController},
        // {"NiArkPathController", NiType::NiArkPathController},
        // {"NiArkLookAtController", NiType::NiArkLookAtController},
        // {"NiArkSwitchController", NiType::NiArkSwitchController},
        // {"NiArkStateController", NiType::NiArkStateController},
        // {"NiArkVisibilityController", NiType::NiArkVisibilityController},
        // {"NiArkStringPalette", NiType::NiArkStringPalette},
        // {"NiArkTriangles", NiType::NiArkTriangles},
        // {"NiArkCylinder", NiType::NiArkCylinder},
        // {"NiArkBoard", NiType::NiArkBoard},
        // {"NiArkQuad", NiType::NiArkQuad},
        // {"NiArkOcclusionQuery", NiType::NiArkOcclusionQuery},
        // {"NiArkOcclusionQueryRegion", NiType::NiArkOcclusionQueryRegion},
        // {"NiArkOcclusionQueryGroup", NiType::NiArkOcclusionQueryGroup},
        // {"NiArkOcclusionQueryScene", NiType::NiArkOcclusionQueryScene},
        // {"NiArkOcclusionQueryTestGroup", NiType::NiArkOcclusionQueryTestGroup},
        // {"NiArkVisibleSet", NiType::NiArkVisibleSet},
        // {"NiArkPortal", NiType::NiArkPortal},
        // {"NiArkRoom", NiType::NiArkRoom},
        // {"NiArkSceneGraph", NiType::NiArkSceneGraph},
        // {"NiArkOccluder", NiType::NiArkOccluder},
        // {"NiArkPortalGraph", NiType::NiArkPortalGraph},
        // {"NiArkPortalGraphNode", NiType::NiArkPortalGraphNode},
        // {"NiArkPortalGraphCtrlr", NiType::NiArkPortalGraphCtrlr},
        // {"NiArkPortalGraphSwitchCtrlr", NiType::NiArkPortalGraphSwitchCtrlr},
        // {"NiArkPortalGraphColorCtrlr", NiType::NiArkPortalGraphColorCtrlr},
        // {"NiArkPortalGraphAlphaCtrlr", NiType::NiArkPortalGraphAlphaCtrlr},
        // {"NiArkPortalGraphUVCtrlr", NiType::NiArkPortalGraphUVCtrlr},
        // {"NiArkPortalGraphTexTransCtrlr", NiType::NiArkPortalGraphTexTransCtrlr},
        // {"NiArkPortalGraphMorphCtrlr", NiType::NiArkPortalGraphMorphCtrlr},
        // {"NiArkPortalGraphVisCtrlr", NiType::NiArkPortalGraphVisCtrlr},
        // {"NiArkPortalGraphFloatInterpCtrlr", NiType::NiArkPortalGraphFloatInterpCtrlr},
        // {"NiArkPortalGraphPoint3InterpCtrlr", NiType::NiArkPortalGraphPoint3InterpCtrlr},
        // {"NiArkPortalGraphTransformInterpCtrlr", NiType::NiArkPortalGraphTransformInterpCtrlr},
        // {"NiArkPortalGraphPathCtrlr", NiType::NiArkPortalGraphPathCtrlr},
        // {"NiArkPortalGraphLookAtCtrlr", NiType::NiArkPortalGraphLookAtCtrlr},
        // {"NiArkStub", NiType::NiArkStub},
        // {"NiArkShaderNodes", NiType::NiArkShaderNodes},
        // {"NiArkShaderNode", NiType::NiArkShaderNode},
        // {"NiArkShaderPassthroughNode", NiType::NiArkShaderPassthroughNode},
        // {"NiArkShaderColorNode", NiType::NiArkShaderColorNode},
        // {"NiArkShaderFloatNode", NiType::NiArkShaderFloatNode},
        // {"NiArkShaderVectorNode", NiType::NiArkShaderVectorNode},
        // {"NiArkShaderTextureNode", NiType::NiArkShaderTextureNode},
        // {"NiArkShaderFresnelNode", NiType::NiArkShaderFresnelNode},
        // {"NiArkShaderPalette", NiType::NiArkShaderPalette},
        // {"NiArkShaderFloatInterpCtrler", NiType::NiArkShaderFloatInterpCtrler},
        // {"NiArkShaderPoint3InterpCtrler", NiType::NiArkShaderPoint3InterpCtrler},
        // {"NiArkShaderTransformInterpCtrler", NiType::NiArkShaderTransformInterpCtrler},
        // {"NiArkShaderColorInterpCtrler", NiType::NiArkShaderColorInterpCtrler},
        // {"NiArkShaderMatrix3InterpCtrler", NiType::NiArkShaderMatrix3InterpCtrler},
        // {"NiArkShaderFeature", NiType::NiArkShaderFeature},
        // {"NiArkShaderStage", NiType::NiArkShaderStage},
        // {"NiArkShaderParameter", NiType::NiArkShaderParameter},
        // {"NiArkShaderConstantMap", NiType::NiArkShaderConstantMap},
        // {"NiArkShaderAccumulatedPass", NiType::NiArkShaderAccumulatedPass},
        // {"NiArkShaderProperties", NiType::NiArkShaderProperties},
        // {"NiArkShaderLightProperty", NiType::NiArkShaderLightProperty},
        // {"NiArkShaderMaterialProperty", NiType::NiArkShaderMaterialProperty},
        // {"NiArkShaderGeometryProperty", NiType::NiArkShaderGeometryProperty},
        // {"NiArkShaderTextureProperty", NiType::NiArkShaderTextureProperty},
        // {"NiArkShaderGeometry", NiType::NiArkShaderGeometry},
        // {"NiArkShaderMesh", NiType::NiArkShaderMesh},
        // {"NiArkShaderPolygon", NiType::NiArkShaderPolygon},
        // {"NiArkShaderSubmesh", NiType::NiArkShaderSubmesh},
        // {"NiArkShaderGeometryData", NiType::NiArkShaderGeometryData},
        // {"NiArkShaderMeshData", NiType::NiArkShaderMeshData},
        // {"NiArkShaderPolygonData", NiType::NiArkShaderPolygonData},
        // {"NiArkShaderSubmeshData", NiType::NiArkShaderSubmeshData},
        // {"NiArkShaderScreenTexture", NiType::NiArkShaderScreenTexture},
        // {"NiArkShaderTextureAnimationProperty", NiType::NiArkShaderTextureAnimationProperty},
        // {"NiArkShaderParameterAnimationProperty", NiType::NiArkShaderParameterAnimationProperty},
        // {"NiArkShaderTextureTransformProperty", NiType::NiArkShaderTextureTransformProperty},
        // {"NiArkShaderColorExtraData", NiType::NiArkShaderColorExtraData},
        // {"NiArkShaderFloatExtraData", NiType::NiArkShaderFloatExtraData},
        // {"NiArkShaderPoint3ExtraData", NiType::NiArkShaderPoint3ExtraData},
        // {"NiArkShaderTransformExtraData", NiType::NiArkShaderTransformExtraData},
        // {"NiArkShaderColorKeyData", NiType::NiArkShaderColorKeyData},
        // {"NiArkShaderFloatKeyData", NiType::NiArkShaderFloatKeyData},
        // {"NiArkShaderPoint3KeyData", NiType::NiArkShaderPoint3KeyData},
        // {"NiArkShaderTransformKeyData", NiType::NiArkShaderTransformKeyData},
        // {"NiArkShaderColorKey", NiType::NiArkShaderColorKey},
        // {"NiArkShaderFloatKey", NiType::NiArkShaderFloatKey},
        // {"NiArkShaderPoint3Key", NiType::NiArkShaderPoint3Key},
        // {"NiArkShaderTransformKey", NiType::NiArkShaderTransformKey},
        // {"NiArkShaderSkinInstance", NiType::NiArkShaderSkinInstance},
        // {"NiArkShaderSkinData", NiType::NiArkShaderSkinData},
        // {"NiArkShaderSkinPartition", NiType::NiArkShaderSkinPartition},
        // {"NiArkShaderControllerManager", NiType::NiArkShaderControllerManager},
        // {"NiArkShaderControllerSequence", NiType::NiArkShaderControllerSequence},
        // {"NiArkShaderDefaultAVObjectPalette", NiType::NiArkShaderDefaultAVObjectPalette},
        // {"NiArkShaderBsplineData", NiType::NiArkShaderBsplineData},
        // {"NiArkShaderBsplineBasisData", NiType::NiArkShaderBsplineBasisData},
        // {"NiArkShaderBlendInterpolator", NiType::NiArkShaderBlendInterpolator},
        // {"NiArkShaderFloatInterpolator", NiType::NiArkShaderFloatInterpolator},
        // {"NiArkShaderPoint3Interpolator", NiType::NiArkShaderPoint3Interpolator},
        // {"NiArkShaderTransformInterpolator", NiType::NiArkShaderTransformInterpolator},
        // {"NiArkShaderColorInterpolator", NiType::NiArkShaderColorInterpolator},
        // {"NiArkShaderBSplineInterpolator", NiType::NiArkShaderBSplineInterpolator},
        // {"NiArkShaderTransformController", NiType::NiArkShaderTransformController},
        // {"NiArkShaderFloatController", NiType::NiArkShaderFloatController},
        // {"NiArkShaderPoint3Controller", NiType::NiArkShaderPoint3Controller},
        // {"NiArkShaderColorController", NiType::NiArkShaderColorController},
        // {"NiArkShaderMatrix3Controller", NiType::NiArkShaderMatrix3Controller},
        // {"NiArkShaderTextureController", NiType::NiArkShaderTextureController},
        // {"NiArkShaderMaterialAlphaController", NiType::NiArkShaderMaterialAlphaController},
        // {"NiArkShaderMaterialColorController", NiType::NiArkShaderMaterialColorController},
        // {"NiArkShaderTextureSRTController", NiType::NiArkShaderTextureSRTController},
        // {"NiArkShaderLookAtController", NiType::NiArkShaderLookAtController},
        // {"NiArkShaderPathController", NiType::NiArkShaderPathController},
        // {"NiArkShaderSwitchController", NiType::NiArkShaderSwitchController},
        // {"NiArkShaderVisibilityController", NiType::NiArkShaderVisibilityController},
        // {"NiArkShaderParameterController", NiType::NiArkShaderParameterController},
        // {"NiArkShaderVertexColorProperty", NiType::NiArkShaderVertexColorProperty},
        // {"NiArkShaderSpecularProperty", NiType::NiArkShaderSpecularProperty},
        // {"NiArkShaderZBufferProperty", NiType::NiArkShaderZBufferProperty},
        // {"NiArkShaderDitherProperty", NiType::NiArkShaderDitherProperty},
        // {"NiArkShaderAlphaProperty", NiType::NiArkShaderAlphaProperty},
        // {"NiArkShaderWireframeProperty", NiType::NiArkShaderWireframeProperty},
        // {"NiArkShaderStencilProperty", NiType::NiArkShaderStencilProperty},
        // {"NiArkShaderTexturingProperty", NiType::NiArkShaderTexturingProperty},
        // {"NiArkShaderFogProperty", NiType::NiArkShaderFogProperty},
        // {"NiArkShaderMaterialProperty", NiType::NiArkShaderMaterialProperty},
        // {"NiArkShaderShadeProperty", NiType::NiArkShaderShadeProperty},
        // {"NiArkShaderPolygonModesProperty", NiType::NiArkShaderPolygonModesProperty},
        // {"NiArkShaderTextureProperty", NiType::NiArkShaderTextureProperty},
        // {"NiArkShaderTexTransProperty", NiType::NiArkShaderTexTransProperty},
        // {"NiArkShaderTexRollProperty", NiType::NiArkShaderTexRollProperty},
        // {"NiArkShaderTexFlipProperty", NiType::NiArkShaderTexFlipProperty},
        // {"NiArkShaderLightingProperty", NiType::NiArkShaderLightingProperty},
        // {"NiArkShaderTextureCoordSet", NiType::NiArkShaderTextureCoordSet},
        // {"NiArkShaderTextureCoordSetData", NiType::NiArkShaderTextureCoordSetData},
        // {"NiArkShaderTextureFlipController", NiType::NiArkShaderTextureFlipController},
        // {"NiArkShaderTextureRepeatController", NiType::NiArkShaderTextureRepeatController},
        // {"NiArkShaderTextureFlipControllerData", NiType::NiArkShaderTextureFlipControllerData},
        // {"NiArkShaderTextureRepeatControllerData", NiType::NiArkShaderTextureRepeatControllerData},
        // {"NiArkShaderTextureControllerData", NiType::NiArkShaderTextureControllerData},
        // {"NiArkShaderMaterialControllerData", NiType::NiArkShaderMaterialControllerData},
        // {"NiArkShaderMaterialAlphaControllerData", NiType::NiArkShaderMaterialAlphaControllerData},
        // {"NiArkShaderMaterialColorControllerData", NiType::NiArkShaderMaterialColorControllerData},
        // {"NiArkShaderMeshModifier", NiType::NiArkShaderMeshModifier},
        // {"NiArkShaderSkinningMeshModifier", NiType::NiArkShaderSkinningMeshModifier},
        // {"NiArkShaderSpringController", NiType::NiArkShaderSpringController},
        // {"NiArkShaderController", NiType::NiArkShaderController},
        // {"NiArkShaderSwitchNode", NiType::NiArkShaderSwitchNode},
        // {"NiArkShaderRangeLODNode", NiType::NiArkShaderRangeLODNode},
        // {"NiArkShaderScreenLODNode", NiType::NiArkShaderScreenLODNode},
        // {"NiArkShaderLODData", NiType::NiArkShaderLODData},
        // {"NiArkShaderRangeLODData", NiType::NiArkShaderRangeLODData},
        // {"NiArkShaderScreenLODData", NiType::NiArkShaderScreenLODData},
        // {"NiArkShaderPortal", NiType::NiArkShaderPortal},
        // {"NiArkShaderPortalGraph", NiType::NiArkShaderPortalGraph},
        // {"NiArkShaderPortalGraphNode", NiType::NiArkShaderPortalGraphNode},
        // {"NiArkShaderPortalGraphCtrlr", NiType::NiArkShaderPortalGraphCtrlr},
        // {"NiArkShaderPortalGraphSwitchCtrlr", NiType::NiArkShaderPortalGraphSwitchCtrlr},
        // {"NiArkShaderPortalGraphColorCtrlr", NiType::NiArkShaderPortalGraphColorCtrlr},
        // {"NiArkShaderPortalGraphAlphaCtrlr", NiType::NiArkShaderPortalGraphAlphaCtrlr},
        // {"NiArkShaderPortalGraphUVCtrlr", NiType::NiArkShaderPortalGraphUVCtrlr},
        // {"NiArkShaderPortalGraphTexTransCtrlr", NiType::NiArkShaderPortalGraphTexTransCtrlr},
        // {"NiArkShaderPortalGraphMorphCtrlr", NiType::NiArkShaderPortalGraphMorphCtrlr},
        // {"NiArkShaderPortalGraphVisCtrlr", NiType::NiArkShaderPortalGraphVisCtrlr},
        // {"NiArkShaderPortalGraphFloatInterpCtrlr", NiType::NiArkShaderPortalGraphFloatInterpCtrlr},
        // {"NiArkShaderPortalGraphPoint3InterpCtrlr", NiType::NiArkShaderPortalGraphPoint3InterpCtrlr},
        // {"NiArkShaderPortalGraphTransformInterpCtrlr", NiType::NiArkShaderPortalGraphTransformInterpCtrlr},
        // {"NiArkShaderPortalGraphPathCtrlr", NiType::NiArkShaderPortalGraphPathCtrlr},
        // {"NiArkShaderPortalGraphLookAtCtrlr", NiType::NiArkShaderPortalGraphLookAtCtrlr},
        // {"NiArkShaderOccluder", NiType::NiArkShaderOccluder},
        // {"NiArkShaderOcclusionQuery", NiType::NiArkShaderOcclusionQuery},
        // {"NiArkShaderOcclusionQueryRegion", NiType::NiArkShaderOcclusionQueryRegion},
        // {"NiArkShaderOcclusionQueryGroup", NiType::NiArkShaderOcclusionQueryGroup},
        // {"NiArkShaderOcclusionQueryScene", NiType::NiArkShaderOcclusionQueryScene},
        // {"NiArkShaderOcclusionQueryTestGroup", NiType::NiArkShaderOcclusionQueryTestGroup},
        // {"NiArkShaderVisibleSet", NiType::NiArkShaderVisibleSet},
        // {"NiArkShaderRoom", NiType::NiArkShaderRoom},
        // {"NiArkShaderSceneGraph", NiType::NiArkShaderSceneGraph},
        // {"NiArkShaderASMParticleSystem", NiType::NiArkShaderASMParticleSystem},
        // {"NiArkShaderASMData", NiType::NiArkShaderASMData},
        // {"NiArkShaderASMController", NiType::NiArkShaderASMController},
        // {"NiArkShaderASMControllerData", NiType::NiArkShaderASMControllerData},
        // {"NiArkShaderASMGenerator", NiType::NiArkShaderASMGenerator},
        // {"NiArkShaderAMMAppearance", NiType::NiArkShaderAMMAppearance},
        // {"NiArkShaderAMMData", NiType::NiArkShaderAMMData},
        // {"NiArkShaderAmmAppearance", NiType::NiArkShaderAmmAppearance},
        // {"NiArkShaderAmmData", NiType::NiArkShaderAmmData},
        // {"NiArkShaderDoodad", NiType::NiArkShaderDoodad},
        // {"NiArkShaderDoodadData", NiType::NiArkShaderDoodadData},
        // {"NiArkShaderDoodadController", NiType::NiArkShaderDoodadController},
        // {"NiArkShaderDoodadControllerData", NiType::NiArkShaderDoodadControllerData},
        // {"NiArkShaderCustomization", NiType::NiArkShaderCustomization},
        // {"NiArkShaderCustomizationData", NiType::NiArkShaderCustomizationData},
        // {"NiArkShaderCustomizationController", NiType::NiArkShaderCustomizationController},
        // {"NiArkShaderCustomizationControllerData", NiType::NiArkShaderCustomizationControllerData},
        // {"NiArkShaderCloth", NiType::NiArkShaderCloth},
        // {"NiArkShaderClothData", NiType::NiArkShaderClothData},
        // {"NiArkShaderClothController", NiType::NiArkShaderClothController},
        // {"NiArkShaderClothControllerData", NiType::NiArkShaderClothControllerData},
        // {"NiArkShaderClothMeshModifier", NiType::NiArkShaderClothMeshModifier},
        // {"NiArkShaderClothSpringController", NiType::NiArkShaderClothSpringController},
        // {"NiArkShaderClothSwitchController", NiType::NiArkShaderClothSwitchController},
        // {"NiArkShaderClothVisibilityController", NiType::NiArkShaderClothVisibilityController},
        // {"NiArkShaderClothParameterController", NiType::NiArkShaderClothParameterController},
        // {"NiArkShaderClothTextureController", NiType::NiArkShaderClothTextureController},
        // {"NiArkShaderClothTextureRepeatController", NiType::NiArkShaderClothTextureRepeatController},
        // {"NiArkShaderClothMaterialAlphaController", NiType::NiArkShaderClothMaterialAlphaController},
        // {"NiArkShaderClothMaterialColorController", NiType::NiArkShaderClothMaterialColorController},
        // {"NiArkShaderClothTextureFlipController", NiType::NiArkShaderClothTextureFlipController},
        // {"NiArkShaderClothTextureSRTController", NiType::NiArkShaderClothTextureSRTController},
        // {"NiArkShaderClothLookAtController", NiType::NiArkShaderClothLookAtController},
        // {"NiArkShaderClothPathController", NiType::NiArkShaderClothPathController},
        // {"NiArkShaderClothBSplineInterpolator", NiType::NiArkShaderClothBSplineInterpolator},
        // {"NiArkShaderClothBlendInterpolator", NiType::NiArkShaderClothBlendInterpolator},
        // {"NiArkShaderClothFloatInterpolator", NiType::NiArkShaderClothFloatInterpolator},
        // {"NiArkShaderClothPoint3Interpolator", NiType::NiArkShaderClothPoint3Interpolator},
        // {"NiArkShaderClothTransformInterpolator", NiType::NiArkShaderClothTransformInterpolator},
        // {"NiArkShaderClothColorInterpolator", NiType::NiArkShaderClothColorInterpolator},
        // {"NiArkShaderClothBSplineBasisData", NiType::NiArkShaderClothBSplineBasisData},
        // {"NiArkShaderClothBSplineData", NiType::NiArkShaderClothBSplineData},
        // {"NiArkShaderClothDefaultAVObjectPalette", NiType::NiArkShaderClothDefaultAVObjectPalette},
        // {"NiArkShaderClothControllerManager", NiType::NiArkShaderClothControllerManager},
        // {"NiArkShaderClothControllerSequence", NiType::NiArkShaderClothControllerSequence},
        // {"NiArkShaderClothTextureCoordSet", NiType::NiArkShaderClothTextureCoordSet},
        // {"NiArkShaderClothTextureCoordSetData", NiType::NiArkShaderClothTextureCoordSetData},
        // {"NiArkShaderClothSkinInstance", NiType::NiArkShaderClothSkinInstance},
        // {"NiArkShaderClothSkinData", NiType::NiArkShaderClothSkinData},
        // {"NiArkShaderClothSkinPartition", NiType::NiArkShaderClothSkinPartition},
        // {"NiArkShaderClothTriangles", NiType::NiArkShaderClothTriangles},
        // {"NiArkShaderClothCylinder", NiType::NiArkShaderClothCylinder},
        // {"NiArkShaderClothBoard", NiType::NiArkShaderClothBoard},
        // {"NiArkShaderClothQuad", NiType::NiArkShaderClothQuad},
        // {"NiArkShaderClothPortal", NiType::NiArkShaderClothPortal},
        // {"NiArkShaderClothRoom", NiType::NiArkShaderClothRoom},
        // {"NiArkShaderClothSceneGraph", NiType::NiArkShaderClothSceneGraph},
        // {"NiArkShaderClothStub", NiType::NiArkShaderClothStub},
        // {"NiArkShaderClothShaderNodes", NiType::NiArkShaderClothShaderNodes},
        // {"NiArkShaderClothShaderNode", NiType::NiArkShaderClothShaderNode},
        // {"NiArkShaderClothShaderPassthroughNode", NiType::NiArkShaderClothShaderPassthroughNode},
        // {"NiArkShaderClothShaderColorNode", NiType::NiArkShaderClothShaderColorNode},
        // {"NiArkShaderClothShaderFloatNode", NiType::NiArkShaderClothShaderFloatNode},
        // {"NiArkShaderClothShaderVectorNode", NiType::NiArkShaderClothShaderVectorNode},
        // {"NiArkShaderClothShaderTextureNode", NiType::NiArkShaderClothShaderTextureNode},
        // {"NiArkShaderClothShaderFresnelNode", NiType::NiArkShaderClothShaderFresnelNode},
        // {"NiArkShaderClothShaderPalette", NiType::NiArkShaderClothShaderPalette},
        // {"NiArkShaderClothShaderFloatInterpCtrler", NiType::NiArkShaderClothShaderFloatInterpCtrler},
        // {"NiArkShaderClothShaderPoint3InterpCtrler", NiType::NiArkShaderClothShaderPoint3InterpCtrler},
        // {"NiArkShaderClothShaderTransformInterpCtrler", NiType::NiArkShaderClothShaderTransformInterpCtrler},
        // {"NiArkShaderClothShaderColorInterpCtrler", NiType::NiArkShaderClothShaderColorInterpCtrler},
        // {"NiArkShaderClothShaderMatrix3InterpCtrler", NiType::NiArkShaderClothShaderMatrix3InterpCtrler},
        // {"NiArkShaderClothShaderFeature", NiType::NiArkShaderClothShaderFeature},
        // {"NiArkShaderClothShaderStage", NiType::NiArkShaderClothShaderStage},
        // {"NiArkShaderClothShaderParameter", NiType::NiArkShaderClothShaderParameter},
        // {"NiArkShaderClothShaderConstantMap", NiType::NiArkShaderClothShaderConstantMap},
        // {"NiArkShaderClothShaderAccumulatedPass", NiType::NiArkShaderClothShaderAccumulatedPass},
        // {"NiArkShaderClothShaderProperties", NiType::NiArkShaderClothShaderProperties},
        // {"NiArkShaderClothShaderLightProperty", NiType::NiArkShaderClothShaderLightProperty},
        // {"NiArkShaderClothShaderMaterialProperty", NiType::NiArkShaderClothShaderMaterialProperty},
        // {"NiArkShaderClothShaderGeometryProperty", NiType::NiArkShaderClothShaderGeometryProperty},
        // {"NiArkShaderClothShaderTextureProperty", NiType::NiArkShaderClothShaderTextureProperty},
        // {"NiArkShaderClothShaderGeometry", NiType::NiArkShaderClothShaderGeometry},
        // {"NiArkShaderClothShaderMesh", NiType::NiArkShaderClothShaderMesh},
        // {"NiArkShaderClothShaderPolygon", NiType::NiArkShaderClothShaderPolygon},
        // {"NiArkShaderClothShaderSubmesh", NiType::NiArkShaderClothShaderSubmesh},
        // {"NiArkShaderClothShaderGeometryData", NiType::NiArkShaderClothShaderGeometryData},
        // {"NiArkShaderClothShaderMeshData", NiType::NiArkShaderClothShaderMeshData},
        // {"NiArkShaderClothShaderPolygonData", NiType::NiArkShaderClothShaderPolygonData},
        // {"NiArkShaderClothShaderSubmeshData", NiType::NiArkShaderClothShaderSubmeshData},
        // {"NiControllerManager", NiType::NiControllerManager},
        // {"NiControllerSequence", NiType::NiControllerSequence},
        // {"NiSequenceStreamHelper", NiType::NiSequenceStreamHelper},
        // {"NiStringPalette", NiType::NiStringPalette},
        // {"NiMultiTargetTransformController", NiType::NiMultiTargetTransformController},
        // {"NiBSBoneLODController", NiType::NiBSBoneLODController},
        // {"NiBlenderNavigator", NiType::NiBlenderNavigator},
        // {"NiParticlesModifier", NiType::NiParticlesModifier},
        // {"NiMaterial", NiType::NiMaterial},
        // {"NiSwitchNodeClone", NiType::NiSwitchNodeClone},
        // {"NiMeshModifier", NiType::NiMeshModifier},
        // {"NiSkinningLODController", NiType::NiSkinningLODController},
        // {"NiSkinningMeshModifier", NiType::NiSkinningMeshModifier},
        // {"NiMeshHWInstance", NiType::NiMeshHWInstance},
        // {"NiPSysMeshUpdateModifier", NiType::NiPSysMeshUpdateModifier},
        // {"NiRenderObject", NiType::NiRenderObject},
        // {"NiProperty", NiType::NiProperty},
        // {"NiShader", NiType::NiShader},
        // {"NiMaterialInstance", NiType::NiMaterialInstance},
        // {"NiMeshArray", NiType::NiMeshArray},
        // {"NiShape", NiType::NiShape},
        // {"NiMesh2", NiType::NiMesh2},
        // {"NiShaderLightProperty", NiType::NiShaderLightProperty},
        // {"NiShaderMaterialProperty", NiType::NiShaderMaterialProperty},
        // {"NiShaderGeometryProperty", NiType::NiShaderGeometryProperty},
        // {"NiShaderTextureProperty", NiType::NiShaderTextureProperty},
        // {"NiArkStub2", NiType::NiArkStub2},
        // {"BSShaderTextureSet", NiType::BSShaderTextureSet},
        // {"BSLightingShaderProperty", NiType::BSLightingShaderProperty},
        // {"BSEffectShaderProperty", NiType::BSEffectShaderProperty},
        // {"BSEffectShaderPropertyFloatController", NiType::BSEffectShaderPropertyFloatController},
        // {"BSEffectShaderPropertyColorController", NiType::BSEffectShaderPropertyColorController},
        // {"BSLightingShaderPropertyColorController", NiType::BSLightingShaderPropertyColorController},
        // {"BSLightingShaderPropertyFloatController", NiType::BSLightingShaderPropertyFloatController},
        // {"BSLightingShaderPropertyUShortController", NiType::BSLightingShaderPropertyUShortController},
        // {"BSEffectShaderPropertyUShortController", NiType::BSEffectShaderPropertyUShortController},
        // {"BSEffectShaderPropertyBoolController", NiType::BSEffectShaderPropertyBoolController},
        // {"BSSkyShaderProperty", NiType::BSSkyShaderProperty},
        // {"BSImageSpaceShaderProperty", NiType::BSImageSpaceShaderProperty},
        // {"BSShaderPPLightingProperty", NiType::BSShaderPPLightingProperty},
        // {"BSWaterShaderProperty", NiType::BSWaterShaderProperty},
        // {"BSSkyShaderPropertyNode", NiType::BSSkyShaderPropertyNode},
        // {"BSShaderTextureArray", NiType::BSShaderTextureArray},
        // {"BSTextureArraySRGB", NiType::BSTextureArraySRGB},
        // {"BSSharedGroupNode", NiType::BSSharedGroupNode},
        // {"BSParentVelocityModifier", NiType::BSParentVelocityModifier},
        // {"BSPSysArrayEmitterTool", NiType::BSPSysArrayEmitterTool},
        // {"BSPSysLODModifier", NiType::BSPSysLODModifier},
        // {"BSValueNode", NiType::BSValueNode},
        // {"BSStripNode", NiType::BSStripNode},
        // {"BSMultiBound", NiType::BSMultiBound},
        // {"BSMultiBoundData", NiType::BSMultiBoundData},
        // {"BSMultiBoundNode", NiType::BSMultiBoundNode},
        // {"BSMultiBoundOBB", NiType::BSMultiBoundOBB},
        // {"BSReference", NiType::BSReference},
        // {"BSInvMarker", NiType::BSInvMarker},
        // {"BSLeafAnimNode", NiType::BSLeafAnimNode},
        // {"BSMasterParticleSystem", NiType::BSMasterParticleSystem},
        // {"BSPSysMultiTargetEmitterCtlr", NiType::BSPSysMultiTargetEmitterCtlr},
        // {"BSRefractionStrengthController", NiType::BSRefractionStrengthController},
        // {"BSTimerModifier", NiType::BSTimerModifier},
        // {"BSXFlags", NiType::BSXFlags},
        // {"BSFrustumFOVController", NiType::BSFrustumFOVController},
        // {"BSLagBoneController", NiType::BSLagBoneController},
        // {"BSNonUniformScaleExtraData", NiType::BSNonUniformScaleExtraData},
        // {"BSEffectShaderPropertyFloatController_Effect", NiType::BSEffectShaderPropertyFloatController_Effect},
        // {"BSEffectShaderPropertyColorController_Effect", NiType::BSEffectShaderPropertyColorController_Effect},
        // {"BSLightingShaderPropertyColorController_Lighting", NiType::BSLightingShaderPropertyColorController_Lighting},
        // {"BSLightingShaderPropertyFloatController_Lighting", NiType::BSLightingShaderPropertyFloatController_Lighting},
        // {"BSLightingShaderPropertyUShortController_Lighting", NiType::BSLightingShaderPropertyUShortController_Lighting},
        // {"BSAnimNotes", NiType::BSAnimNotes},
        // {"BSAnimationNode", NiType::BSAnimationNode},
        // {"BSParticleSystemManager", NiType::BSParticleSystemManager},
        // {"BSBound", NiType::BSBound},
        // {"BSFaceGenAnimationData", NiType::BSFaceGenAnimationData},
        // {"BSFaceGenBaseMorphExtraData", NiType::BSFaceGenBaseMorphExtraData},
        // {"BSFaceGenModelExtraData", NiType::BSFaceGenModelExtraData},
        // {"BSFaceGenMorphData", NiType::BSFaceGenMorphData},
        // {"BSFaceGenMorphDataHead", NiType::BSFaceGenMorphDataHead},
        // {"BSFaceGenMorphDataHair", NiType::BSFaceGenMorphDataHair},
        // {"BSFaceGenNiNode", NiType::BSFaceGenNiNode},
        // {"BSFaceGenKeyframeMultiple", NiType::BSFaceGenKeyframeMultiple},
        // {"BSAnisotropicProperty", NiType::BSAnisotropicProperty},
        // {"BSEffectShaderPropertyFloatController_Effect_FO4", NiType::BSEffectShaderPropertyFloatController_Effect_FO4},
        // {"BSLightingShaderPropertyColorController_Lighting_FO4", NiType::BSLightingShaderPropertyColorController_Lighting_FO4},
        // {"BSLightingShaderPropertyFloatController_Lighting_FO4", NiType::BSLightingShaderPropertyFloatController_Lighting_FO4},
        // {"BSLightingShaderPropertyUShortController_Lighting_FO4", NiType::BSLightingShaderPropertyUShortController_Lighting_FO4},
        // {"BSSkin__Instance", NiType::BSSkin__Instance},
        // {"BSSkin__BoneData", NiType::BSSkin__BoneData},
        // {"BSBoneMap", NiType::BSBoneMap},
        // {"BSDecalPlacementVectorExtraData", NiType::BSDecalPlacementVectorExtraData},
        // {"BSSegment", NiType::BSSegment},
        // {"BSSubIndexTriShape", NiType::BSSubIndexTriShape},
        // {"BSDynamicTriShape", NiType::BSDynamicTriShape},
        // {"BSMeshLODTriShape", NiType::BSMeshLODTriShape},
        // {"BSGeometrySubset", NiType::BSGeometrySubset},
        // {"BSNiAlphaPropertyTestRefController", NiType::BSNiAlphaPropertyTestRefController},
        // {"BSNiNode", NiType::BSNiNode},
        // {"BSFadeNode", NiType::BSFadeNode},
        // {"BSMultiBoundRoom", NiType::BSMultiBoundRoom},
        // {"BSMultiBoundPortal", NiType::BSMultiBoundPortal},
        // {"BSTreeNode", NiType::BSTreeNode},
        // {"BSLODShape", NiType::BSLODShape},
        // {"BSXipmap", NiType::BSXipmap},
        // {"BSWaterShaderPropertyColorController", NiType::BSWaterShaderPropertyColorController},
        // {"BSProceduralLightningController", NiType::BSProceduralLightningController},
        // {"BSWArray", NiType::BSWArray},
        // {"BSSceneGraphExtraData", NiType::BSSceneGraphExtraData},
        // {"BSBehaviorGraphExtraData", NiType::BSBehaviorGraphExtraData},
        // {"BSEyeCenterExtraData", NiType::BSEyeCenterExtraData},
        // {"BSMaterialTextureSwap", NiType::BSMaterialTextureSwap},
        // {"BSPackedCombinedSharedGeomDataExtra", NiType::BSPackedCombinedSharedGeomDataExtra},
        // {"BSWaterShaderPropertyRefractionPowerController", NiType::BSWaterShaderPropertyRefractionPowerController},
        // {"BSWaterShaderPropertyFresnelAmountController", NiType::BSWaterShaderPropertyFresnelAmountController},
        // {"BSWaterShaderPropertySunSpecularPowerController", NiType::BSWaterShaderPropertySunSpecularPowerController},
        // {"BSWaterShaderPropertySunSpecularColorController", NiType::BSWaterShaderPropertySunSpecularColorController},
        // {"BSWaterShaderPropertySunLightDirectionController", NiType::BSWaterShaderPropertySunLightDirectionController},
        // {"BSWaterShaderPropertyReflectivePowerController", NiType::BSWaterShaderPropertyReflectivePowerController},
        // {"BSWaterShaderPropertyFlowSpeedController", NiType::BSWaterShaderPropertyFlowSpeedController},
        // {"BSWaterShaderPropertyWashColorController", NiType::BSWaterShaderPropertyWashColorController},
        // {"BSWaterShaderPropertyRainSimulator", NiType::BSWaterShaderPropertyRainSimulator},
        // {"BSLightingShaderTextureSet", NiType::BSLightingShaderTextureSet},
        // {"BSEffectShaderProperty__SkyShaderProperty", NiType::BSEffectShaderProperty__SkyShaderProperty},
        // {"BSShaderProperty", NiType::BSShaderProperty},
        // {"BSEffectShaderProperty__BSShaderProperty", NiType::BSEffectShaderProperty__BSShaderProperty},
        // {"BSLightingShaderProperty__BSShaderProperty", NiType::BSLightingShaderProperty__BSShaderProperty},
        // {"BSSkyShaderProperty__BSShaderProperty", NiType::BSSkyShaderProperty__BSShaderProperty},
        // {"BSGrassShaderProperty", NiType::BSGrassShaderProperty},
        // {"BSShaderMaterial", NiType::BSShaderMaterial},
        // {"BSNiAlphaProperty", NiType::BSNiAlphaProperty},
        // {"BSTreadTransfControllers", NiType::BSTreadTransfControllers},
        // {"BSBloodSplatterShaderProperty", NiType::BSBloodSplatterShaderProperty},
        // {"BSEffectShaderPropertyRefController", NiType::BSEffectShaderPropertyRefController},
        // {"BSLightingShaderPropertyRefController", NiType::BSLightingShaderPropertyRefController},
        // {"BSTExtureSet", NiType::BSTExtureSet},
        // {"BSSubIndexLandTriShape", NiType::BSSubIndexLandTriShape},
        // {"BSSubIndexLandTriShape__BSGeometry", NiType::BSSubIndexLandTriShape__BSGeometry},
        // {"BSTriShape", NiType::BSTriShape},
        // {"BSDistantObjectInstanceNode", NiType::BSDistantObjectInstanceNode},
        // {"BSDistantObjectLandscapeCell", NiType::BSDistantObjectLandscapeCell},
        // {"BSDistantObjectNode", NiType::BSDistantObjectNode},
        // {"BSDistantObjectInstanceParent", NiType::BSDistantObjectInstanceParent},
        // {"BSTagCustomNamedObject", NiType::BSTagCustomNamedObject},
        // {"BSCombinedTriShape", NiType::BSCombinedTriShape},
        // {"BSOcclusionShape", NiType::BSOcclusionShape},
        // {"BSPSysScaleModifier", NiType::BSPSysScaleModifier},
        // {"BSPSysSimpleColorModifier", NiType::BSPSysSimpleColorModifier},
        // {"BSPSysShapeEmitter", NiType::BSPSysShapeEmitter},
        // {"BSPSysInheritVelocityModifier", NiType::BSPSysInheritVelocityModifier},
        // {"BSPSysHavokUpdateModifier", NiType::BSPSysHavokUpdateModifier},
        // {"BSPSysModifyGravityModifier", NiType::BSPSysModifyGravityModifier},
        // {"BSPSysSubTexModifier", NiType::BSPSysSubTexModifier},
        // {"BSLight", NiType::BSLight},
        // {"BSWaterShaderProperty__ImageSpaceShaderProperty", NiType::BSWaterShaderProperty__ImageSpaceShaderProperty},
        // {"BSLeafAnimNode__NiNode", NiType::BSLeafAnimNode__NiNode},
        // {"BSOrderedNode", NiType::BSOrderedNode},
        // {"BSAnimGroupSequence", NiType::BSAnimGroupSequence},
        // {"BSMultiIndexTriShape", NiType::BSMultiIndexTriShape},
        // {"BSMultiStreamInstanceTriShape", NiType::BSMultiStreamInstanceTriShape},
        // {"BSGeometry", NiType::BSGeometry},
        // {"BSShaderProperty__NiProperty", NiType::BSShaderProperty__NiProperty},
        // {"BSExternalEmittanceController", NiType::BSExternalEmittanceController},
        // {"BSNiParticleSystem", NiType::BSNiParticleSystem},
        // {"BSEffectShaderPropertyColorController_Effect", NiType::BSEffectShaderPropertyColorController_Effect},
        // {"BSTread", NiType::BSTread},
        // {"BSTransformMultipleNodes", NiType::BSTransformMultipleNodes},
        // {"BSTreadTransfController", NiType::BSTreadTransfController},
        // {"BSInvMarker__NiNode", NiType::BSInvMarker__NiNode},
        // {"BSProjectedDecal", NiType::BSProjectedDecal},
        // {"BSInstanceTriShape", NiType::BSInstanceTriShape},
        // {"BSInstanceTriShape_BSDynamicTriShape", NiType::BSInstanceTriShape_BSDynamicTriShape},
        // {"BSFadeNode__NiNode", NiType::BSFadeNode__NiNode},
        // {"BSSubIndexTriShape__NiTriShape", NiType::BSSubIndexTriShape__NiTriShape},
        // {"BSRangeLODData", NiType::BSRangeLODData},
        // {"BSRootNode", NiType::BSRootNode},
        // {"BSCompoundShape", NiType::BSCompoundShape},
        // {"BSOrderedTriShape", NiType::BSOrderedTriShape},
        // {"BSBloodSplatterShaderProperty__BSShaderProperty", NiType::BSBloodSplatterShaderProperty__BSShaderProperty},
        // {"BSCombinedTriShape__BSTriShape", NiType::BSCombinedTriShape__BSTriShape},
        // {"BSBatchRenderer2", NiType::BSBatchRenderer2},
        // {"BSShaderPPLightingProperty__BSShaderProperty", NiType::BSShaderPPLightingProperty__BSShaderProperty},
        // {"BSClothExtraData", NiType::BSClothExtraData},
        // {"BSShaderTextureSet__BSShaderProperty", NiType::BSShaderTextureSet__BSShaderProperty},
        // {"BSImageSpaceShaderProperty__BSShaderProperty", NiType::BSImageSpaceShaderProperty__BSShaderProperty},
        // {"BSDistantTreeShaderProperty", NiType::BSDistantTreeShaderProperty},
        // {"BSLightingShaderProperty__NiProperty", NiType::BSLightingShaderProperty__NiProperty},
        // {"BSEffectShaderProperty__NiProperty", NiType::BSEffectShaderProperty__NiProperty},
        // {"BSSkyShaderProperty__NiProperty", NiType::BSSkyShaderProperty__NiProperty},
        // {"BSLightingShaderProperty_FO4", NiType::BSLightingShaderProperty_FO4},
        // {"BSDistantObjectInstantiationData", NiType::BSDistantObjectInstantiationData},
        // {"BSDistantObjectInstanceChild", NiType::BSDistantObjectInstanceChild},
        // {"BSDistantObjectInstanceBurst", NiType::BSDistantObjectInstanceBurst},
        // {"BSDistantObjectInstanceBlock", NiType::BSDistantObjectInstanceBlock},
        // {"BSDistantObjectInstanceDecoration", NiType::BSDistantObjectInstanceDecoration},
        // {"BSDistantObjectInstancedNode", NiType::BSDistantObjectInstancedNode},
        // {"BSCollisionQueryProxyExtraData", NiType::BSCollisionQueryProxyExtraData},
        // {"CsNiNode", NiType::CsNiNode},
        // {"NiYAMaterialProperty", NiType::NiYAMaterialProperty},
        // {"NiRimLightProperty", NiType::NiRimLightProperty},
        // {"NiProgramLODData", NiType::NiProgramLODData},
        // {"MdlMan__CDataEntry", NiType::MdlMan__CDataEntry},
        // {"MdlMan__CModelTemplateDataEntry", NiType::MdlMan__CModelTemplateDataEntry},
        // {"MdlMan__CAMDataEntry", NiType::MdlMan__CAMDataEntry},
        // {"MdlMan__CMeshDataEntry", NiType::MdlMan__CMeshDataEntry},
        // {"MdlMan__CSkeletonDataEntry", NiType::MdlMan__CSkeletonDataEntry},
        // {"MdlMan__CAnimationDataEntry", NiType::MdlMan__CAnimationDataEntry},
        {"Unknown", NiType::Unknown}
    };

    auto it = map.find(str);
    return it != map.end() ? it->second : NiType::Unknown;
}


