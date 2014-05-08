#include "irrlicht.h"

#include "OtherShip.hpp"

using namespace irr;

OtherShip::OtherShip(const irr::io::path& filename, const irr::core::vector3df& location, const irr::f32 scaleFactor, const irr::f32 yCorrection, irr::scene::ISceneManager* smgr)
{
    //FIXME: Use similar code for ownship loading (could extend from this?)
    scene::IMesh* shipMesh = smgr->getMesh(filename);

    //scale and translate
    core::matrix4 transformMatrix;
    transformMatrix.setScale(core::vector3df(scaleFactor,scaleFactor,scaleFactor));
    transformMatrix.setTranslation(core::vector3df(0,yCorrection*scaleFactor,0));
    smgr->getMeshManipulator()->transform(shipMesh,transformMatrix);

    //add to scene node
	otherShip = smgr->addMeshSceneNode( shipMesh, 0, -1, location );

    //Set Ambient colour to match diffuse, so lighting works: Fixme: This should work by setting the lighting to use ECM_DIFFUSE_AND_AMBIENT?
	if(otherShip->getMaterialCount()>0) {
        for(int mat=0;mat<otherShip->getMaterialCount();mat++) {
            otherShip->getMaterial(mat).AmbientColor.set(otherShip->getMaterial(mat).DiffuseColor.color);
        }
    }
}

OtherShip::~OtherShip()
{
    //dtor
}
