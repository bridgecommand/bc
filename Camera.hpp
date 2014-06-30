#ifndef __CAMERA_HPP_INCLUDED__
#define __CAMERA_HPP_INCLUDED__

#include "irrlicht.h"

#include <vector>

class Camera
{
    public:
        Camera();
        virtual ~Camera();

        void load(irr::scene::ISceneManager* smgr, irr::scene::IMeshSceneNode* parent, std::vector<irr::core::vector3df> views);
        irr::scene::ISceneNode* getSceneNode() const;
        irr::core::vector3df getPosition() const;
        void lookLeft();
        void lookRight();
        void lookAhead();
        void lookAstern();
        void lookPort();
        void lookStbd();
        void changeView();
        void update();

    private:
        irr::scene::ICameraSceneNode* camera;
        irr::scene::IMeshSceneNode* parent;
        irr::u32 currentView;
        std::vector<irr::core::vector3df> views;
        irr::f32 lookAngle;
};

#endif
