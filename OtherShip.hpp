#ifndef __OTHERSHIP_HPP_INCLUDED__
#define __OTHERSHIP_HPP_INCLUDED__

#include "irrlicht.h"

#include "NavLight.hpp"
#include "Leg.hpp"

#include <vector>
#include <string>

class OtherShip
{
    public:
        OtherShip (const std::string& name,const irr::core::vector3df& location, std::vector<Leg> legsLoaded, irr::scene::ISceneManager* smgr);
        virtual ~OtherShip();
        void setHeading(irr::f32 hdg);
        void setSpeed(irr::f32 spd);
        irr::f32 getHeading() const;
        irr::f32 getSpeed() const;
        void update(irr::f32 deltaTime, irr::f32 scenarioTime, irr::core::vector3df viewPosition);
        void setPosition(irr::core::vector3df position);
        void setRotation(irr::core::vector3df rotation);
    protected:
    private:
        irr::scene::IMeshSceneNode* otherShip; //The scene node for the other ship.
        std::vector<Leg> legs;
        std::vector<NavLight> navLights;
        irr::f32 heading;
        irr::f32 xPos;
        irr::f32 yPos;
        irr::f32 zPos;
        irr::f32 speed;
};

#endif
