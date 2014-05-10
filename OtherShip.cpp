#include "irrlicht.h"

#include "OtherShip.hpp"

using namespace irr;

OtherShip::OtherShip(const irr::io::path& filename, const irr::core::vector3df& location, const irr::f32 scaleFactor, const irr::f32 yCorrection, std::vector<Leg> legsLoaded, irr::scene::ISceneManager* smgr)
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

    //Set lighting to use diffuse and ambient, so lighting of untextured models works
	if(otherShip->getMaterialCount()>0) {
        for(int mat=0;mat<otherShip->getMaterialCount();mat++) {
            otherShip->getMaterial(mat).ColorMaterial = video::ECM_DIFFUSE_AND_AMBIENT;
        }
    }

    //store leg information
    legs=legsLoaded;
}

OtherShip::~OtherShip()
{
    //dtor
}

void OtherShip::update()
{
    //Fixme:
    //move according to what's in the 'legs' information
}
