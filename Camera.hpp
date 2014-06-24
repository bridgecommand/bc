#ifndef __CAMERA_HPP_INCLUDED__
#define __CAMERA_HPP_INCLUDED__

#include "irrlicht.h"

class Camera
{
    public:
        Camera();
        virtual ~Camera();

        void load(irr::scene::ISceneManager* smgr, irr::scene::IMeshSceneNode* parent, irr::core::vector3df offset);
        irr::scene::ISceneNode* getSceneNode() const;
        irr::core::vector3df getPosition() const;
        void update();

    private:
        irr::scene::ICameraSceneNode* camera;
        irr::scene::IMeshSceneNode* parent;
        irr::core::vector3df offset;
};

#endif
