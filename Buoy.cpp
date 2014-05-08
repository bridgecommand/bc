#include "irrlicht.h"

#include "Buoy.hpp"

using namespace irr;

Buoy::Buoy(const irr::io::path& filename, const irr::core::vector3df& location, const irr::f32 scale, irr::scene::ISceneManager* smgr)
{
    scene::IMesh* buoyMesh = smgr->getMesh(filename);
	buoy = smgr->addMeshSceneNode( buoyMesh, 0, -1, location );

	//Set Ambient colour to match diffuse, so lighting works: Fixme: This should work by setting the lighting to use ECM_DIFFUSE_AND_AMBIENT?
	if(buoy->getMaterialCount()>0) {
        for(int mat=0;mat<buoy->getMaterialCount();mat++) {
            buoy->getMaterial(mat).AmbientColor.set(buoy->getMaterial(mat).DiffuseColor.color);
        }
    }

    buoy->setScale(core::vector3df(scale,scale,scale));
}

Buoy::~Buoy()
{
    //dtor
}
