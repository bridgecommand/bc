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

        void load(irr::scene::ISceneManager* smgr, irr::ILogger* logger, irr::scene::ISceneNode* parent, std::vector<irr::core::vector3df> views, std::vector<bool> isHighView, float hFOV, float lookAngle, float angleCorrection);
        irr::scene::ISceneNode* getSceneNode() const;
        irr::core::vector3df getPosition() const;
        irr::core::vector3df getBasePosition() const;
        irr::core::matrix4 getBaseRotation() const;
        void setHFOV(float hFOV);
        void updateViewport(float aspect);
        void setActive();
        void lookUp();
        void lookDown();
        void setPanSpeed(float horizontalPanSpeed);
        void setVerticalPanSpeed(float verticalPanSpeed);
		void setLookUp(float angle);
        void lookLeft();
        void lookRight();
        void lookChange(float deltaX, float deltaY); //As a proportion of screen width
        void lookStepLeft();
        void lookStepRight();
        void moveForwards();
        void moveBackwards();
        void lookAhead();
        void lookAstern();
        void lookPort();
        void lookStbd();
        float getLook() const;
        float getLookUp() const;
        irr::core::vector3df getForwardVector() const;
        void highView(bool highViewRequired);
        void changeView();
        void setView(unsigned int view);
        unsigned int getView() const;
        void setNearValue(float zn);
        void setFarValue(float zf);
        void setFrozen(bool frozen);
        void toggleFrozen();
        void applyOffset(float deltaX, float deltaY, float deltaZ);
        void update(float deltaTime=0, irr::core::quaternion quat=irr::core::quaternion(0,0,0,1), irr::core::vector3df pos=irr::core::vector3df(0,0,0), irr::core::vector2df lensShift=irr::core::vector2df(0,0), bool vrMode = false);

    private:
        irr::scene::ICameraSceneNode* camera;
        irr::scene::ISceneNode* parent;
        irr::core::vector3df parentPosition;
        irr::core::matrix4 parentAngles;
        bool frozen;
        irr::ILogger* logger;
        unsigned int currentView;
        std::vector<irr::core::vector3df> views;
        std::vector<bool> isHighView; //Should be the same size as views (todo: Make this into a struct with views)
        float angleCorrection;
        float lookAngle; //In degrees
        float lookUpAngle;
        float minLookUpAngle;
        float maxLookUpAngle;
        signed int horizontalPanSpeed; //Degrees per second
        signed int verticalPanSpeed; //Degrees per second
        float hFOV;//horizontal field of view (radians)
        irr::core::vector3df sideViewVector; // Side vector
        irr::core::vector3df frv;

        bool isHighViewActive;
        float previousLookAngle;
        float previousLookUpAngle;
};

#endif
