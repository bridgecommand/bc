#ifndef __OWNSHIP_HPP_INCLUDED__
#define __OWNSHIP_HPP_INCLUDED__

#include "irrlicht.h"
#include <string>

//Forward declarations
class SimulationModel;

class OwnShip
{
    public:
        OwnShip();
        virtual ~OwnShip();

        void load(const std::string& scenarioName, irr::scene::ISceneManager* smgr, SimulationModel* model);
        irr::scene::IMeshSceneNode* getSceneNode() const;
        void update(irr::f32 deltaTime);
        irr::core::vector3df getRotation() const;
        irr::core::vector3df getPosition() const;
        irr::core::vector3df getCameraOffset() const;
        void setHeading(irr::f32 hdg);
        void setSpeed(irr::f32 spd);
        irr::f32 getHeading() const;
        irr::f32 getSpeed() const;

    protected:
    private:

        void setPosition(irr::core::vector3df position);
        void setRotation(irr::core::vector3df rotation);

        irr::scene::IMeshSceneNode* ownShip; //The scene node for the own ship.
        irr::core::vector3df cameraOffset; //The offset of the camera origin from the own ship origin
        irr::f32 heading;
        irr::f32 xPos;
        irr::f32 yPos;
        irr::f32 zPos;
        irr::f32 speed;
};

#endif // __OWNSHIP_HPP_INCLUDED__
