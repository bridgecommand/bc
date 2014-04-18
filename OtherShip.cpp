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
    otherShip->setMaterialFlag(video::EMF_LIGHTING, false);
}

OtherShip::~OtherShip()
{
    //dtor
}
