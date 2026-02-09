/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2024 James Packer

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License version 2 as
     published by the Free Software Foundation */

#ifndef BC_GRAPHICS_IRRLICHT_SCENEMANAGER_HPP
#define BC_GRAPHICS_IRRLICHT_SCENEMANAGER_HPP

#include "../ISceneManager.hpp"
#include "IrrlichtSceneNode.hpp"
#include "IrrlichtMeshNode.hpp"
#include "IrrlichtCameraNode.hpp"
#include "IrrlichtLightNode.hpp"
#include <memory>
#include <vector>

namespace bc { namespace graphics { namespace irrlicht {

// Wraps irr::scene::ISceneManager
class IrrlichtSceneManager : public bc::graphics::ISceneManager {
public:
    explicit IrrlichtSceneManager(irr::scene::ISceneManager* smgr) : irrSmgr(smgr) {}
    ~IrrlichtSceneManager() override = default;

    bc::graphics::ISceneNode* getRootSceneNode() override {
        if (!rootNode) {
            rootNode = std::make_unique<IrrlichtSceneNode>(irrSmgr->getRootSceneNode());
        }
        return rootNode.get();
    }

    bc::graphics::IMeshNode* addMeshSceneNode(MeshHandle mesh,
                                               bc::graphics::ISceneNode* parent,
                                               int id,
                                               const Vec3& position,
                                               const Vec3& rotation,
                                               const Vec3& scale) override {
        irr::scene::ISceneNode* p = parent
            ? static_cast<irr::scene::ISceneNode*>(parent->getNativeNode())
            : nullptr;
        auto* node = irrSmgr->addMeshSceneNode(
            static_cast<irr::scene::IMesh*>(mesh), p, id,
            toIrr(position), toIrr(rotation), toIrr(scale));
        if (!node) return nullptr;
        auto wrapper = std::make_unique<IrrlichtMeshNode>(node);
        auto* ptr = wrapper.get();
        ownedNodes.push_back(std::move(wrapper));
        return ptr;
    }

    bc::graphics::IAnimatedMeshNode* addAnimatedMeshSceneNode(MeshHandle mesh,
                                                               bc::graphics::ISceneNode* parent,
                                                               int id,
                                                               const Vec3& position,
                                                               const Vec3& rotation,
                                                               const Vec3& scale) override {
        irr::scene::ISceneNode* p = parent
            ? static_cast<irr::scene::ISceneNode*>(parent->getNativeNode())
            : nullptr;
        auto* node = irrSmgr->addAnimatedMeshSceneNode(
            static_cast<irr::scene::IAnimatedMesh*>(mesh), p, id,
            toIrr(position), toIrr(rotation), toIrr(scale));
        if (!node) return nullptr;
        auto wrapper = std::make_unique<IrrlichtAnimatedMeshNode>(node);
        auto* ptr = wrapper.get();
        ownedNodes.push_back(std::move(wrapper));
        return ptr;
    }

    bc::graphics::ICameraNode* addCameraSceneNode(bc::graphics::ISceneNode* parent,
                                                    const Vec3& position,
                                                    const Vec3& lookAt,
                                                    int id) override {
        irr::scene::ISceneNode* p = parent
            ? static_cast<irr::scene::ISceneNode*>(parent->getNativeNode())
            : nullptr;
        auto* node = irrSmgr->addCameraSceneNode(p, toIrr(position), toIrr(lookAt), id);
        if (!node) return nullptr;
        auto wrapper = std::make_unique<IrrlichtCameraNode>(node);
        auto* ptr = wrapper.get();
        ownedNodes.push_back(std::move(wrapper));
        return ptr;
    }

    bc::graphics::ILightNode* addLightSceneNode(bc::graphics::ISceneNode* parent,
                                                  const Vec3& position,
                                                  Colorf color,
                                                  float radius,
                                                  int id) override {
        irr::scene::ISceneNode* p = parent
            ? static_cast<irr::scene::ISceneNode*>(parent->getNativeNode())
            : nullptr;
        auto* node = irrSmgr->addLightSceneNode(p, toIrr(position), toIrrColorf(color), radius, id);
        if (!node) return nullptr;
        auto wrapper = std::make_unique<IrrlichtLightNode>(node);
        auto* ptr = wrapper.get();
        ownedNodes.push_back(std::move(wrapper));
        return ptr;
    }

    bc::graphics::ISceneNode* addBillboardSceneNode(bc::graphics::ISceneNode* parent,
                                                     const Vec2& size,
                                                     const Vec3& position,
                                                     int id) override {
        irr::scene::ISceneNode* p = parent
            ? static_cast<irr::scene::ISceneNode*>(parent->getNativeNode())
            : nullptr;
        auto* node = irrSmgr->addBillboardSceneNode(p,
            irr::core::dimension2d<irr::f32>(size.x, size.y),
            toIrr(position), id);
        if (!node) return nullptr;
        auto wrapper = std::make_unique<IrrlichtSceneNode>(node);
        auto* ptr = wrapper.get();
        ownedNodes.push_back(std::move(wrapper));
        return ptr;
    }

    bc::graphics::ISceneNode* addSkyBoxSceneNode(TextureHandle top, TextureHandle bottom,
                                                  TextureHandle left, TextureHandle right,
                                                  TextureHandle front, TextureHandle back,
                                                  bc::graphics::ISceneNode* parent,
                                                  int id) override {
        irr::scene::ISceneNode* p = parent
            ? static_cast<irr::scene::ISceneNode*>(parent->getNativeNode())
            : nullptr;
        auto* node = irrSmgr->addSkyBoxSceneNode(
            static_cast<irr::video::ITexture*>(top),
            static_cast<irr::video::ITexture*>(bottom),
            static_cast<irr::video::ITexture*>(left),
            static_cast<irr::video::ITexture*>(right),
            static_cast<irr::video::ITexture*>(front),
            static_cast<irr::video::ITexture*>(back),
            p, id);
        if (!node) return nullptr;
        auto wrapper = std::make_unique<IrrlichtSceneNode>(node);
        auto* ptr = wrapper.get();
        ownedNodes.push_back(std::move(wrapper));
        return ptr;
    }

    bc::graphics::ISceneNode* addTerrainSceneNode(const std::string& heightmapFile,
                                                   bc::graphics::ISceneNode* parent,
                                                   int id,
                                                   const Vec3& position,
                                                   const Vec3& rotation,
                                                   const Vec3& scale) override {
        irr::scene::ISceneNode* p = parent
            ? static_cast<irr::scene::ISceneNode*>(parent->getNativeNode())
            : nullptr;
        auto* node = irrSmgr->addTerrainSceneNode(
            heightmapFile.c_str(), p, id,
            toIrr(position), toIrr(rotation), toIrr(scale));
        if (!node) return nullptr;
        auto wrapper = std::make_unique<IrrlichtSceneNode>(node);
        auto* ptr = wrapper.get();
        ownedNodes.push_back(std::move(wrapper));
        return ptr;
    }

    MeshHandle getMesh(const std::string& filename) override {
        return irrSmgr->getMesh(filename.c_str());
    }

    bc::graphics::ISceneNode* getSceneNodeFromId(int id, bc::graphics::ISceneNode* start) override {
        irr::scene::ISceneNode* startNode = start
            ? static_cast<irr::scene::ISceneNode*>(start->getNativeNode())
            : nullptr;
        auto* node = irrSmgr->getSceneNodeFromId(id, startNode);
        if (!node) return nullptr;
        auto wrapper = std::make_unique<IrrlichtSceneNode>(node);
        auto* ptr = wrapper.get();
        ownedNodes.push_back(std::move(wrapper));
        return ptr;
    }

    void drawAll() override { irrSmgr->drawAll(); }

    void* getNativeSceneManager() const override { return irrSmgr; }

private:
    irr::scene::ISceneManager* irrSmgr;
    std::unique_ptr<IrrlichtSceneNode> rootNode;
    // Owns wrapper objects (the underlying Irrlicht nodes are owned by Irrlicht)
    std::vector<std::unique_ptr<bc::graphics::ISceneNode>> ownedNodes;
};

}}} // namespace bc::graphics::irrlicht

#endif // BC_GRAPHICS_IRRLICHT_SCENEMANAGER_HPP
