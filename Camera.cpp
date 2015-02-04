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


void Camera::load(irr::scene::ISceneManager* smgr, irr::scene::ISceneNode* parent, std::vector<irr::core::vector3df> views)
{
    hFOV = core::PI/2.0f; //90 degrees
    camera = smgr->addCameraSceneNode(0, core::vector3df(0,0,0), core::vector3df(0,0,1));
    camera->setFarValue(6*M_IN_NM);//Todo: This should depend on the current (variable) fog range
    camera->setFOV(hFOV/camera->getAspectRatio()); //Convert horizontal field of view to vertical
    this->parent = parent;
    this->views = views;
    currentView = 0;
    lookAngle = 0;
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
}

void Camera::lookAstern()
{
    lookAngle = 180;
}

void Camera::lookPort()
{
    lookAngle = 270;
}

void Camera::lookStbd()
{
    lookAngle = 90;
}

irr::f32 Camera::getLook() const
{
    return lookAngle;
}

void Camera::changeView()
{
    currentView++;
    if (currentView==views.size()) {
        currentView = 0;
    }
}

irr::u32 Camera::getView() const
{
    return currentView;
}

void Camera::setAspectRatio(irr::f32 aspect)
{
    camera->setAspectRatio(aspect);
    camera->setFOV(hFOV/aspect);
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
        core::vector3df frv(1.0f*std::sin(irr::core::DEGTORAD*lookAngle), 0.0f, 1.0f*std::cos(irr::core::DEGTORAD*lookAngle));
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
