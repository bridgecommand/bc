#include "irrlicht.h"

#include "Buoy.hpp"

using namespace irr;

Buoy::Buoy(const irr::io::path& filename, const irr::core::vector3df& location, irr::scene::ISceneManager* smgr)
{
    scene::IMesh* buoyMesh = smgr->getMesh(filename);
    scene::IMeshSceneNode* buoy = smgr->addMeshSceneNode(buoyMesh,0,-1,location);
    buoy->setMaterialFlag(video::EMF_LIGHTING, false);
}

Buoy::~Buoy()
{
    //dtor
}
