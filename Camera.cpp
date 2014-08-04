#include "Camera.hpp"

#include <cmath>

using namespace irr;

Camera::Camera()
{

}

Camera::~Camera()
{
    //dtor
}


void Camera::load(irr::scene::ISceneManager* smgr, irr::scene::IMeshSceneNode* parent, std::vector<irr::core::vector3df> views)
{
    camera = smgr->addCameraSceneNode(0, core::vector3df(0,0,0), core::vector3df(0,0,1));
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

void Camera::changeView()
{
    currentView++;
    if (currentView==views.size()) {
        currentView = 0;
    }
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
