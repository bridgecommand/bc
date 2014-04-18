#ifndef __CAMERA_HPP_INCLUDED__
#define __CAMERA_HPP_INCLUDED__

#include "irrlicht.h"

class Camera
{
    public:
        Camera();
        virtual ~Camera();

        void loadCamera(irr::scene::ISceneManager* smgr, irr::scene::IMeshSceneNode* par, irr::core::vector3df off);
        void updateCamera();

        void setPosition(irr::core::vector3df position);
        void setRotation(irr::core::vector3df rotation);

    private:
        irr::scene::ICameraSceneNode* camera;
        irr::scene::IMeshSceneNode* parent;
        irr::core::vector3df offset;
};

#endif