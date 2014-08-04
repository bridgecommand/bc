#include "Sky.hpp"

using namespace irr;

Sky::Sky(irr::scene::ISceneManager* smgr)
{
    irr::video::IVideoDriver* driver = smgr->getVideoDriver();
    driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, false);
    skyNode=smgr->addSkyDomeSceneNode(driver->getTexture("media/sky.bmp"),16,8,0.95f,2.0f);
    driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, true);
    skyNode->setMaterialFlag(video::EMF_FOG_ENABLE, true);
    skyNode->setMaterialFlag(video::EMF_LIGHTING, true); //Turn on lighting, so the sky gets dark as ambient light goes down

}

Sky::~Sky()
{
    //dtor
}
