/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2026 James Packer

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License version 2 as
     published by the Free Software Foundation */

#ifndef BC_GRAPHICS_WICKED_SCENEMANAGER_HPP
#define BC_GRAPHICS_WICKED_SCENEMANAGER_HPP

#ifdef WITH_WICKED_ENGINE

#include "../ISceneManager.hpp"
#include "WickedSceneNode.hpp"
#include "WickedModelImporter.hpp"
#include <vector>
#include <memory>

namespace bc { namespace graphics { namespace wicked {

// Wicked Engine implementation of ISceneManager.
// Maps Bridge Command's scene graph model to WE's Entity Component System.
class WickedSceneManager : public bc::graphics::ISceneManager {
public:
    explicit WickedSceneManager(wi::scene::Scene* scene)
        : weScene(scene) {
        // Create a root entity for the scene graph hierarchy
        rootEntity = wi::ecs::CreateEntity();
        weScene->transforms.Create(rootEntity);
        weScene->names.Create(rootEntity) = "BC_Root";
        rootNode = std::make_unique<WickedSceneNode>(rootEntity, weScene);
    }

    ~WickedSceneManager() override = default;

    ISceneNode* getRootSceneNode() override {
        return rootNode.get();
    }

    IMeshNode* addMeshSceneNode(MeshHandle mesh, ISceneNode* parent,
                                 int id, const Vec3& position,
                                 const Vec3& rotation, const Vec3& scale) override {
        // In WE, we load models via Scene::Entity_CreateObject
        // mesh is expected to be an entity ID from getMesh()
        wi::ecs::Entity meshEntity = (wi::ecs::Entity)(uintptr_t)mesh;

        // Create an object that references this mesh
        wi::ecs::Entity objectEntity = weScene->Entity_CreateObject("BC_Object");
        auto* object = weScene->objects.GetComponent(objectEntity);
        if (object) {
            object->meshID = meshEntity;
        }

        // Set transform
        auto* transform = weScene->transforms.GetComponent(objectEntity);
        if (transform) {
            float degToRad = 3.14159265358979f / 180.0f;
            transform->Translate(toXM(position));
            transform->RotateRollPitchYaw({rotation.x * degToRad, rotation.y * degToRad, rotation.z * degToRad});
            transform->Scale(toXM(scale));
            transform->UpdateTransform();
        }

        // Parent to scene graph
        if (parent) {
            auto* wp = dynamic_cast<WickedSceneNode*>(parent);
            if (wp) weScene->Component_Attach(objectEntity, wp->getEntity());
        }

        auto node = std::make_unique<WickedMeshNode>(objectEntity, meshEntity, weScene);
        auto* ptr = node.get();
        ownedNodes.push_back(std::move(node));
        return ptr;
    }

    IAnimatedMeshNode* addAnimatedMeshSceneNode(MeshHandle mesh, ISceneNode* parent,
                                                  int id, const Vec3& position,
                                                  const Vec3& rotation, const Vec3& scale) override {
        // WE handles animation via AnimationComponent on the same entity
        wi::ecs::Entity meshEntity = (wi::ecs::Entity)(uintptr_t)mesh;

        wi::ecs::Entity objectEntity = weScene->Entity_CreateObject("BC_AnimObject");
        auto* object = weScene->objects.GetComponent(objectEntity);
        if (object) {
            object->meshID = meshEntity;
        }

        auto* transform = weScene->transforms.GetComponent(objectEntity);
        if (transform) {
            float degToRad = 3.14159265358979f / 180.0f;
            transform->Translate(toXM(position));
            transform->RotateRollPitchYaw({rotation.x * degToRad, rotation.y * degToRad, rotation.z * degToRad});
            transform->Scale(toXM(scale));
            transform->UpdateTransform();
        }

        if (parent) {
            auto* wp = dynamic_cast<WickedSceneNode*>(parent);
            if (wp) weScene->Component_Attach(objectEntity, wp->getEntity());
        }

        // Find an animation component associated with the loaded model
        // WE stores animations at the scene level, not per-object
        wi::ecs::Entity animEntity = wi::ecs::INVALID_ENTITY;
        for (size_t i = 0; i < weScene->animations.GetCount(); ++i) {
            animEntity = weScene->animations.GetEntity(i);
            break; // Use first animation found
        }

        auto node = std::make_unique<WickedAnimatedMeshNode>(
            objectEntity, meshEntity, animEntity, weScene);
        node->setID(id);
        auto* ptr = node.get();
        ownedNodes.push_back(std::move(node));
        return ptr;
    }

    ICameraNode* addCameraSceneNode(ISceneNode* parent,
                                     const Vec3& position,
                                     const Vec3& lookAt,
                                     int id) override {
        wi::ecs::Entity camEntity = weScene->Entity_CreateCamera(
            "BC_Camera", 1920, 1080, 0.1f, 5000.0f);

        auto* cam = weScene->cameras.GetComponent(camEntity);
        if (cam) {
            cam->Eye = toXM(position);
            cam->At = toXM(lookAt);
            cam->Up = {0, 1, 0};
            cam->SetDirty();
        }

        if (parent) {
            auto* wp = dynamic_cast<WickedSceneNode*>(parent);
            if (wp) weScene->Component_Attach(camEntity, wp->getEntity());
        }

        auto node = std::make_unique<WickedCameraNode>(camEntity, weScene);
        node->setID(id);
        auto* ptr = node.get();
        ownedNodes.push_back(std::move(node));
        return ptr;
    }

    ILightNode* addLightSceneNode(ISceneNode* parent,
                                   const Vec3& position,
                                   Colorf color, float radius,
                                   int id) override {
        wi::ecs::Entity lightEntity = weScene->Entity_CreateLight(
            "BC_Light",
            toXM(position),
            {color.r, color.g, color.b},
            1.0f,   // intensity
            radius
        );

        if (parent) {
            auto* wp = dynamic_cast<WickedSceneNode*>(parent);
            if (wp) weScene->Component_Attach(lightEntity, wp->getEntity());
        }

        auto node = std::make_unique<WickedLightNode>(lightEntity, weScene);
        node->setID(id);
        auto* ptr = node.get();
        ownedNodes.push_back(std::move(node));
        return ptr;
    }

    ISceneNode* addBillboardSceneNode(ISceneNode* parent,
                                       const Vec2& size,
                                       const Vec3& position,
                                       int id) override {
        // WE doesn't have a direct billboard node.
        // Create an empty entity with transform for now -- nav lights
        // will be implemented with point lights + lens flare in Phase 3.
        wi::ecs::Entity entity = wi::ecs::CreateEntity();
        weScene->transforms.Create(entity);
        weScene->names.Create(entity) = "BC_Billboard";

        auto* transform = weScene->transforms.GetComponent(entity);
        if (transform) {
            transform->Translate(toXM(position));
            transform->UpdateTransform();
        }

        if (parent) {
            auto* wp = dynamic_cast<WickedSceneNode*>(parent);
            if (wp) weScene->Component_Attach(entity, wp->getEntity());
        }

        auto node = std::make_unique<WickedSceneNode>(entity, weScene);
        node->setID(id);
        auto* ptr = node.get();
        ownedNodes.push_back(std::move(node));
        return ptr;
    }

    ISceneNode* addSkyBoxSceneNode(TextureHandle top, TextureHandle bottom,
                                    TextureHandle left, TextureHandle right,
                                    TextureHandle front, TextureHandle back,
                                    ISceneNode* parent, int id) override {
        // WE uses WeatherComponent for sky/atmosphere -- no separate node needed.
        // Return a dummy entity so callers have a valid node to work with.
        wi::ecs::Entity entity = wi::ecs::CreateEntity();
        weScene->transforms.Create(entity);
        weScene->names.Create(entity) = "BC_SkyBox";

        auto node = std::make_unique<WickedSceneNode>(entity, weScene);
        node->setID(id);
        auto* ptr = node.get();
        ownedNodes.push_back(std::move(node));
        return ptr;
    }

    ISceneNode* addTerrainSceneNode(const std::string& heightmapFile,
                                     ISceneNode* parent, int id,
                                     const Vec3& position,
                                     const Vec3& rotation,
                                     const Vec3& scale) override {
        // WE has terrain support via wi::terrain system.
        // Create placeholder entity -- full terrain loading in Phase 2B-07.
        wi::ecs::Entity entity = wi::ecs::CreateEntity();
        weScene->transforms.Create(entity);
        weScene->names.Create(entity) = "BC_Terrain";

        auto* transform = weScene->transforms.GetComponent(entity);
        if (transform) {
            float degToRad = 3.14159265358979f / 180.0f;
            transform->Translate(toXM(position));
            transform->RotateRollPitchYaw({rotation.x * degToRad, rotation.y * degToRad, rotation.z * degToRad});
            transform->Scale(toXM(scale));
            transform->UpdateTransform();
        }

        if (parent) {
            auto* wp = dynamic_cast<WickedSceneNode*>(parent);
            if (wp) weScene->Component_Attach(entity, wp->getEntity());
        }

        auto node = std::make_unique<WickedSceneNode>(entity, weScene);
        node->setID(id);
        auto* ptr = node.get();
        ownedNodes.push_back(std::move(node));
        return ptr;
    }

    MeshHandle getMesh(const std::string& filename) override {
        // Load model file via BC's Wicked Engine importer
        // Supports .obj (TinyObjLoader), .wiscene (native), .gltf/.glb
        wi::ecs::Entity rootEntity = LoadModelFromFile(filename, *weScene);
        return (MeshHandle)(uintptr_t)rootEntity;
    }

    ISceneNode* getSceneNodeFromId(int id, ISceneNode* start) override {
        // Search owned nodes by their assigned BC ID
        for (auto& node : ownedNodes) {
            if (node && node->getID() == id) {
                return node.get();
            }
        }
        return nullptr;
    }

    void drawAll() override {
        // In WE, rendering is handled by the Application::Run() loop
        // which calls RenderPath3D internally.
        // This is a no-op - WE renders automatically.
    }

    void* getNativeSceneManager() const override {
        return weScene;
    }

private:
    wi::scene::Scene* weScene;
    wi::ecs::Entity rootEntity = wi::ecs::INVALID_ENTITY;
    std::unique_ptr<WickedSceneNode> rootNode;
    std::vector<std::unique_ptr<ISceneNode>> ownedNodes;
};

}}} // namespace bc::graphics::wicked

#endif // WITH_WICKED_ENGINE
#endif // BC_GRAPHICS_WICKED_SCENEMANAGER_HPP
