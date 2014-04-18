#include "irrlicht.h"

#include "Water.hpp"

using namespace irr;

Water::Water(irr::scene::ISceneManager* smgr, irr::video::IVideoDriver* driver)
{
    //some water (from demo 8)
    scene::IAnimatedMesh* waterMesh = smgr->addHillPlaneMesh( "myHill",
                       core::dimension2d<f32>(500,500),
                       core::dimension2d<u32>(100,100), 0, 0,
                       core::dimension2d<f32>(0,0),
                       core::dimension2d<f32>(10,10));

    waterNode = smgr->addWaterSurfaceSceneNode(waterMesh->getMesh(0), 0.5f, 300.0f, 10.0f);
    waterNode->setPosition(core::vector3df(0,-2*0.5f,0));

    waterNode->setMaterialTexture(0, driver->getTexture("media/water.bmp"));
    waterNode->setMaterialFlag(video::EMF_LIGHTING, false);
    waterNode->setMaterialFlag(video::EMF_FOG_ENABLE, true);
}

Water::~Water()
{
    //dtor
}
