/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2024 James Packer

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License version 2 as
     published by the Free Software Foundation */

#ifndef BC_GRAPHICS_IRRLICHT_CAMERANODE_HPP
#define BC_GRAPHICS_IRRLICHT_CAMERANODE_HPP

#include "../ICameraNode.hpp"
#include "IrrlichtSceneNode.hpp"

namespace bc { namespace graphics { namespace irrlicht {

inline Matrix4 fromIrrMatrix(const irr::core::matrix4& m) {
    Matrix4 result;
    for (int i = 0; i < 16; i++) result.m[i] = m[i];
    return result;
}

// Wraps irr::scene::ICameraSceneNode
class IrrlichtCameraNode : public bc::graphics::ICameraNode {
public:
    explicit IrrlichtCameraNode(irr::scene::ICameraSceneNode* node) : irrNode(node) {}
    ~IrrlichtCameraNode() override = default;

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

    // ICameraNode interface
    void setTarget(const Vec3& target) override { irrNode->setTarget(toIrr(target)); }
    Vec3 getTarget() const override { return fromIrr(irrNode->getTarget()); }
    void setUpVector(const Vec3& up) override { irrNode->setUpVector(toIrr(up)); }
    Vec3 getUpVector() const override { return fromIrr(irrNode->getUpVector()); }
    void setFOV(float fov) override { irrNode->setFOV(fov); }
    float getFOV() const override { return irrNode->getFOV(); }
    void setAspectRatio(float aspect) override { irrNode->setAspectRatio(aspect); }
    float getAspectRatio() const override { return irrNode->getAspectRatio(); }
    void setNearValue(float near) override { irrNode->setNearValue(near); }
    float getNearValue() const override { return irrNode->getNearValue(); }
    void setFarValue(float far) override { irrNode->setFarValue(far); }
    float getFarValue() const override { return irrNode->getFarValue(); }
    Matrix4 getViewMatrix() const override { return fromIrrMatrix(irrNode->getViewMatrix()); }
    Matrix4 getProjectionMatrix() const override { return fromIrrMatrix(irrNode->getProjectionMatrix()); }

private:
    irr::scene::ICameraSceneNode* irrNode;
};

}}} // namespace bc::graphics::irrlicht

#endif // BC_GRAPHICS_IRRLICHT_CAMERANODE_HPP
