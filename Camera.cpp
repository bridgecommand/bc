#include "irrlicht.h"

#include "Camera.hpp"

using namespace irr;

Camera::Camera()
{

}

Camera::~Camera()
{
    //dtor
}


void Camera::loadCamera(irr::scene::ISceneManager* smgr)
{
    camera = smgr->addCameraSceneNode(0, core::vector3df(0,0,0), core::vector3df(0,0,1));
}

void Camera::updateCamera(irr::core::vector3df position, irr::core::vector3df upv, irr::core::vector3df target)
{
    camera->setPosition(position);
    camera->setUpVector(upv); //set up vector of camera
    camera->setTarget(target); //set target of camera (look at point)
    camera->updateAbsolutePosition();
}

void Camera::setPosition(irr::core::vector3df)
{

}

void Camera::setRotation(irr::core::vector3df)
{

}
