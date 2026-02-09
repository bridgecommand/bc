/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2024 James Packer

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License version 2 as
     published by the Free Software Foundation */

#ifndef BC_GRAPHICS_IRRLICHT_SCENENODE_HPP
#define BC_GRAPHICS_IRRLICHT_SCENENODE_HPP

#include "../ISceneNode.hpp"
#include <irrlicht.h>

namespace bc { namespace graphics { namespace irrlicht {

// Converts bc::graphics types to/from Irrlicht types
inline irr::core::vector3df toIrr(const Vec3& v) { return {v.x, v.y, v.z}; }
inline Vec3 fromIrr(const irr::core::vector3df& v) { return {v.X, v.Y, v.Z}; }
inline irr::core::vector2df toIrr2(const Vec2& v) { return {v.x, v.y}; }
inline Vec2 fromIrr2(const irr::core::vector2df& v) { return {v.X, v.Y}; }
inline irr::video::SColor toIrrColor(const Color& c) { return {c.a, c.r, c.g, c.b}; }
inline irr::video::SColorf toIrrColorf(const Colorf& c) { return {c.r, c.g, c.b, c.a}; }

inline irr::video::E_MATERIAL_FLAG toIrrFlag(MaterialFlag f) {
    switch (f) {
        case MaterialFlag::Lighting:         return irr::video::EMF_LIGHTING;
        case MaterialFlag::Fog:              return irr::video::EMF_FOG_ENABLE;
        case MaterialFlag::BackFaceCulling:  return irr::video::EMF_BACK_FACE_CULLING;
        case MaterialFlag::FrontFaceCulling: return irr::video::EMF_FRONT_FACE_CULLING;
        case MaterialFlag::Wireframe:        return irr::video::EMF_WIREFRAME;
        case MaterialFlag::ZBuffer:          return irr::video::EMF_ZBUFFER;
        case MaterialFlag::ZWrite:           return irr::video::EMF_ZWRITE_ENABLE;
        case MaterialFlag::AntiAliasing:     return irr::video::EMF_ANTI_ALIASING;
        case MaterialFlag::BilinearFilter:   return irr::video::EMF_BILINEAR_FILTER;
        case MaterialFlag::TrilinearFilter:  return irr::video::EMF_TRILINEAR_FILTER;
        case MaterialFlag::AnisotropicFilter:return irr::video::EMF_ANISOTROPIC_FILTER;
        case MaterialFlag::NormalizeNormals: return irr::video::EMF_NORMALIZE_NORMALS;
        case MaterialFlag::ColorMask:        return irr::video::EMF_COLOR_MASK;
        default: return irr::video::EMF_LIGHTING;
    }
}

inline irr::video::E_MATERIAL_TYPE toIrrMaterialType(MaterialType t) {
    switch (t) {
        case MaterialType::Solid:                    return irr::video::EMT_SOLID;
        case MaterialType::TransparentAddColor:      return irr::video::EMT_TRANSPARENT_ADD_COLOR;
        case MaterialType::TransparentAlphaChannel:  return irr::video::EMT_TRANSPARENT_ALPHA_CHANNEL;
        case MaterialType::TransparentAlphaChannelRef:return irr::video::EMT_TRANSPARENT_ALPHA_CHANNEL_REF;
        case MaterialType::TransparentVertexAlpha:   return irr::video::EMT_TRANSPARENT_VERTEX_ALPHA;
        case MaterialType::Lightmap:                 return irr::video::EMT_LIGHTMAP;
        case MaterialType::LightmapAdd:              return irr::video::EMT_LIGHTMAP_ADD;
        case MaterialType::DetailMap:                return irr::video::EMT_DETAIL_MAP;
        case MaterialType::SphereMap:                return irr::video::EMT_SPHERE_MAP;
        case MaterialType::OneTextureBlend:           return irr::video::EMT_ONETEXTURE_BLEND;
        default: return irr::video::EMT_SOLID;
    }
}

// Wraps irr::scene::ISceneNode behind bc::graphics::ISceneNode
class IrrlichtSceneNode : public bc::graphics::ISceneNode {
public:
    explicit IrrlichtSceneNode(irr::scene::ISceneNode* node) : irrNode(node) {}
    ~IrrlichtSceneNode() override = default;

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
        if (parent) {
            auto* p = static_cast<irr::scene::ISceneNode*>(parent->getNativeNode());
            irrNode->setParent(p);
        }
    }
    bc::graphics::ISceneNode* getParent() const override { return nullptr; /* migration: return wrapper */ }
    void remove() override { irrNode->remove(); }

    void setMaterialFlag(MaterialFlag flag, bool value) override {
        irrNode->setMaterialFlag(toIrrFlag(flag), value);
    }
    void setMaterialType(MaterialType type) override {
        irrNode->setMaterialType(toIrrMaterialType(type));
    }
    void setMaterialTexture(int layer, TextureHandle tex) override {
        irrNode->setMaterialTexture(layer, static_cast<irr::video::ITexture*>(tex));
    }

    void setID(int id) override { irrNode->setID(id); }
    int getID() const override { return irrNode->getID(); }

    void* getNativeNode() const override { return irrNode; }

protected:
    irr::scene::ISceneNode* irrNode;
};

}}} // namespace bc::graphics::irrlicht

#endif // BC_GRAPHICS_IRRLICHT_SCENENODE_HPP
