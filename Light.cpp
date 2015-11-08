/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2014 James Packer

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License version 2 as
     published by the Free Software Foundation

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY Or FITNESS For A PARTICULAR PURPOSE.  See the
     GNU General Public License For more details.

     You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA. */

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

void Light::load(irr::scene::ISceneManager* smgr, irr::f32 sunRise, irr::f32 sunSet, irr::scene::ISceneNode* parent)
{

    this->smgr = smgr;
    this->sunRise = sunRise;
    this->sunSet = sunSet;
    this->parent = parent;

    lightLevel = 0;

    ambientColor = video::SColor(255,64,64,64);

    smgr->setAmbientLight(ambientColor);
    //add a directional light
    //scene::ILightSceneNode* light = smgr->addLightSceneNode( ownShip.getSceneNode(), core::vector3df(0,400,-200), video::SColorf(0.3f,0.3f,0.3f), 100000.0f, 1 );
    //Probably set this as an ELT_DIRECTIONAL light, to set an 'infinitely' far light with constant direction.

    directionalLight = smgr->addLightSceneNode();
    //directionalLight = smgr->addSphereSceneNode(0.5);
}

void Light::update(irr::f32 scenarioTime)
{
    //convert scenario time (in seconds) into hours
    irr::f32 hourTime = std::fmod(scenarioTime,SECONDS_IN_DAY)/SECONDS_IN_HOUR;

    //Light parameters
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

    //Update the directional light
    irr::video::SLight lightData;
    lightData.DiffuseColor = video::SColor(255,0,0,0);
    lightData.AmbientColor = video::SColor(255,0,0,0);
    lightData.SpecularColor = ambientColor;
    lightData.Radius = 500;
    directionalLight->setLightData(lightData);
    parent->updateAbsolutePosition();
    core::vector3df lightPosition = parent->getAbsolutePosition() + core::vector3df(0,100,100); //Light to south at 45deg up.
    directionalLight->setPosition(lightPosition);

}

irr::video::SColor Light::getLightSColor() const
{
    return ambientColor;
}

irr::u32 Light::getLightLevel() const
{
    return lightLevel;
}
