#ifndef __CAMERA_HPP_INCLUDED__
#define __CAMERA_HPP_INCLUDED__

#include "irrlicht.h"

class Camera
{
    public:
        Camera();
        virtual ~Camera();

        void loadCamera(irr::scene::ISceneManager*);
        void updateCamera(irr::core::vector3df, irr::core::vector3df, irr::core::vector3df);

        void setPosition(irr::core::vector3df);
        void setRotation(irr::core::vector3df);

    private:
        irr::scene::ICameraSceneNode* camera;
};

#endif
