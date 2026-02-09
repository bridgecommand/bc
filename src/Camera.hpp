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

#include "graphics/Types.hpp"

#include <vector>

//Forward declarations
namespace irr { class ILogger; namespace scene { class ICameraSceneNode; class ISceneManager; class ISceneNode; } }

class Camera
{
    public:
        Camera();
        virtual ~Camera();

        void load(irr::scene::ISceneManager* smgr, irr::ILogger* logger, irr::scene::ISceneNode* parent, std::vector<bc::graphics::Vec3> views, std::vector<bool> isHighView, float hFOV, float lookAngle, float angleCorrection);
        irr::scene::ISceneNode* getSceneNode() const;
        bc::graphics::Vec3 getPosition() const;
        bc::graphics::Vec3 getBasePosition() const;
        bc::graphics::Matrix4 getBaseRotation() const;
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
        bc::graphics::Vec3 getForwardVector() const;
        void highView(bool highViewRequired);
        void changeView();
        void setView(uint32_t view);
        uint32_t getView() const;
        void setNearValue(float zn);
        void setFarValue(float zf);
        void setFrozen(bool frozen);
        void toggleFrozen();
        void applyOffset(float deltaX, float deltaY, float deltaZ);
        void update(float deltaTime=0, bc::graphics::Quaternion quat=bc::graphics::Quaternion(0,0,0,1), bc::graphics::Vec3 pos=bc::graphics::Vec3(0,0,0), bc::graphics::Vec2 lensShift=bc::graphics::Vec2(0,0), bool vrMode = false);

    private:
        irr::scene::ICameraSceneNode* camera;
        irr::scene::ISceneNode* parent;
        bc::graphics::Vec3 parentPosition;
        bc::graphics::Matrix4 parentAngles;
        bool frozen;
        irr::ILogger* logger;
        uint32_t currentView;
        std::vector<bc::graphics::Vec3> views;
        std::vector<bool> isHighView; //Should be the same size as views (todo: Make this into a struct with views)
        float angleCorrection;
        float lookAngle; //In degrees
        float lookUpAngle;
        float minLookUpAngle;
        float maxLookUpAngle;
        int32_t horizontalPanSpeed; //Degrees per second
        int32_t verticalPanSpeed; //Degrees per second
        float hFOV;//horizontal field of view (radians)
        bc::graphics::Vec3 sideViewVector; // Side vector
        bc::graphics::Vec3 frv;

        bool isHighViewActive;
        float previousLookAngle;
        float previousLookUpAngle;
};

#endif
