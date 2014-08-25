#ifndef __LANDOBJECT_HPP_INCLUDED__
#define __LANDOBJECT_HPP_INCLUDED__

#include "irrlicht.h"

#include <string>

class LandObject
{
    public:
        LandObject(const std::string& name, const irr::core::vector3df& location, irr::f32 rotation, irr::scene::ISceneManager* smgr);
        virtual ~LandObject();
        irr::core::vector3df getPosition() const;
        void moveNode(irr::f32 deltaX, irr::f32 deltaY, irr::f32 deltaZ);
    protected:
    private:
        irr::scene::IMeshSceneNode* landObject; //The scene node for the object.
};

#endif
