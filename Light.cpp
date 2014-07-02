#include "Light.hpp"

#include <cmath> //For fmod
#include <iostream>
#include "Constants.hpp"

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

    this->smgr = smgr;

    lightLevel = 0;

    ambientColor = video::SColor(255,64,64,64);

    smgr->setAmbientLight(ambientColor);
    //add a directional light
    //scene::ILightSceneNode* light = smgr->addLightSceneNode( ownShip.getSceneNode(), core::vector3df(0,400,-200), video::SColorf(0.3f,0.3f,0.3f), 100000.0f, 1 );
    //Probably set this as an ELT_DIRECTIONAL light, to set an 'infinitely' far light with constant direction.
}

void Light::update(irr::f32 scenarioTime)
{
    //convert scenario time (in seconds) into hours
    irr::f32 hourTime = std::fmod(scenarioTime,SECONDS_IN_DAY)/SECONDS_IN_HOUR;

    //Light parameters
    irr::f32 sunRise = 6; //FIXME: Hardcoded
	irr::f32 sunSet = 18; //FIXME: Hardcoded
    irr::s32 lightLow=50;
	irr::s32 lightHigh=205;
	irr::s32 lightCos=45;

    if (hourTime >= 0               && hourTime < (sunRise - 0.5)) {lightLevel = lightLow;}
	if (hourTime >= (sunRise - 0.5) && hourTime < (sunRise + 0.5)) {lightLevel = (lightHigh-lightLow) * (hourTime - (sunRise - 0.5)) + lightLow;}
	if (hourTime >= (sunRise + 0.5) && hourTime < (sunSet  - 0.5)) {lightLevel = lightHigh;}
	if (hourTime >= (sunSet  - 0.5) && hourTime < (sunSet  + 0.5)) {lightLevel = (lightLow-lightHigh) * (hourTime - (sunSet - 0.5)) + lightHigh;}
	if (hourTime >= (sunSet  + 0.5) && hourTime <= 24            ) {lightLevel = lightLow;}

	//sinusoidal component
	lightLevel = (s32)lightLevel + lightCos*cos((2*PI/24.0)*(hourTime-12.0));

    //do something with ambient colour
    ambientColor=video::SColor(255,lightLevel,lightLevel,lightLevel);
    //update ambient light
    smgr->setAmbientLight(ambientColor);
}

irr::video::SColor Light::getLightSColor() const
{
    return ambientColor;
}

irr::u32 Light::getLightLevel() const
{
    return lightLevel;
}
