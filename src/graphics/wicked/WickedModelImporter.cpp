/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2026 James Packer

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License version 2 as
     published by the Free Software Foundation

     OBJ import adapted from Wicked Engine Editor (MIT license)
     Original: Copyright (c) Turanszki Janos */

#ifdef WITH_WICKED_ENGINE

#include "WickedModelImporter.hpp"

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

#include <fstream>
#include <iostream>
#include <algorithm>
#include <unordered_map>

using namespace wi::graphics;
using namespace wi::scene;
using namespace wi::ecs;

namespace bc { namespace graphics { namespace wicked {

// Helper: get directory from path
static std::string getDirectory(const std::string& path) {
    size_t pos = path.find_last_of("/\\");
    if (pos != std::string::npos) return path.substr(0, pos + 1);
    return "";
}

// Helper: get filename from path
static std::string getFilename(const std::string& path) {
    size_t pos = path.find_last_of("/\\");
    if (pos != std::string::npos) return path.substr(pos + 1);
    return path;
}

// Helper: get file extension (lowercase)
static std::string getExtension(const std::string& path) {
    size_t pos = path.find_last_of('.');
    if (pos != std::string::npos) {
        std::string ext = path.substr(pos);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        return ext;
    }
    return "";
}

// Read a file into a byte vector
static bool readFile(const std::string& path, std::vector<uint8_t>& data) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return false;
    auto size = file.tellg();
    if (size <= 0) return false;
    data.resize(static_cast<size_t>(size));
    file.seekg(0);
    file.read(reinterpret_cast<char*>(data.data()), size);
    return true;
}

// membuf for streaming file data
struct membuf : std::streambuf {
    membuf(char* begin, char* end) {
        this->setg(begin, begin, end);
    }
};

// Custom material file reader for TinyObjLoader
class MaterialFileReader : public tinyobj::MaterialReader {
public:
    explicit MaterialFileReader(const std::string& mtlDir) : dir_(mtlDir) {}
    ~MaterialFileReader() override = default;

    bool operator()(const std::string& matId,
                    std::vector<tinyobj::material_t>* materials,
                    std::map<std::string, int>* matMap,
                    std::string* err) override {
        std::string filepath = dir_.empty() ? matId : dir_ + matId;
        std::vector<uint8_t> filedata;
        if (!readFile(filepath, filedata)) {
            if (err) *err += "WARN: Material file [ " + filepath + " ] not found.\n";
            return false;
        }
        membuf sbuf(reinterpret_cast<char*>(filedata.data()),
                     reinterpret_cast<char*>(filedata.data() + filedata.size()));
        std::istream in(&sbuf);
        std::string warning;
        LoadMtl(matMap, materials, &in, &warning);
        if (!warning.empty() && err) *err += warning;
        return true;
    }

private:
    std::string dir_;
};

// Hash for unique vertex deduplication
struct IndexAndMat {
    tinyobj::index_t index;
    int materialIndex;
    bool operator==(const IndexAndMat& other) const {
        return index.vertex_index == other.index.vertex_index &&
               index.normal_index == other.index.normal_index &&
               index.texcoord_index == other.index.texcoord_index &&
               materialIndex == other.materialIndex;
    }
};

struct IndexAndMatHash {
    size_t operator()(const IndexAndMat& o) const noexcept {
        size_t h = 0;
        h ^= std::hash<int>()(o.index.vertex_index) + 0x9e3779b9 + (h << 6) + (h >> 2);
        h ^= std::hash<int>()(o.index.normal_index) + 0x9e3779b9 + (h << 6) + (h >> 2);
        h ^= std::hash<int>()(o.index.texcoord_index) + 0x9e3779b9 + (h << 6) + (h >> 2);
        h ^= std::hash<int>()(o.materialIndex) + 0x9e3779b9 + (h << 6) + (h >> 2);
        return h;
    }
};

void ImportModel_OBJ(const std::string& filename, Scene& scene,
                      Entity rootEntity) {
    std::string directory = getDirectory(filename);
    std::string name = getFilename(filename);

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string errors;

    std::vector<uint8_t> filedata;
    bool success = readFile(filename, filedata);

    if (success) {
        membuf sbuf(reinterpret_cast<char*>(filedata.data()),
                     reinterpret_cast<char*>(filedata.data() + filedata.size()));
        std::istream in(&sbuf);
        MaterialFileReader matReader(directory);
        success = tinyobj::LoadObj(&attrib, &shapes, &materials, &errors, &in, &matReader, true);
    } else {
        errors = "Failed to read file: " + filename;
    }

    if (!errors.empty()) {
        std::cerr << "WickedModelImporter OBJ: " << errors << std::endl;
    }

    if (!success) return;

    // Load material library
    std::vector<Entity> materialLibrary;
    for (auto& mat : materials) {
        Entity matEntity = scene.Entity_CreateMaterial(mat.name);
        scene.Component_Attach(matEntity, rootEntity);
        MaterialComponent& material = *scene.materials.GetComponent(matEntity);

        material.baseColor = DirectX::XMFLOAT4(mat.diffuse[0], mat.diffuse[1], mat.diffuse[2], 1.0f);
        material.textures[MaterialComponent::BASECOLORMAP].name = mat.diffuse_texname;
        material.textures[MaterialComponent::DISPLACEMENTMAP].name = mat.displacement_texname;
        material.emissiveColor.x = mat.emission[0];
        material.emissiveColor.y = mat.emission[1];
        material.emissiveColor.z = mat.emission[2];
        material.emissiveColor.w = std::max(mat.emission[0], std::max(mat.emission[1], mat.emission[2]));
        material.metalness = mat.metallic;
        material.textures[MaterialComponent::NORMALMAP].name = mat.normal_texname;
        material.textures[MaterialComponent::SURFACEMAP].name = mat.specular_texname;
        material.roughness = mat.roughness;

        if (material.textures[MaterialComponent::NORMALMAP].name.empty())
            material.textures[MaterialComponent::NORMALMAP].name = mat.bump_texname;
        if (material.textures[MaterialComponent::SURFACEMAP].name.empty())
            material.textures[MaterialComponent::SURFACEMAP].name = mat.specular_highlight_texname;

        // Resolve texture paths relative to model directory
        for (auto& tex : material.textures) {
            if (!tex.name.empty()) {
                tex.name = directory + tex.name;
            }
        }

        material.CreateRenderData();
        materialLibrary.push_back(matEntity);
    }

    // Create default material if none found
    if (materialLibrary.empty()) {
        Entity matEntity = scene.Entity_CreateMaterial("BC_DefaultMaterial");
        scene.Component_Attach(matEntity, rootEntity);
        materialLibrary.push_back(matEntity);
    }

    // Load shapes (objects/meshes)
    for (auto& shape : shapes) {
        Entity objectEntity = scene.Entity_CreateObject(shape.name);
        scene.Component_Attach(objectEntity, rootEntity);
        Entity meshEntity = scene.Entity_CreateMesh(shape.name + "_mesh");
        scene.Component_Attach(meshEntity, rootEntity);
        ObjectComponent& object = *scene.objects.GetComponent(objectEntity);
        MeshComponent& mesh = *scene.meshes.GetComponent(meshEntity);

        object.meshID = meshEntity;

        std::unordered_map<int, int> registeredMaterials;
        std::unordered_map<IndexAndMat, uint32_t, IndexAndMatHash> uniqueVertices;

        for (size_t i = 0; i < shape.mesh.indices.size(); i += 3) {
            tinyobj::index_t indices[3] = {
                shape.mesh.indices[i + 0],
                shape.mesh.indices[i + 1],
                shape.mesh.indices[i + 2],
            };

            for (auto& index : indices) {
                DirectX::XMFLOAT3 pos(
                    attrib.vertices[index.vertex_index * 3 + 0],
                    attrib.vertices[index.vertex_index * 3 + 1],
                    attrib.vertices[index.vertex_index * 3 + 2]
                );

                DirectX::XMFLOAT3 nor(0, 0, 0);
                if (!attrib.normals.empty() && index.normal_index >= 0) {
                    nor = DirectX::XMFLOAT3(
                        attrib.normals[index.normal_index * 3 + 0],
                        attrib.normals[index.normal_index * 3 + 1],
                        attrib.normals[index.normal_index * 3 + 2]
                    );
                }

                DirectX::XMFLOAT2 tex(0, 0);
                if (index.texcoord_index >= 0 && !attrib.texcoords.empty()) {
                    tex = DirectX::XMFLOAT2(
                        attrib.texcoords[index.texcoord_index * 2 + 0],
                        1.0f - attrib.texcoords[index.texcoord_index * 2 + 1]
                    );
                }

                int matIdx = std::max(0, shape.mesh.material_ids[i / 3]);
                if (registeredMaterials.count(matIdx) == 0) {
                    registeredMaterials[matIdx] = static_cast<int>(mesh.subsets.size());
                    mesh.subsets.push_back(MeshComponent::MeshSubset());
                    mesh.subsets.back().materialID = materialLibrary[std::min(matIdx, (int)materialLibrary.size() - 1)];
                    mesh.subsets.back().indexOffset = static_cast<uint32_t>(mesh.indices.size());
                }

                // OBJ is right-handed, WE is left-handed: flip Z
                pos.z *= -1;
                nor.z *= -1;

                IndexAndMat im = {index, matIdx};
                if (uniqueVertices.count(im) == 0) {
                    uniqueVertices[im] = static_cast<uint32_t>(mesh.vertex_positions.size());
                    mesh.vertex_positions.push_back(pos);
                    mesh.vertex_normals.push_back(nor);
                    mesh.vertex_uvset_0.push_back(tex);
                }
                mesh.indices.push_back(uniqueVertices[im]);
                mesh.subsets.back().indexCount++;
            }
        }
        mesh.CreateRenderData();
    }
}

wi::ecs::Entity LoadModelFromFile(const std::string& filename, Scene& scene) {
    std::string ext = getExtension(filename);

    // Create root entity for the model
    Entity rootEntity = CreateEntity();
    scene.transforms.Create(rootEntity);
    scene.names.Create(rootEntity) = getFilename(filename);

    if (ext == ".obj") {
        ImportModel_OBJ(filename, scene, rootEntity);
    } else if (ext == ".wiscene") {
        // Native WE format -- load via WE's built-in loader
        // LoadModel merges into the global scene; for our purposes, use LoadModel2
        scene.Entity_Remove(rootEntity); // Remove our placeholder
        rootEntity = wi::scene::LoadModel(filename);
    } else if (ext == ".gltf" || ext == ".glb") {
        // GLTF support requires WE Editor importer -- not yet integrated
        // For now, try WE's LoadModel which may handle GLTF in some versions
        scene.Entity_Remove(rootEntity);
        rootEntity = wi::scene::LoadModel(filename);
    } else if (ext == ".x" || ext == ".3ds") {
        // DirectX .x and .3ds formats are not supported by Wicked Engine.
        // These models need to be converted to .obj or .gltf first.
        // Use the conversion utility: tools/convert_models.py (Phase 2B-05)
        std::cerr << "WickedModelImporter: Unsupported format '" << ext
                  << "' for file: " << filename << std::endl;
        std::cerr << "  Convert to .obj or .gltf first." << std::endl;
        // Return root with no geometry so scene graph isn't broken
    } else {
        std::cerr << "WickedModelImporter: Unknown format: " << ext << std::endl;
    }

    return rootEntity;
}

}}} // namespace bc::graphics::wicked

#endif // WITH_WICKED_ENGINE
