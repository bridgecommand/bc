/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2024 James Packer

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License version 2 as
     published by the Free Software Foundation */

#ifndef BC_GRAPHICS_IRRLICHT_MESHNODE_HPP
#define BC_GRAPHICS_IRRLICHT_MESHNODE_HPP

#include "../IMeshNode.hpp"
#include "IrrlichtSceneNode.hpp"

namespace bc { namespace graphics { namespace irrlicht {

// Wraps irr::scene::IMeshSceneNode
class IrrlichtMeshNode : public bc::graphics::IMeshNode {
public:
    explicit IrrlichtMeshNode(irr::scene::IMeshSceneNode* node) : irrNode(node) {}
    ~IrrlichtMeshNode() override = default;

    // ISceneNode interface
    void setPosition(const Vec3& pos) override { irrNode->setPosition(toIrr(pos)); }
    Vec3 getPosition() const override { return fromIrr(irrNode->getPosition()); }
    Vec3 getAbsolutePosition() const override { return fromIrr(irrNode->getAbsolutePosition()); }
    void setRotation(const Vec3& rot) override { irrNode->setRotation(toIrr(rot)); }
    Vec3 getRotation() const override { return fromIrr(irrNode->getRotation()); }
    void setScale(const Vec3& s) override { irrNode->setScale(toIrr(s)); }
    Vec3 getScale() const override { return fromIrr(irrNode->getScale()); }
    void setVisible(bool vis) override { irrNode->setVisible(vis); }
    bool isVisible() const override { return irrNode->isVisible(); }
    void setParent(bc::graphics::ISceneNode* parent) override {
        if (parent) irrNode->setParent(static_cast<irr::scene::ISceneNode*>(parent->getNativeNode()));
    }
    bc::graphics::ISceneNode* getParent() const override { return nullptr; }
    void remove() override { irrNode->remove(); }
    void setMaterialFlag(MaterialFlag flag, bool value) override { irrNode->setMaterialFlag(toIrrFlag(flag), value); }
    void setMaterialType(MaterialType type) override { irrNode->setMaterialType(toIrrMaterialType(type)); }
    void setMaterialTexture(int layer, TextureHandle tex) override {
        irrNode->setMaterialTexture(layer, static_cast<irr::video::ITexture*>(tex));
    }
    void setID(int id) override { irrNode->setID(id); }
    int getID() const override { return irrNode->getID(); }
    void* getNativeNode() const override { return irrNode; }

    // IMeshNode interface
    MeshHandle getMesh() const override { return irrNode->getMesh(); }
    int getMaterialCount() const override { return irrNode->getMaterialCount(); }
    void setMaterialFlagForAll(MaterialFlag flag, bool value) override {
        for (irr::u32 i = 0; i < irrNode->getMaterialCount(); i++)
            irrNode->getMaterial(i).setFlag(toIrrFlag(flag), value);
    }
    void setMaterialTypeForAll(MaterialType type) override {
        for (irr::u32 i = 0; i < irrNode->getMaterialCount(); i++)
            irrNode->getMaterial(i).MaterialType = toIrrMaterialType(type);
    }

private:
    irr::scene::IMeshSceneNode* irrNode;
};

// Wraps irr::scene::IAnimatedMeshSceneNode
class IrrlichtAnimatedMeshNode : public bc::graphics::IAnimatedMeshNode {
public:
    explicit IrrlichtAnimatedMeshNode(irr::scene::IAnimatedMeshSceneNode* node) : irrNode(node) {}
    ~IrrlichtAnimatedMeshNode() override = default;

    // ISceneNode interface
    void setPosition(const Vec3& pos) override { irrNode->setPosition(toIrr(pos)); }
    Vec3 getPosition() const override { return fromIrr(irrNode->getPosition()); }
    Vec3 getAbsolutePosition() const override { return fromIrr(irrNode->getAbsolutePosition()); }
    void setRotation(const Vec3& rot) override { irrNode->setRotation(toIrr(rot)); }
    Vec3 getRotation() const override { return fromIrr(irrNode->getRotation()); }
    void setScale(const Vec3& s) override { irrNode->setScale(toIrr(s)); }
    Vec3 getScale() const override { return fromIrr(irrNode->getScale()); }
    void setVisible(bool vis) override { irrNode->setVisible(vis); }
    bool isVisible() const override { return irrNode->isVisible(); }
    void setParent(bc::graphics::ISceneNode* parent) override {
        if (parent) irrNode->setParent(static_cast<irr::scene::ISceneNode*>(parent->getNativeNode()));
    }
    bc::graphics::ISceneNode* getParent() const override { return nullptr; }
    void remove() override { irrNode->remove(); }
    void setMaterialFlag(MaterialFlag flag, bool value) override { irrNode->setMaterialFlag(toIrrFlag(flag), value); }
    void setMaterialType(MaterialType type) override { irrNode->setMaterialType(toIrrMaterialType(type)); }
    void setMaterialTexture(int layer, TextureHandle tex) override {
        irrNode->setMaterialTexture(layer, static_cast<irr::video::ITexture*>(tex));
    }
    void setID(int id) override { irrNode->setID(id); }
    int getID() const override { return irrNode->getID(); }
    void* getNativeNode() const override { return irrNode; }

    // IAnimatedMeshNode interface
    MeshHandle getMesh() const override { return irrNode->getMesh(); }
    void setCurrentFrame(float frame) override { irrNode->setCurrentFrame(frame); }
    float getFrameCount() const override {
        irr::scene::IAnimatedMesh* mesh = irrNode->getMesh();
        return mesh ? static_cast<float>(mesh->getFrameCount()) : 0;
    }
    void setAnimationSpeed(float fps) override { irrNode->setAnimationSpeed(fps); }
    void setLoopMode(bool loop) override { irrNode->setLoopMode(loop); }
    int getMaterialCount() const override { return irrNode->getMaterialCount(); }
    void setMaterialFlagForAll(MaterialFlag flag, bool value) override {
        for (irr::u32 i = 0; i < irrNode->getMaterialCount(); i++)
            irrNode->getMaterial(i).setFlag(toIrrFlag(flag), value);
    }
    void setMaterialTypeForAll(MaterialType type) override {
        for (irr::u32 i = 0; i < irrNode->getMaterialCount(); i++)
            irrNode->getMaterial(i).MaterialType = toIrrMaterialType(type);
    }

private:
    irr::scene::IAnimatedMeshSceneNode* irrNode;
};

}}} // namespace bc::graphics::irrlicht

#endif // BC_GRAPHICS_IRRLICHT_MESHNODE_HPP
