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

#include "RadarScreen.hpp"
#include <iostream>

#ifdef WITH_PROFILING
#include "iprof.hpp"
#else
#define IPROF(a) //intentionally empty placeholder
#endif

//using namespace irr;

RadarScreen::RadarScreen()
{

}

RadarScreen::~RadarScreen()
{
    //dtor
}


void RadarScreen::load(irr::scene::ISceneManager* smgr, irr::scene::ISceneNode* parent, irr::core::vector3df offset, irr::f32 size, irr::f32 tilt)
{
    driver = smgr->getVideoDriver(); //store video driver so we can work with textures


	irr::scene::IMesh* radarPlane = smgr->getGeometryCreator()->createPlaneMesh(irr::core::dimension2d<irr::f32>(size, size));

    radarScreen = smgr->addMeshSceneNode(radarPlane);
    radarScreen->setMaterialFlag(irr::video::EMF_LIGHTING, false);
    this->parent = parent;
    this->offset = offset;
	this->tilt = tilt;
}

void RadarScreen::setRadarDisplayRadius(irr::u32 radiusPx)
{
    radarRadiusPx = radiusPx;
}

void RadarScreen::update(irr::video::IImage* radarImage)
{
    #ifdef WITH_PROFILING
    IPROF_FUNC;
    #endif

    irr::core::matrix4 m;
    irr::core::vector3df offsetTransformed;
    irr::video::ITexture* oldTexture = 0;

    radarScreen->setVisible(true);

    { IPROF("link camera rotation");
    //link camera rotation to shipNode
    // get transformation matrix of node
    m.setRotationDegrees(parent->getRotation());

    // transform offset('offset' is relative to the local ship coordinates, and stays the same.)
    //'offsetTransformed' is transformed into the global coordinates

    m.transformVect(offsetTransformed,offset);

    //move screen
    radarScreen->setPosition(parent->getPosition() + offsetTransformed);
	radarScreen->setRotation(parent->getRotation()+irr::core::vector3df(-90+tilt,0,0));

    }{ IPROF("Get old texture");

    //Get old texture if it exists

    if (radarScreen->getMaterialCount()>0) {
        oldTexture = radarScreen->getMaterial(0).getTexture(0);
    }
    }{ IPROF("Make texture from image");
    //make texture from image and apply to the screen
    radarScreen->setMaterialTexture(0,driver->addTexture("RadarImage",radarImage));
    }{ IPROF("Scale texture");
    //Scale the texture to get 1:1 image to screen pixel mapping
    irr::f32 radarTextureScaling=1;
    if (radarImage->getDimension().Width>0) {
        radarTextureScaling = (irr::f32)radarRadiusPx * 2.0 / radarImage->getDimension().Width;
        if (radarTextureScaling > 1) {radarTextureScaling = 1;} //Don't scale if not needed
    }
    radarScreen->getMaterial(0).getTextureMatrix(0).setTextureScale(radarTextureScaling,radarTextureScaling); //Use this to scale to the correct size: Ratio between radarImage size and the screen pixel diameter.
    }{ IPROF("Remove old texture");
    //Remove old texture if it exists
    if (oldTexture!=0) {
            driver->removeTexture(oldTexture);
    }
    }

}

irr::scene::ISceneNode* RadarScreen::getSceneNode() const
{
    return radarScreen;
}

