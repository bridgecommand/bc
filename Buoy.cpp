#include "irrlicht.h"

#include "Buoy.hpp"

using namespace irr;

Buoy::Buoy(const irr::io::path& filename, const irr::core::vector3df& location, const irr::f32 scale, irr::scene::ISceneManager* smgr)
{
    scene::IMesh* buoyMesh = smgr->getMesh(filename);
	buoy = smgr->addMeshSceneNode( buoyMesh, 0, -1, location );

    buoy->setScale(core::vector3df(scale,scale,scale));
    buoy->setMaterialFlag(video::EMF_LIGHTING, false);
}

Buoy::~Buoy()
{
    //dtor
}
