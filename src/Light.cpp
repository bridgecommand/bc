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

#include "irrlicht.h"

#include <cmath> //For fmod
#include <iostream>
#include "Constants.hpp"

namespace {
    inline irr::video::SColor toIrrColor(const bc::graphics::Color& c) {
        return irr::video::SColor(c.a, c.r, c.g, c.b);
    }
}

Light::Light()
{
    //ctor
}

Light::~Light()
{
    //dtor
}

void Light::load(irr::scene::ISceneManager* smgr, float sunRise, float sunSet, irr::scene::ISceneNode* parent)
{

    this->smgr = smgr;
    this->sunRise = sunRise;
    this->sunSet = sunSet;
    this->parent = parent;

    lightLevel = 0;

    ambientColor = bc::graphics::Color(255,64,64,64);
    smgr->setAmbientLight(toIrrColor(ambientColor));

    //add a directional light
    directionalLight = smgr->addLightSceneNode();
    directionalLight->setLightType(irr::video::ELT_DIRECTIONAL);
    directionalLight->setRotation(irr::core::vector3df(30,0,0)); //Light from South, 30 deg above horizon
    //Set non-varying light data
    irr::video::SLight lightData = directionalLight->getLightData();
    lightData.AmbientColor = irr::video::SColor(255,0,0,0);
    lightData.SpecularColor = irr::video::SColor(255,0,0,0);
    lightData.Radius = 50000;
    directionalLight->setLightData(lightData);

}

void Light::update(float scenarioTime)
{
    //convert scenario time (in seconds) into hours
    float hourTime = std::fmod(scenarioTime,SECONDS_IN_DAY)/SECONDS_IN_HOUR;

    //Light parameters
    int32_t lightLow=50;
	int32_t lightHigh=205;
	int32_t lightCos=45;

    if (hourTime >= 0               && hourTime < (sunRise - 0.5)) {lightLevel = lightLow;}
	if (hourTime >= (sunRise - 0.5) && hourTime < (sunRise + 0.5)) {lightLevel = (lightHigh-lightLow) * (hourTime - (sunRise - 0.5)) + lightLow;}
	if (hourTime >= (sunRise + 0.5) && hourTime < (sunSet  - 0.5)) {lightLevel = lightHigh;}
	if (hourTime >= (sunSet  - 0.5) && hourTime < (sunSet  + 0.5)) {lightLevel = (lightLow-lightHigh) * (hourTime - (sunSet - 0.5)) + lightHigh;}
	if (hourTime >= (sunSet  + 0.5) && hourTime <= 24            ) {lightLevel = lightLow;}

	//sinusoidal component
	lightLevel = (int32_t)lightLevel + lightCos*cos((2*PI/24.0)*(hourTime-12.0));

    //do something with ambient colour
    ambientColor = bc::graphics::Color(255,lightLevel,lightLevel,lightLevel);
    //update ambient light
    smgr->setAmbientLight(toIrrColor(ambientColor));

    //Update the directional light
    irr::video::SLight lightData = directionalLight->getLightData();
    lightData.DiffuseColor = toIrrColor(ambientColor);
    directionalLight->setLightData(lightData);

}

bc::graphics::Color Light::getLightSColor() const
{
    return ambientColor;
}

uint32_t Light::getLightLevel() const
{
    return lightLevel;
}
