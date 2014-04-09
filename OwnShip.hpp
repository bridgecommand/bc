#ifndef __OWNSHIP_HPP_INCLUDED__
#define __OWNSHIP_HPP_INCLUDED__

#include "irrlicht.h"

class OwnShip
{
    public:
        OwnShip();
        virtual ~OwnShip();

        void loadModel(const irr::io::path&, const irr::core::vector3df&, irr::scene::ISceneManager*);
        irr::scene::IMeshSceneNode* getSceneNode();

        void setPosition(irr::f32 x, irr::f32 y, irr::f32 z); //Should these take in vector3df instead?
        void setRotation(irr::f32 rx, irr::f32 ry, irr::f32 rz); //Should these take in vector3df instead?
        irr::core::vector3df getRotation();
        irr::core::vector3df getPosition();

    protected:
    private:
        irr::scene::IMeshSceneNode* ownShip; //The scene node for the own ship.
};

#endif // __OWNSHIP_HPP_INCLUDED__
