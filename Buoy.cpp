#include "irrlicht.h"

#include "Buoy.hpp"

using namespace irr;

Buoy::Buoy(const irr::io::path& filename, const irr::core::vector3df& location, const irr::f32 scale, irr::scene::ISceneManager* smgr)
{
    scene::IMesh* buoyMesh = smgr->getMesh(filename);
	buoy = smgr->addMeshSceneNode( buoyMesh, 0, -1, location );

    //Set lighting to use diffuse and ambient, so lighting of untextured models works
	if(buoy->getMaterialCount()>0) {
        for(int mat=0;mat<buoy->getMaterialCount();mat++) {
            buoy->getMaterial(mat).ColorMaterial = video::ECM_DIFFUSE_AND_AMBIENT;
        }
    }

    buoy->setScale(core::vector3df(scale,scale,scale));
}

Buoy::~Buoy()
{
    //dtor
}
