#ifndef __CAMERA_HPP_INCLUDED__
#define __CAMERA_HPP_INCLUDED__

#include "irrlicht.h"

class Camera
{
    public:
        Camera();
        virtual ~Camera();

        void loadCamera(irr::scene::ISceneManager* smgr, irr::scene::IMeshSceneNode* par, irr::core::vector3df off);
        irr::scene::ISceneNode* getSceneNode() const;
        void updateCamera();

    private:
        irr::scene::ICameraSceneNode* camera;
        irr::scene::IMeshSceneNode* parent;
        irr::core::vector3df offset;
};

#endif
