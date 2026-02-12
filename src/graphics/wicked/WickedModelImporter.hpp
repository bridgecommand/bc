/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2026 James Packer

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License version 2 as
     published by the Free Software Foundation */

#ifndef BC_GRAPHICS_WICKED_MODELIMPORTER_HPP
#define BC_GRAPHICS_WICKED_MODELIMPORTER_HPP

#ifdef WITH_WICKED_ENGINE

#include "WickedEngine.h"
#include <string>

namespace bc { namespace graphics { namespace wicked {

// Load a model file into a Wicked Engine scene.
// Supports: .obj (via TinyObjLoader), .gltf/.glb (via WE), .wiscene (native).
// Returns the root entity of the loaded model, or INVALID_ENTITY on failure.
wi::ecs::Entity LoadModelFromFile(const std::string& filename, wi::scene::Scene& scene);

// Import an OBJ file into the scene. Adapted from WE Editor's ModelImporter_OBJ.
void ImportModel_OBJ(const std::string& filename, wi::scene::Scene& scene,
                      wi::ecs::Entity rootEntity);

}}} // namespace bc::graphics::wicked

#endif // WITH_WICKED_ENGINE
#endif // BC_GRAPHICS_WICKED_MODELIMPORTER_HPP
