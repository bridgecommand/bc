#ifndef __OWNSHIP_HPP_INCLUDED__
#define __OWNSHIP_HPP_INCLUDED__

#include "irrlicht.h"

class OwnShip
{
    public:
        OwnShip();
        virtual ~OwnShip();

        void loadModel(const irr::io::path& filename, const irr::core::vector3df& location, irr::scene::ISceneManager* smgr);
        irr::scene::IMeshSceneNode* getSceneNode();

        void setPosition(irr::core::vector3df position);
        void setRotation(irr::core::vector3df rotation);
        irr::core::vector3df getRotation();
        irr::core::vector3df getPosition();

    protected:
    private:
        irr::scene::IMeshSceneNode* ownShip; //The scene node for the own ship.
};

#endif // __OWNSHIP_HPP_INCLUDED__
