/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2014 James Packer

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License version 2 as
     published by the Free Software Foundation

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY Or FITNESS For A PARTICULAR PURPOSE.  See the
     GNU General Public License For more details.

     You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA. */

#include "Camera.hpp"
#include "Constants.hpp"
#include "irrlicht.h"

#include <cmath>
#include <string>

namespace {
    constexpr float RADTODEG = 180.0f / 3.14159265358979323846f;
    constexpr float DEGTORAD = 3.14159265358979323846f / 180.0f;

    inline irr::core::vector3df toIrrVec(const bc::graphics::Vec3& v) { return {v.x, v.y, v.z}; }
    inline bc::graphics::Vec3 fromIrrVec(const irr::core::vector3df& v) { return {v.X, v.Y, v.Z}; }
    inline irr::core::vector2df toIrrVec2(const bc::graphics::Vec2& v) { return {v.x, v.y}; }

    inline irr::core::matrix4 toIrrMat(const bc::graphics::Matrix4& m) {
        irr::core::matrix4 result;
        for (int i = 0; i < 16; i++) result[i] = m.m[i];
        return result;
    }
    inline bc::graphics::Matrix4 fromIrrMat(const irr::core::matrix4& m) {
        bc::graphics::Matrix4 result;
        for (int i = 0; i < 16; i++) result.m[i] = m[i];
        return result;
    }

    inline irr::core::quaternion toIrrQuat(const bc::graphics::Quaternion& q) {
        return irr::core::quaternion(q.x, q.y, q.z, q.w);
    }
}

//using namespace irr;

Camera::Camera()
{

}

Camera::~Camera()
{
    //dtor
}


void Camera::load(irr::scene::ISceneManager* smgr, irr::ILogger* logger, irr::scene::ISceneNode* parent, std::vector<bc::graphics::Vec3> views, std::vector<bool> isHighView, float hFOV, float lookAngle, float angleCorrection)
{
    this->hFOV = hFOV;
    camera = smgr->addCameraSceneNode(0, irr::core::vector3df(0,0,0), irr::core::vector3df(0,0,1));

    this->parent = parent;
    this->views = views;
    this->isHighView = isHighView;
    currentView = 0;
    this->lookAngle = lookAngle;
    minLookUpAngle = -85;
    maxLookUpAngle = 85;
    lookUpAngle = 0;
    this->angleCorrection = angleCorrection;

    this->logger = logger;

    verticalPanSpeed = 0;
    horizontalPanSpeed = 0;

    isHighViewActive = false;
    previousLookAngle = lookAngle;
    previousLookUpAngle = lookUpAngle;

    frozen = false;

    sideViewVector = bc::graphics::Vec3(1,0,0);
}

irr::scene::ISceneNode* Camera::getSceneNode() const
{
    return camera;
}

//Return the position of the camera, including any active VR camera offset
bc::graphics::Vec3 Camera::getPosition() const
{
    camera->updateAbsolutePosition();//ToDo: This may be needed, but seems odd that it's required
    return fromIrrVec(camera->getAbsolutePosition());
}

// Return the position of the base camera, without any VR camera offset
bc::graphics::Vec3 Camera::getBasePosition() const
{
    // Uses current parentPosition and parentAngles
    irr::core::matrix4 irrAngles = toIrrMat(parentAngles);
    irr::core::vector3df offsetTransformed;
    irrAngles.transformVect(offsetTransformed, toIrrVec(views[currentView]));
    irr::core::vector3df cameraPosition = toIrrVec(parentPosition) + offsetTransformed;
    return fromIrrVec(cameraPosition);
}

bc::graphics::Matrix4 Camera::getBaseRotation() const
{
    return parentAngles;
}

void Camera::lookUp()
{
    lookUpAngle++;
    if (lookUpAngle>maxLookUpAngle)
    {
        lookUpAngle=maxLookUpAngle;
    }
}

void Camera::lookDown()
{
    lookUpAngle--;
    if (lookUpAngle<minLookUpAngle)
    {
        lookUpAngle=minLookUpAngle;
    }
}

void Camera::setPanSpeed(float horizontalPanSpeed){
    this->horizontalPanSpeed = horizontalPanSpeed;
}

void Camera::setVerticalPanSpeed(float verticalPanSpeed){
    this->verticalPanSpeed = verticalPanSpeed;
}

void Camera::setLookUp(float angle)
{
	lookUpAngle = angle;
}

void Camera::lookLeft()
{
    lookAngle--;
    while (lookAngle<0)
    {
        lookAngle+=360;
    }
}

void Camera::lookRight()
{
    lookAngle++;
    while (lookAngle>=360)
    {
        lookAngle-=360;
    }
}

void Camera::lookChange(float deltaX, float deltaY) //Change as a proportion of screen width
{
    lookAngle -= deltaX*hFOV*RADTODEG;
    lookUpAngle += deltaY*hFOV*RADTODEG; //hFOV for this, as both are scaled by screen width
    while (lookAngle<0)
    {
        lookAngle+=360;
    }
    while (lookAngle>=360)
    {
        lookAngle-=360;
    }
    if (lookUpAngle>maxLookUpAngle)
    {
        lookUpAngle=maxLookUpAngle;
    }
    if (lookUpAngle<minLookUpAngle)
    {
        lookUpAngle=minLookUpAngle;
    }
}

void Camera::lookStepLeft()
{
    lookAngle -= hFOV*RADTODEG;
    while (lookAngle<0)
    {
        lookAngle+=360;
    }
}

void Camera::lookStepRight()
{
    lookAngle += hFOV*RADTODEG;
    while (lookAngle>=360)
    {
        lookAngle-=360;
    }
}

void Camera::lookAhead()
{
    lookAngle = 0;
    lookUpAngle = 0;
}

void Camera::lookAstern()
{
    lookAngle = 180;
    lookUpAngle = 0;
}

void Camera::lookPort()
{
    lookAngle = 270;
    lookUpAngle = 0;
}

void Camera::lookStbd()
{
    lookAngle = 90;
    lookUpAngle = 0;
}

float Camera::getLook() const
{
    return lookAngle;
}

float Camera::getLookUp() const
{
    return lookUpAngle;
}

bc::graphics::Vec3 Camera::getForwardVector() const
{
    return frv;
}

void Camera::moveForwards()
{
    irr::core::vector3df frvLocal(1.0f*std::sin(DEGTORAD*(lookAngle-angleCorrection))*std::cos(DEGTORAD*lookUpAngle), 1.0f*std::sin(DEGTORAD*lookUpAngle), 1.0f*std::cos(DEGTORAD*(lookAngle-angleCorrection))*std::cos(DEGTORAD*lookUpAngle));
    irr::core::vector3df irrView = toIrrVec(views[currentView]);
    irrView += 0.05 * frvLocal;
    views[currentView] = fromIrrVec(irrView);

    //For displaying position
    std::string cameraPositionText = "Camera: (";
    cameraPositionText.append(std::to_string(views[currentView].x));
    cameraPositionText.append(",");
    cameraPositionText.append(std::to_string(views[currentView].y));
    cameraPositionText.append(",");
    cameraPositionText.append(std::to_string(views[currentView].z));
    cameraPositionText.append(")");
    logger->log(cameraPositionText.c_str());
}

void Camera::moveBackwards()
{
    irr::core::vector3df frvLocal(1.0f*std::sin(DEGTORAD*(lookAngle-angleCorrection))*std::cos(DEGTORAD*lookUpAngle), 1.0f*std::sin(DEGTORAD*lookUpAngle), 1.0f*std::cos(DEGTORAD*(lookAngle-angleCorrection))*std::cos(DEGTORAD*lookUpAngle));
    irr::core::vector3df irrView = toIrrVec(views[currentView]);
    irrView -= 0.05 * frvLocal;
    views[currentView] = fromIrrVec(irrView);

    //For displaying position
    std::string cameraPositionText = "Camera: (";
    cameraPositionText.append(std::to_string(views[currentView].x));
    cameraPositionText.append(",");
    cameraPositionText.append(std::to_string(views[currentView].y));
    cameraPositionText.append(",");
    cameraPositionText.append(std::to_string(views[currentView].z));
    cameraPositionText.append(")");
    logger->log(cameraPositionText.c_str());
}

void Camera::highView(bool highViewRequired)
{
    if (isHighViewActive != highViewRequired) {
        if (!highViewRequired) {
            lookAngle = previousLookAngle;
            lookUpAngle = previousLookUpAngle;
            isHighViewActive = false;
        } else {
            previousLookAngle = lookAngle;
            previousLookUpAngle = lookUpAngle;
            lookAngle = 0;
            lookUpAngle = -89.99; //Almost straight down. Avoid -90 as this gives an odd rotation effect (gymbal lock?)
            isHighViewActive = true;
        }
    }
}

void Camera::changeView()
{
    currentView++;
    if (currentView==views.size()) {
        currentView = 0;
    }

    if (currentView<isHighView.size()) {
        if (isHighView[currentView]) {
            highView(true);
        } else {
            highView(false);
        }
    }

}

void Camera::setView(uint32_t view) {
    if (view<views.size()) {
        currentView = view;
    }

    if (currentView<isHighView.size()) {
        if (isHighView[currentView]) {
            highView(true);
        } else {
            highView(false);
        }
    }

}

uint32_t Camera::getView() const
{
    return currentView;
}

void Camera::setHFOV(float hFOV)
{
    this->hFOV=hFOV;
    float aspect=camera->getAspectRatio();

    float vFOV = 2*atan(tan(hFOV/2)/aspect); //Calculate vertical field of view angle from horizontal one
    camera->setFOV(vFOV);
}

void Camera::updateViewport(float aspect)
{
    camera->setAspectRatio(aspect);
    float vFOV = 2*atan(tan(hFOV/2)/aspect); //Calculate vertical field of view angle from horizontal one
    camera->setFOV(vFOV);
}

void Camera::setActive()
{
    camera->getSceneManager()->setActiveCamera(camera);
}

void Camera::setNearValue(float zn)
{
    camera->setNearValue(zn);
}

void Camera::setFarValue(float zf)
{
    camera->setFarValue(zf);
}

void Camera::setFrozen(bool frozen)
{
    this->frozen = frozen;
}

void Camera::toggleFrozen()
{
    frozen = !frozen;
}

void Camera::applyOffset(float deltaX, float deltaY, float deltaZ)
{
    if (frozen) {
        // Only applicable in frozen mode, otherwise update will be
        // handled when own ship position offset is applied
        parentPosition.x += deltaX;
        parentPosition.y += deltaY;
        parentPosition.z += deltaZ;
    }
}

void Camera::update(float deltaTime, bc::graphics::Quaternion quat, bc::graphics::Vec3 pos, bc::graphics::Vec2 lensShift, bool vrMode)
{
     //link camera rotation to shipNode
        //Adjust camera angle if panning
        lookAngle += horizontalPanSpeed * deltaTime;
        while (lookAngle>=360) {
            lookAngle -= 360;
        }
        while (lookAngle<0) {
            lookAngle += 360;
        }

        lookUpAngle += verticalPanSpeed * deltaTime;
        if (lookUpAngle > maxLookUpAngle) {
            lookUpAngle = maxLookUpAngle;
        }
        if (lookUpAngle < minLookUpAngle) {
            lookUpAngle = minLookUpAngle;
        }

        // get position and transformation matrix of parent node, unless camera is static
        if (!frozen) {
            irr::core::matrix4 irrAngles;
            irrAngles.setRotationDegrees(parent->getRotation());
            parentAngles = fromIrrMat(irrAngles);
            parentPosition = fromIrrVec(parent->getPosition());
        }

        // Convert to Irrlicht types for math operations
        irr::core::quaternion irrQuat = toIrrQuat(quat);
        irr::core::matrix4 irrParentAngles = toIrrMat(parentAngles);

        // Quaternion for the view angles, ignoring the lookUpAngle if in VR Mode
        irr::core::quaternion viewQuat;
        if (vrMode) {
            viewQuat = irr::core::quaternion(0,
                                             DEGTORAD*(lookAngle-angleCorrection),
                                             0);
        } else {
            viewQuat = irr::core::quaternion(DEGTORAD*(-1*lookUpAngle),
                                             DEGTORAD*(lookAngle-angleCorrection),
                                             0);
        }

        // transform forward vector of camera
        irr::core::vector3df irrFrv(0.0f,0.0f,1.0f);
        irrFrv=irrQuat*irrFrv;
        irrFrv=viewQuat*irrFrv;
        irrParentAngles.transformVect(irrFrv);
        frv = fromIrrVec(irrFrv);

        // transform upvector of camera
        irr::core::vector3df upv(0.0f, 1.0f, 0.0f);
        upv = irrQuat*upv;
        upv = viewQuat*upv;
        irrParentAngles.transformVect(upv);

        // Update side view vector (must be perpendicular to upv and irrFrv)
        sideViewVector = fromIrrVec(upv.crossProduct(irrFrv));

        // transform camera offset ('offset' is relative to the local ship coordinates, and stays the same.)
        //'offsetTransformed' is transformed into the global coordinates
        irr::core::vector3df offsetTransformed;
        irrParentAngles.transformVect(offsetTransformed, toIrrVec(views[currentView]) + toIrrVec(pos));

        // Set lens shift (TODO: avoid recalculation)
        camera->setLensShift(toIrrVec2(lensShift));

        //move camera and angle
        irr::core::vector3df cameraPosition = toIrrVec(parentPosition) + offsetTransformed;
        camera->setPosition(cameraPosition);
        camera->setUpVector(upv); //set up vector of camera
        camera->setTarget(cameraPosition + irrFrv); //set target of camera (look at point)
        camera->updateAbsolutePosition();

        //also set rotation, so we can get camera's direction
        camera->setRotation(parent->getRotation());

}
