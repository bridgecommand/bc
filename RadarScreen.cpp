#include "irrlicht.h"

#include "RadarScreen.hpp"
#include <iostream>

using namespace irr;

RadarScreen::RadarScreen()
{

}

RadarScreen::~RadarScreen()
{
    //dtor
}


void RadarScreen::load(irr::scene::ISceneManager* smgr, irr::scene::ISceneNode* parent, irr::core::vector3df offset)
{
    driver = smgr->getVideoDriver(); //store video driver so we can work with textures
    radarScreen = smgr->addBillboardSceneNode();
    radarScreen->setMaterialFlag(video::EMF_LIGHTING, false);
    radarScreen->setSize(core::dimension2d<f32>(0.5f, 0.5f)); //FIXME: Hardcoded size
    this->parent = parent;
    this->offset = offset;

    //add an initial texture to the screen
    video::IImage * radarImage = driver->createImage (video::ECF_A1R5G5B5, core::dimension2d<u32>(256, 256)); //Create image for radar calculation to work on
    radarScreen->setMaterialTexture(0,driver->addTexture("RadarImage",radarImage));
    radarImage->drop();
}

void RadarScreen::update(video::IImage* radarImage)
{
     //link camera rotation to shipNode
    // get transformation matrix of node
    core::matrix4 m;
    m.setRotationDegrees(parent->getRotation());

    // transform offset('offset' is relative to the local ship coordinates, and stays the same.)
    //'offsetTransformed' is transformed into the global coordinates
    core::vector3df offsetTransformed;
    m.transformVect(offsetTransformed,offset);

    //move screen
    radarScreen->setPosition(parent->getPosition() + offsetTransformed);

    //Drop old texture if present
    if (radarScreen->getMaterialCount()>0) {
        video::ITexture* oldTexture = radarScreen->getMaterial(0).getTexture(0);
        if (oldTexture!=0) {
            oldTexture->drop();
        }
    }
    //make texture from image and apply to the screen
    radarScreen->setMaterialTexture(0,driver->addTexture("RadarImage",radarImage));

}

