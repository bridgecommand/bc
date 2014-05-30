#ifndef __BUOY_HPP_INCLUDED__
#define __BUOY_HPP_INCLUDED__

#include "irrlicht.h"

#include <string>

class Buoy
{
    public:
        Buoy(const std::string& name, const irr::core::vector3df& location, irr::scene::ISceneManager* smgr);
        virtual ~Buoy();
        irr::core::vector3df getPosition() const;
    protected:
    private:
        irr::scene::IMeshSceneNode* buoy; //The scene node for the buoy.
};

#endif // __BUOY_HPP_INCLUDED__
