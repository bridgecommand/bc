/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2024 James Packer

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License version 2 as
     published by the Free Software Foundation */

#ifndef BC_GRAPHICS_IRRLICHT_LIGHTNODE_HPP
#define BC_GRAPHICS_IRRLICHT_LIGHTNODE_HPP

#include "../ILightNode.hpp"
#include "IrrlichtSceneNode.hpp"

namespace bc { namespace graphics { namespace irrlicht {

inline irr::video::E_LIGHT_TYPE toIrrLightType(LightType t) {
    switch (t) {
        case LightType::Point:       return irr::video::ELT_POINT;
        case LightType::Directional: return irr::video::ELT_DIRECTIONAL;
        case LightType::Spot:        return irr::video::ELT_SPOT;
        default: return irr::video::ELT_POINT;
    }
}

inline LightType fromIrrLightType(irr::video::E_LIGHT_TYPE t) {
    switch (t) {
        case irr::video::ELT_POINT:       return LightType::Point;
        case irr::video::ELT_DIRECTIONAL: return LightType::Directional;
        case irr::video::ELT_SPOT:        return LightType::Spot;
        default: return LightType::Point;
    }
}

// Wraps irr::scene::ILightSceneNode
class IrrlichtLightNode : public bc::graphics::ILightNode {
public:
    explicit IrrlichtLightNode(irr::scene::ILightSceneNode* node) : irrNode(node) {}
    ~IrrlichtLightNode() override = default;

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

    // ILightNode interface
    void setLightData(const LightData& data) override {
        irr::video::SLight light = irrNode->getLightData();
        light.Type = toIrrLightType(data.type);
        light.DiffuseColor = toIrrColorf(data.diffuse);
        light.SpecularColor = toIrrColorf(data.specular);
        light.AmbientColor = toIrrColorf(data.ambient);
        light.Direction = toIrr(data.direction);
        light.Radius = data.radius;
        light.InnerCone = data.innerCone;
        light.OuterCone = data.outerCone;
        light.Falloff = data.falloff;
        irrNode->setLightData(light);
    }

    LightData getLightData() const override {
        const irr::video::SLight& light = irrNode->getLightData();
        LightData data;
        data.type = fromIrrLightType(light.Type);
        data.diffuse = {light.DiffuseColor.r, light.DiffuseColor.g, light.DiffuseColor.b, light.DiffuseColor.a};
        data.specular = {light.SpecularColor.r, light.SpecularColor.g, light.SpecularColor.b, light.SpecularColor.a};
        data.ambient = {light.AmbientColor.r, light.AmbientColor.g, light.AmbientColor.b, light.AmbientColor.a};
        data.direction = fromIrr(light.Direction);
        data.radius = light.Radius;
        data.innerCone = light.InnerCone;
        data.outerCone = light.OuterCone;
        data.falloff = light.Falloff;
        return data;
    }

    void setLightType(LightType type) override {
        irrNode->setLightType(toIrrLightType(type));
    }
    LightType getLightType() const override {
        return fromIrrLightType(irrNode->getLightData().Type);
    }
    void setRadius(float radius) override { irrNode->setRadius(radius); }
    float getRadius() const override { return irrNode->getRadius(); }

private:
    irr::scene::ILightSceneNode* irrNode;
};

}}} // namespace bc::graphics::irrlicht

#endif // BC_GRAPHICS_IRRLICHT_LIGHTNODE_HPP
