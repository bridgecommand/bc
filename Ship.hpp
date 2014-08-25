//Parent class for own and other ships - not used un-extended

#ifndef __SHIP_HPP_INCLUDED__
#define __SHIP_HPP_INCLUDED__

#include "irrlicht.h"
#include <string>

//Forward declarations
class SimulationModel;

class Ship
{
    public:
        Ship();
        virtual ~Ship();

        irr::scene::IMeshSceneNode* getSceneNode() const;
        irr::core::vector3df getRotation() const;
        irr::core::vector3df getPosition() const;
        void setHeading(irr::f32 hdg);
        void setSpeed(irr::f32 spd);
        irr::f32 getHeading() const;
        irr::f32 getSpeed() const;
        void moveNode(irr::f32 deltaX, irr::f32 deltaY, irr::f32 deltaZ);

    protected:

        void setPosition(irr::core::vector3df position);
        void setRotation(irr::core::vector3df rotation);

        irr::scene::IMeshSceneNode* ship; //The scene node for the own ship.
        irr::f32 hdg;
        irr::f32 xPos;
        irr::f32 yPos;
        irr::f32 zPos;
        irr::f32 spd;
};

#endif // __SHIP_HPP_INCLUDED__
