#include "Light.hpp"

using namespace irr;

Light::Light()
{
    //ctor
}

Light::~Light()
{
    //dtor
}

void Light::load(irr::scene::ISceneManager* smgr)
{

    scene = smgr;

    ambientColor = video::SColor(255,64,64,64);

    scene->setAmbientLight(ambientColor);
    //add a directional light
    //scene::ILightSceneNode* light = smgr->addLightSceneNode( ownShip.getSceneNode(), core::vector3df(0,400,-200), video::SColorf(0.3f,0.3f,0.3f), 100000.0f, 1 );
    //Probably set this as an ELT_DIRECTIONAL light, to set an 'infinitely' far light with constant direction.
}

void Light::update()
{
    //do something with ambient colour
    ambientColor=ambientColor;
    //update ambient light
    scene->setAmbientLight(ambientColor);
}

irr::video::SColor Light::getLightSColor()
{
    return ambientColor;
}
