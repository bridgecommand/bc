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

#ifndef __CAMERA_HPP_INCLUDED__
#define __CAMERA_HPP_INCLUDED__

#include "irrlicht.h"

#include <vector>

class Camera
{
    public:
        Camera();
        virtual ~Camera();

        void load(irr::scene::ISceneManager* smgr, irr::scene::ISceneNode* parent, std::vector<irr::core::vector3df> views, irr::f32 hFOV, irr::f32 lookAngle, irr::f32 angleCorrection);
        irr::scene::ISceneNode* getSceneNode() const;
        irr::core::vector3df getPosition() const;
        void setHFOV(irr::f32 hFOV);
        void updateViewport(irr::f32 aspect);
        void setActive();
        void lookUp();
        void lookDown();
		void setLookUp(irr::f32 angle);
        void lookLeft();
        void lookRight();
        void lookChange(irr::f32 deltaX, irr::f32 deltaY); //As a proportion of screen width
        void lookStepLeft();
        void lookStepRight();
        void lookAhead();
        void lookAstern();
        void lookPort();
        void lookStbd();
        irr::f32 getLook() const;
        irr::f32 getLookUp() const;
        void changeView();
        void setView(irr::u32 view);
        irr::u32 getView() const;
        void setNearValue(irr::f32 zn);
        void setFarValue(irr::f32 zf);
        void update();

    private:
        irr::scene::ICameraSceneNode* camera;
        irr::scene::ISceneNode* parent;
        irr::u32 currentView;
        std::vector<irr::core::vector3df> views;
        irr::f32 angleCorrection;
        irr::f32 lookAngle; //In degrees
        irr::f32 lookUpAngle;
        irr::f32 hFOV;//horizontal field of view (radians)
};

#endif
