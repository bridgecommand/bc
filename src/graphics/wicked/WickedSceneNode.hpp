/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2026 James Packer

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License version 2 as
     published by the Free Software Foundation */

#ifndef BC_GRAPHICS_WICKED_SCENENODE_HPP
#define BC_GRAPHICS_WICKED_SCENENODE_HPP

#ifdef WITH_WICKED_ENGINE

#include "../ISceneNode.hpp"
#include "../IMeshNode.hpp"
#include "../ICameraNode.hpp"
#include "../ILightNode.hpp"
#include "WickedEngine.h"

namespace bc { namespace graphics { namespace wicked {

// Helper: convert bc::graphics::Vec3 to XMFLOAT3
inline DirectX::XMFLOAT3 toXM(const Vec3& v) {
    return {v.x, v.y, v.z};
}

// Helper: convert XMFLOAT3 to bc::graphics::Vec3
inline Vec3 fromXM(const DirectX::XMFLOAT3& v) {
    return {v.x, v.y, v.z};
}

// Wraps a Wicked Engine entity (TransformComponent) as an ISceneNode.
class WickedSceneNode : public ISceneNode {
public:
    WickedSceneNode(wi::ecs::Entity entity, wi::scene::Scene* scene)
        : entity(entity), weScene(scene) {}

    ~WickedSceneNode() override = default;

    void setPosition(const Vec3& pos) override {
        auto* transform = weScene->transforms.GetComponent(entity);
        if (transform) {
            transform->ClearTransform();
            transform->Translate(toXM(pos));
            transform->UpdateTransform();
        }
    }

    Vec3 getPosition() const override {
        auto* transform = weScene->transforms.GetComponent(entity);
        if (transform) {
            return {transform->GetPosition().x,
                    transform->GetPosition().y,
                    transform->GetPosition().z};
        }
        return {};
    }

    void setRotation(const Vec3& rot) override {
        auto* transform = weScene->transforms.GetComponent(entity);
        if (transform) {
            // rot is in degrees (Irrlicht convention), convert to radians
            float degToRad = 3.14159265358979f / 180.0f;
            DirectX::XMFLOAT3 radians = {rot.x * degToRad, rot.y * degToRad, rot.z * degToRad};
            transform->ClearTransform();
            transform->Translate(toXM(getPosition()));
            transform->RotateRollPitchYaw(radians);
            transform->UpdateTransform();
        }
    }

    Vec3 getRotation() const override {
        // WE uses quaternions internally; return Euler approximation
        auto* transform = weScene->transforms.GetComponent(entity);
        if (transform) {
            auto& q = transform->rotation_local;
            // Convert quaternion to Euler (pitch, yaw, roll) in degrees
            float radToDeg = 180.0f / 3.14159265358979f;
            float sinr_cosp = 2.0f * (q.w * q.x + q.y * q.z);
            float cosr_cosp = 1.0f - 2.0f * (q.x * q.x + q.y * q.y);
            float roll = std::atan2(sinr_cosp, cosr_cosp);
            float sinp = 2.0f * (q.w * q.y - q.z * q.x);
            float pitch = std::abs(sinp) >= 1.0f ? std::copysign(1.5707963f, sinp) : std::asin(sinp);
            float siny_cosp = 2.0f * (q.w * q.z + q.x * q.y);
            float cosy_cosp = 1.0f - 2.0f * (q.y * q.y + q.z * q.z);
            float yaw = std::atan2(siny_cosp, cosy_cosp);
            return {pitch * radToDeg, yaw * radToDeg, roll * radToDeg};
        }
        return {};
    }

    void setScale(const Vec3& scale) override {
        auto* transform = weScene->transforms.GetComponent(entity);
        if (transform) {
            transform->scale_local = toXM(scale);
            transform->SetDirty();
        }
    }

    Vec3 getScale() const override {
        auto* transform = weScene->transforms.GetComponent(entity);
        if (transform) {
            return fromXM(transform->scale_local);
        }
        return {1, 1, 1};
    }

    void setVisible(bool visible) override {
        auto* object = weScene->objects.GetComponent(entity);
        if (object) {
            if (visible) {
                object->SetRenderable(true);
            } else {
                object->SetRenderable(false);
            }
        }
        visible_ = visible;
    }

    bool isVisible() const override {
        return visible_;
    }

    void setParent(ISceneNode* parent) override {
        if (parent) {
            auto* wickedParent = dynamic_cast<WickedSceneNode*>(parent);
            if (wickedParent) {
                weScene->Component_Attach(entity, wickedParent->entity);
            }
        } else {
            weScene->Component_Detach(entity);
        }
    }

    Vec3 getAbsolutePosition() const override {
        auto* transform = weScene->transforms.GetComponent(entity);
        if (transform) {
            // World matrix column 3 contains the world position
            auto& w = transform->world;
            return {w._41, w._42, w._43};
        }
        return {};
    }

    ISceneNode* getParent() const override {
        return parent_;
    }

    void remove() override {
        weScene->Entity_Remove(entity);
    }

    void setMaterialFlag(MaterialFlag flag, bool value) override {}
    void setMaterialType(MaterialType type) override {}
    void setMaterialTexture(int layer, TextureHandle tex) override {}

    void setID(int id) override { id_ = id; }
    int getID() const override { return id_; }

    void* getNativeNode() const override { return (void*)(uintptr_t)entity; }

    wi::ecs::Entity getEntity() const { return entity; }

protected:
    wi::ecs::Entity entity = wi::ecs::INVALID_ENTITY;
    wi::scene::Scene* weScene = nullptr;
    ISceneNode* parent_ = nullptr;
    bool visible_ = true;
    int id_ = -1;
};

// Wraps a Wicked Engine entity with an ObjectComponent + MeshComponent
class WickedMeshNode : public IMeshNode {
public:
    WickedMeshNode(wi::ecs::Entity objectEntity, wi::ecs::Entity meshEntity,
                    wi::scene::Scene* scene)
        : objectEntity(objectEntity), meshEntity(meshEntity), weScene(scene) {}

    ~WickedMeshNode() override = default;

    // ISceneNode interface - delegates to object entity's transform
    void setPosition(const Vec3& pos) override {
        auto* transform = weScene->transforms.GetComponent(objectEntity);
        if (transform) {
            transform->ClearTransform();
            transform->Translate(toXM(pos));
            transform->UpdateTransform();
        }
    }

    Vec3 getPosition() const override {
        auto* transform = weScene->transforms.GetComponent(objectEntity);
        if (transform) {
            auto p = transform->GetPosition();
            return {p.x, p.y, p.z};
        }
        return {};
    }

    void setRotation(const Vec3& rot) override {
        auto* transform = weScene->transforms.GetComponent(objectEntity);
        if (transform) {
            float degToRad = 3.14159265358979f / 180.0f;
            transform->ClearTransform();
            transform->Translate(toXM(getPosition()));
            transform->RotateRollPitchYaw({rot.x * degToRad, rot.y * degToRad, rot.z * degToRad});
            transform->UpdateTransform();
        }
    }

    Vec3 getRotation() const override { return {}; }

    void setScale(const Vec3& scale) override {
        auto* transform = weScene->transforms.GetComponent(objectEntity);
        if (transform) {
            transform->scale_local = toXM(scale);
            transform->SetDirty();
        }
    }

    Vec3 getScale() const override {
        auto* transform = weScene->transforms.GetComponent(objectEntity);
        if (transform) return fromXM(transform->scale_local);
        return {1, 1, 1};
    }

    void setVisible(bool visible) override {
        auto* object = weScene->objects.GetComponent(objectEntity);
        if (object) object->SetRenderable(visible);
        visible_ = visible;
    }

    bool isVisible() const override { return visible_; }

    void setParent(ISceneNode* parent) override {
        if (parent) {
            auto* wp = dynamic_cast<WickedSceneNode*>(parent);
            if (wp) weScene->Component_Attach(objectEntity, wp->getEntity());
        } else {
            weScene->Component_Detach(objectEntity);
        }
    }

    Vec3 getAbsolutePosition() const override {
        auto* transform = weScene->transforms.GetComponent(objectEntity);
        if (transform) {
            return {transform->world._41, transform->world._42, transform->world._43};
        }
        return {};
    }

    ISceneNode* getParent() const override { return parent_; }

    void remove() override {
        weScene->Entity_Remove(objectEntity);
    }

    void setMaterialFlag(MaterialFlag flag, bool value) override {}
    void setMaterialType(MaterialType type) override {}
    void setMaterialTexture(int layer, TextureHandle tex) override {}

    void setID(int id) override { id_ = id; }
    int getID() const override { return id_; }

    void* getNativeNode() const override { return (void*)(uintptr_t)objectEntity; }

    // IMeshNode interface
    MeshHandle getMesh() const override {
        return (MeshHandle)(uintptr_t)meshEntity;
    }

    int getMaterialCount() const override {
        auto* mesh = weScene->meshes.GetComponent(meshEntity);
        if (mesh) return static_cast<int>(mesh->subsets.size());
        return 0;
    }

    void setMaterialFlagForAll(MaterialFlag flag, bool value) override {}
    void setMaterialTypeForAll(MaterialType type) override {}

    wi::ecs::Entity getObjectEntity() const { return objectEntity; }
    wi::ecs::Entity getMeshEntity() const { return meshEntity; }

protected:
    wi::ecs::Entity objectEntity = wi::ecs::INVALID_ENTITY;
    wi::ecs::Entity meshEntity = wi::ecs::INVALID_ENTITY;
    wi::scene::Scene* weScene = nullptr;
    ISceneNode* parent_ = nullptr;
    bool visible_ = true;
    int id_ = -1;
};

// Wraps a Wicked Engine entity with CameraComponent as an ICameraNode.
class WickedCameraNode : public ICameraNode {
public:
    WickedCameraNode(wi::ecs::Entity entity, wi::scene::Scene* scene)
        : entity_(entity), weScene(scene) {}

    ~WickedCameraNode() override = default;

    // ISceneNode interface
    void setPosition(const Vec3& pos) override {
        auto* transform = weScene->transforms.GetComponent(entity_);
        if (transform) {
            transform->ClearTransform();
            transform->Translate(toXM(pos));
            transform->UpdateTransform();
        }
        // Also update camera Eye for immediate effect
        auto* cam = weScene->cameras.GetComponent(entity_);
        if (cam) {
            cam->Eye = toXM(pos);
            cam->SetDirty();
        }
    }

    Vec3 getPosition() const override {
        auto* cam = weScene->cameras.GetComponent(entity_);
        if (cam) return fromXM(cam->Eye);
        return {};
    }

    void setRotation(const Vec3& rot) override {
        auto* transform = weScene->transforms.GetComponent(entity_);
        if (transform) {
            float degToRad = 3.14159265358979f / 180.0f;
            transform->ClearTransform();
            transform->Translate(toXM(getPosition()));
            transform->RotateRollPitchYaw({rot.x * degToRad, rot.y * degToRad, rot.z * degToRad});
            transform->UpdateTransform();
        }
    }

    Vec3 getRotation() const override { return {}; }

    void setScale(const Vec3& s) override {}
    Vec3 getScale() const override { return {1, 1, 1}; }

    void setVisible(bool vis) override { visible_ = vis; }
    bool isVisible() const override { return visible_; }

    void setParent(ISceneNode* parent) override {
        if (parent) {
            auto* wp = dynamic_cast<WickedSceneNode*>(parent);
            if (wp) weScene->Component_Attach(entity_, wp->getEntity());
        } else {
            weScene->Component_Detach(entity_);
        }
    }

    Vec3 getAbsolutePosition() const override {
        auto* cam = weScene->cameras.GetComponent(entity_);
        if (cam) return fromXM(cam->Eye);
        return {};
    }

    ISceneNode* getParent() const override { return nullptr; }

    void remove() override {
        weScene->Entity_Remove(entity_);
    }

    void setMaterialFlag(MaterialFlag, bool) override {}
    void setMaterialType(MaterialType) override {}
    void setMaterialTexture(int, TextureHandle) override {}

    void setID(int id) override { id_ = id; }
    int getID() const override { return id_; }

    void* getNativeNode() const override { return (void*)(uintptr_t)entity_; }

    // ICameraNode interface
    void setTarget(const Vec3& target) override {
        auto* cam = weScene->cameras.GetComponent(entity_);
        if (cam) {
            cam->At = toXM(target);
            cam->SetDirty();
        }
    }

    Vec3 getTarget() const override {
        auto* cam = weScene->cameras.GetComponent(entity_);
        if (cam) return fromXM(cam->At);
        return {};
    }

    void setUpVector(const Vec3& up) override {
        auto* cam = weScene->cameras.GetComponent(entity_);
        if (cam) {
            cam->Up = toXM(up);
            cam->SetDirty();
        }
    }

    Vec3 getUpVector() const override {
        auto* cam = weScene->cameras.GetComponent(entity_);
        if (cam) return fromXM(cam->Up);
        return {0, 1, 0};
    }

    void setFOV(float fovRadians) override {
        auto* cam = weScene->cameras.GetComponent(entity_);
        if (cam) {
            cam->fov = fovRadians;
            cam->SetDirty();
        }
    }

    float getFOV() const override {
        auto* cam = weScene->cameras.GetComponent(entity_);
        return cam ? cam->fov : 1.0472f; // default ~60 deg
    }

    void setAspectRatio(float aspect) override {
        auto* cam = weScene->cameras.GetComponent(entity_);
        if (cam && cam->height > 0) {
            cam->width = cam->height * aspect;
            cam->SetDirty();
        }
    }

    float getAspectRatio() const override {
        auto* cam = weScene->cameras.GetComponent(entity_);
        if (cam && cam->height > 0) return cam->width / cam->height;
        return 1.7778f; // 16:9 default
    }

    void setNearValue(float nearVal) override {
        auto* cam = weScene->cameras.GetComponent(entity_);
        if (cam) {
            cam->zNearP = nearVal;
            cam->SetDirty();
        }
    }

    float getNearValue() const override {
        auto* cam = weScene->cameras.GetComponent(entity_);
        return cam ? cam->zNearP : 0.1f;
    }

    void setFarValue(float farVal) override {
        auto* cam = weScene->cameras.GetComponent(entity_);
        if (cam) {
            cam->zFarP = farVal;
            cam->SetDirty();
        }
    }

    float getFarValue() const override {
        auto* cam = weScene->cameras.GetComponent(entity_);
        return cam ? cam->zFarP : 5000.0f;
    }

    Matrix4 getViewMatrix() const override {
        auto* cam = weScene->cameras.GetComponent(entity_);
        if (cam) return fromXMMatrix(cam->View);
        return Matrix4::identity();
    }

    Matrix4 getProjectionMatrix() const override {
        auto* cam = weScene->cameras.GetComponent(entity_);
        if (cam) return fromXMMatrix(cam->Projection);
        return Matrix4::identity();
    }

    wi::ecs::Entity getEntity() const { return entity_; }

private:
    static Matrix4 fromXMMatrix(const DirectX::XMFLOAT4X4& xm) {
        Matrix4 result;
        // XMFLOAT4X4 is row-major, same as our Matrix4
        for (int r = 0; r < 4; r++)
            for (int c = 0; c < 4; c++)
                result.m[r * 4 + c] = xm.m[r][c];
        return result;
    }

    wi::ecs::Entity entity_ = wi::ecs::INVALID_ENTITY;
    wi::scene::Scene* weScene = nullptr;
    bool visible_ = true;
    int id_ = -1;
};

// Wraps a Wicked Engine entity with LightComponent as an ILightNode.
class WickedLightNode : public ILightNode {
public:
    WickedLightNode(wi::ecs::Entity entity, wi::scene::Scene* scene)
        : entity_(entity), weScene(scene) {}

    ~WickedLightNode() override = default;

    // ISceneNode interface
    void setPosition(const Vec3& pos) override {
        auto* transform = weScene->transforms.GetComponent(entity_);
        if (transform) {
            transform->ClearTransform();
            transform->Translate(toXM(pos));
            transform->UpdateTransform();
        }
    }

    Vec3 getPosition() const override {
        auto* transform = weScene->transforms.GetComponent(entity_);
        if (transform) {
            return {transform->GetPosition().x,
                    transform->GetPosition().y,
                    transform->GetPosition().z};
        }
        return {};
    }

    void setRotation(const Vec3& rot) override {
        auto* transform = weScene->transforms.GetComponent(entity_);
        if (transform) {
            float degToRad = 3.14159265358979f / 180.0f;
            transform->ClearTransform();
            transform->Translate(toXM(getPosition()));
            transform->RotateRollPitchYaw({rot.x * degToRad, rot.y * degToRad, rot.z * degToRad});
            transform->UpdateTransform();
        }
    }

    Vec3 getRotation() const override { return {}; }

    void setScale(const Vec3& s) override {}
    Vec3 getScale() const override { return {1, 1, 1}; }

    void setVisible(bool vis) override { visible_ = vis; }
    bool isVisible() const override { return visible_; }

    void setParent(ISceneNode* parent) override {
        if (parent) {
            auto* wp = dynamic_cast<WickedSceneNode*>(parent);
            if (wp) weScene->Component_Attach(entity_, wp->getEntity());
        } else {
            weScene->Component_Detach(entity_);
        }
    }

    Vec3 getAbsolutePosition() const override {
        auto* transform = weScene->transforms.GetComponent(entity_);
        if (transform) {
            return {transform->world._41, transform->world._42, transform->world._43};
        }
        return {};
    }

    ISceneNode* getParent() const override { return nullptr; }

    void remove() override {
        weScene->Entity_Remove(entity_);
    }

    void setMaterialFlag(MaterialFlag, bool) override {}
    void setMaterialType(MaterialType) override {}
    void setMaterialTexture(int, TextureHandle) override {}

    void setID(int id) override { id_ = id; }
    int getID() const override { return id_; }

    void* getNativeNode() const override { return (void*)(uintptr_t)entity_; }

    // ILightNode interface
    void setLightData(const LightData& data) override {
        auto* light = weScene->lights.GetComponent(entity_);
        if (!light) return;
        light->SetType(toWELightType(data.type));
        light->color = {data.diffuse.r, data.diffuse.g, data.diffuse.b};
        light->intensity = 1.0f;
        light->range = data.radius;
        light->outerConeAngle = data.outerCone * (3.14159265358979f / 180.0f);
        light->innerConeAngle = data.innerCone * (3.14159265358979f / 180.0f);
    }

    LightData getLightData() const override {
        LightData data;
        auto* light = weScene->lights.GetComponent(entity_);
        if (light) {
            data.type = fromWELightType(light->GetType());
            data.diffuse = {light->color.x, light->color.y, light->color.z, 1.0f};
            data.radius = light->range;
            data.outerCone = light->outerConeAngle * (180.0f / 3.14159265358979f);
            data.innerCone = light->innerConeAngle * (180.0f / 3.14159265358979f);
        }
        return data;
    }

    void setLightType(LightType type) override {
        auto* light = weScene->lights.GetComponent(entity_);
        if (light) light->SetType(toWELightType(type));
    }

    LightType getLightType() const override {
        auto* light = weScene->lights.GetComponent(entity_);
        if (light) return fromWELightType(light->GetType());
        return LightType::Point;
    }

    void setRadius(float radius) override {
        auto* light = weScene->lights.GetComponent(entity_);
        if (light) light->range = radius;
    }

    float getRadius() const override {
        auto* light = weScene->lights.GetComponent(entity_);
        return light ? light->range : 100.0f;
    }

    wi::ecs::Entity getEntity() const { return entity_; }

private:
    static wi::scene::LightComponent::LightType toWELightType(LightType t) {
        switch (t) {
            case LightType::Point:       return wi::scene::LightComponent::POINT;
            case LightType::Directional: return wi::scene::LightComponent::DIRECTIONAL;
            case LightType::Spot:        return wi::scene::LightComponent::SPOT;
            default: return wi::scene::LightComponent::POINT;
        }
    }

    static LightType fromWELightType(wi::scene::LightComponent::LightType t) {
        switch (t) {
            case wi::scene::LightComponent::POINT:       return LightType::Point;
            case wi::scene::LightComponent::DIRECTIONAL: return LightType::Directional;
            case wi::scene::LightComponent::SPOT:        return LightType::Spot;
            default: return LightType::Point;
        }
    }

    wi::ecs::Entity entity_ = wi::ecs::INVALID_ENTITY;
    wi::scene::Scene* weScene = nullptr;
    bool visible_ = true;
    int id_ = -1;
};

// Wraps a Wicked Engine entity with animation as an IAnimatedMeshNode.
class WickedAnimatedMeshNode : public IAnimatedMeshNode {
public:
    WickedAnimatedMeshNode(wi::ecs::Entity objectEntity, wi::ecs::Entity meshEntity,
                            wi::ecs::Entity animEntity, wi::scene::Scene* scene)
        : objectEntity_(objectEntity), meshEntity_(meshEntity),
          animEntity_(animEntity), weScene(scene) {}

    ~WickedAnimatedMeshNode() override = default;

    // ISceneNode interface
    void setPosition(const Vec3& pos) override {
        auto* transform = weScene->transforms.GetComponent(objectEntity_);
        if (transform) {
            transform->ClearTransform();
            transform->Translate(toXM(pos));
            transform->UpdateTransform();
        }
    }

    Vec3 getPosition() const override {
        auto* transform = weScene->transforms.GetComponent(objectEntity_);
        if (transform) {
            auto p = transform->GetPosition();
            return {p.x, p.y, p.z};
        }
        return {};
    }

    void setRotation(const Vec3& rot) override {
        auto* transform = weScene->transforms.GetComponent(objectEntity_);
        if (transform) {
            float degToRad = 3.14159265358979f / 180.0f;
            transform->ClearTransform();
            transform->Translate(toXM(getPosition()));
            transform->RotateRollPitchYaw({rot.x * degToRad, rot.y * degToRad, rot.z * degToRad});
            transform->UpdateTransform();
        }
    }

    Vec3 getRotation() const override { return {}; }

    void setScale(const Vec3& scale) override {
        auto* transform = weScene->transforms.GetComponent(objectEntity_);
        if (transform) {
            transform->scale_local = toXM(scale);
            transform->SetDirty();
        }
    }

    Vec3 getScale() const override {
        auto* transform = weScene->transforms.GetComponent(objectEntity_);
        if (transform) return fromXM(transform->scale_local);
        return {1, 1, 1};
    }

    void setVisible(bool visible) override {
        auto* object = weScene->objects.GetComponent(objectEntity_);
        if (object) object->SetRenderable(visible);
        visible_ = visible;
    }

    bool isVisible() const override { return visible_; }

    void setParent(ISceneNode* parent) override {
        if (parent) {
            auto* wp = dynamic_cast<WickedSceneNode*>(parent);
            if (wp) weScene->Component_Attach(objectEntity_, wp->getEntity());
        } else {
            weScene->Component_Detach(objectEntity_);
        }
    }

    Vec3 getAbsolutePosition() const override {
        auto* transform = weScene->transforms.GetComponent(objectEntity_);
        if (transform) {
            return {transform->world._41, transform->world._42, transform->world._43};
        }
        return {};
    }

    ISceneNode* getParent() const override { return nullptr; }

    void remove() override {
        weScene->Entity_Remove(objectEntity_);
    }

    void setMaterialFlag(MaterialFlag, bool) override {}
    void setMaterialType(MaterialType) override {}
    void setMaterialTexture(int, TextureHandle) override {}

    void setID(int id) override { id_ = id; }
    int getID() const override { return id_; }

    void* getNativeNode() const override { return (void*)(uintptr_t)objectEntity_; }

    // IAnimatedMeshNode interface
    MeshHandle getMesh() const override {
        return (MeshHandle)(uintptr_t)meshEntity_;
    }

    void setCurrentFrame(float frame) override {
        auto* anim = weScene->animations.GetComponent(animEntity_);
        if (anim) {
            anim->timer = frame / 30.0f; // Convert frame number to time (assume 30fps)
        }
    }

    float getFrameCount() const override {
        auto* anim = weScene->animations.GetComponent(animEntity_);
        if (anim) {
            return (anim->end - anim->start) * 30.0f; // Convert time range to frame count
        }
        return 0;
    }

    void setAnimationSpeed(float fps) override {
        auto* anim = weScene->animations.GetComponent(animEntity_);
        if (anim) {
            anim->speed = fps / 30.0f; // Normalize to WE's internal speed
        }
    }

    void setLoopMode(bool loop) override {
        auto* anim = weScene->animations.GetComponent(animEntity_);
        if (anim) {
            if (loop) {
                anim->_flags |= wi::scene::AnimationComponent::LOOPED;
            } else {
                anim->_flags &= ~wi::scene::AnimationComponent::LOOPED;
            }
        }
    }

    int getMaterialCount() const override {
        auto* mesh = weScene->meshes.GetComponent(meshEntity_);
        if (mesh) return static_cast<int>(mesh->subsets.size());
        return 0;
    }

    void setMaterialFlagForAll(MaterialFlag, bool) override {}
    void setMaterialTypeForAll(MaterialType) override {}

    wi::ecs::Entity getObjectEntity() const { return objectEntity_; }

private:
    wi::ecs::Entity objectEntity_ = wi::ecs::INVALID_ENTITY;
    wi::ecs::Entity meshEntity_ = wi::ecs::INVALID_ENTITY;
    wi::ecs::Entity animEntity_ = wi::ecs::INVALID_ENTITY;
    wi::scene::Scene* weScene = nullptr;
    bool visible_ = true;
    int id_ = -1;
};

}}} // namespace bc::graphics::wicked

#endif // WITH_WICKED_ENGINE
#endif // BC_GRAPHICS_WICKED_SCENENODE_HPP
