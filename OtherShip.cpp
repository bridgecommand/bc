#include "irrlicht.h"

#include "OtherShip.hpp"

using namespace irr;

OtherShip::OtherShip(const irr::io::path& filename, const irr::core::vector3df& location, const irr::f32 scale, irr::scene::ISceneManager* smgr)
{
    //FIXME: Use similar code for ownship loading (could extend from this?)
    scene::IMesh* otherShipMesh = smgr->getMesh(filename);
	otherShip = smgr->addMeshSceneNode( otherShipMesh, 0, -1, location );

    otherShip->setScale(core::vector3df(scale,scale,scale));
    otherShip->setMaterialFlag(video::EMF_LIGHTING, false);
}

OtherShip::~OtherShip()
{
    //dtor
}
