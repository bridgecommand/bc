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

#include <cmath>


using namespace irr;

Camera::Camera()
{

}

Camera::~Camera()
{
    //dtor
}


void Camera::load(irr::scene::ISceneManager* smgr, irr::scene::ISceneNode* parent, std::vector<irr::core::vector3df> views, irr::f32 hFOV, irr::f32 lookAngle, irr::f32 angleCorrection)
{
    this->hFOV = hFOV;
    camera = smgr->addCameraSceneNode(0, core::vector3df(0,0,0), core::vector3df(0,0,1));

    this->parent = parent;
    this->views = views;
    currentView = 0;
    this->lookAngle = lookAngle;
    lookUpAngle = 0;
    this->angleCorrection = angleCorrection;
}

irr::scene::ISceneNode* Camera::getSceneNode() const
{
    return camera;
}

irr::core::vector3df Camera::getPosition() const
{
    camera->updateAbsolutePosition();//ToDo: This may be needed, but seems odd that it's required
    return camera->getAbsolutePosition();
}

void Camera::lookUp()
{
    lookUpAngle++;
    if (lookUpAngle>40)
    {
        lookUpAngle=40;
    }
}

void Camera::lookDown()
{
    lookUpAngle--;
    if (lookUpAngle<-40)
    {
        lookUpAngle=-40;
    }
}

void Camera::setLookUp(irr::f32 angle)
{
	lookUpAngle = angle;
}

void Camera::lookLeft()
{
    lookAngle--;
    if (lookAngle<0)
    {
        lookAngle+=360;
    }
}

void Camera::lookRight()
{
    lookAngle++;
    if (lookAngle>=360)
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

irr::f32 Camera::getLook() const
{
    return lookAngle;
}

irr::f32 Camera::getLookUp() const
{
    return lookUpAngle;
}

void Camera::changeView()
{
    currentView++;
    if (currentView==views.size()) {
        currentView = 0;
    }
}

void Camera::setView(irr::u32 view) {
    if (view<views.size()) {
        currentView = view;
    }
}

irr::u32 Camera::getView() const
{
    return currentView;
}

void Camera::setHFOV(irr::f32 hFOV)
{
    this->hFOV=hFOV;
    irr::f32 aspect=camera->getAspectRatio();

    irr::f32 vFOV = 2*atan(tan(hFOV/2)/aspect); //Calculate vertical field of view angle from horizontal one
    camera->setFOV(vFOV);
}

void Camera::updateViewport(irr::f32 aspect)
{
    camera->setAspectRatio(aspect);
    irr::f32 vFOV = 2*atan(tan(hFOV/2)/aspect); //Calculate vertical field of view angle from horizontal one
    camera->setFOV(vFOV);
}

void Camera::setActive()
{
    camera->getSceneManager()->setActiveCamera(camera);
}

void Camera::setNearValue(irr::f32 zn)
{
    camera->setNearValue(zn);
}

void Camera::setFarValue(irr::f32 zf)
{
    camera->setFarValue(zf);
}

void Camera::update()
{
     //link camera rotation to shipNode
        // get transformation matrix of node
        core::matrix4 m;
        m.setRotationDegrees(parent->getRotation());

        // transform forward vector of camera
        core::vector3df frv(1.0f*std::sin(irr::core::DEGTORAD*(lookAngle-angleCorrection))*std::cos(irr::core::DEGTORAD*lookUpAngle), 1.0f*std::sin(irr::core::DEGTORAD*lookUpAngle), 1.0f*std::cos(irr::core::DEGTORAD*(lookAngle-angleCorrection))*std::cos(irr::core::DEGTORAD*lookUpAngle));
        m.transformVect(frv);

        // transform upvector of camera
        core::vector3df upv(0.0f, 1.0f, 0.0f);
        m.transformVect(upv);

        // transform camera offset ('offset' is relative to the local ship coordinates, and stays the same.)
        //'offsetTransformed' is transformed into the global coordinates
        core::vector3df offsetTransformed;
        m.transformVect(offsetTransformed,views[currentView]);

        //move camera and angle
        camera->setPosition(parent->getPosition() + offsetTransformed);
        camera->setUpVector(upv); //set up vector of camera
        camera->setTarget(parent->getPosition() + offsetTransformed + frv); //set target of camera (look at point)
        camera->updateAbsolutePosition();

        //also set rotation, so we can get camera's direction
        camera->setRotation(parent->getRotation());

}
