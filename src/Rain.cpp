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

#include "Rain.hpp"
#include "Utilities.hpp"
#include <iostream>


Rain::Rain() {
}

Rain::~Rain() {
}

void Rain::load(irr::scene::ISceneManager* smgr, irr::scene::ISceneNode* parent, irr::IrrlichtDevice* dev, irr::f32 ShipPosX, irr::f32 ShipPosY, irr::f32 ShipPosZ, irr::f32 ShipLength, irr::f32 ShipBreadth)
{
    this->parent = parent;
    irr::video::IVideoDriver* driver = smgr->getVideoDriver();
    irr::f32 a = 0, b = 0, c = 0, d = 0;

    //Make 4 "rain blocks" around the ship, so there are no rain into ship
    for (unsigned int i = 0; i < 4; i++)
    {
        if (i == 0) {//Front block
            a = ShipBreadth / RATIO_SHELTER_RAIN;
            b = RAIN_BORDER_MAX;
            c = -RAIN_BORDER_MAX;
            d = RAIN_BORDER_MAX;
        }
        else if (i == 1) {//Back block
            a = -ShipBreadth / RATIO_SHELTER_RAIN;
            b = -RAIN_BORDER_MAX;
            c = -RAIN_BORDER_MAX;
            d = RAIN_BORDER_MAX;
        }
        else if (i == 2) {//Right Block
            a = ShipBreadth / RATIO_SHELTER_RAIN;
            b = -ShipBreadth / RATIO_SHELTER_RAIN;
            c = -ShipLength / RATIO_SHELTER_RAIN;
            d = -(RAIN_BORDER_MAX * RAIN_BORDER_MAX)/ ShipBreadth;
        }
        else if (i == 3) {//Left Block
            a = ShipBreadth / RATIO_SHELTER_RAIN;
            b = -ShipBreadth / RATIO_SHELTER_RAIN;
            c = ShipLength / RATIO_SHELTER_RAIN;
            d = (RAIN_BORDER_MAX * RAIN_BORDER_MAX) / ShipBreadth;
        }

        //For each block, create the particles system
        ps[i] = smgr->addParticleSystemSceneNode(false);

        irr::scene::IParticleEmitter* em = ps[i]->createBoxEmitter(
            irr::core::aabbox3d<irr::f32>(a, 0, c, b, RAIN_BORDER_MAX, d),
            irr::core::vector3df(ShipPosX, RAIN_DIRECTION_AND_FORCE, ShipPosZ),
            0, 0,                            
            irr::video::SColor(0, 255, 255, 255),       
            irr::video::SColor(0, 255, 255, 255),      
            RAIN_DENSITY_MIN, RAIN_DENSITY_MAX, 0,
            irr::core::dimension2df(RAIN_DROP_SIZE_MIN, RAIN_DROP_SIZE_MIN),
            irr::core::dimension2df(RAIN_DROP_SIZE_MAX, RAIN_DROP_SIZE_MAX));

        ps[i]->setEmitter(em);
        em->drop();

        //Create density
        irr::scene::IParticleAffector* paf = ps[i]->createGravityAffector(irr::core::vector3df(0, RAIN_DIRECTION_AND_FORCE, 0), RAIN_TIME_FORCE_LOST);
        ps[i]->addAffector(paf);
        paf->drop();

        ps[i]->setMaterialFlag(irr::video::EMF_BACK_FACE_CULLING, true);
        ps[i]->setMaterialFlag(irr::video::EMF_LIGHTING, false);          
        ps[i]->setMaterialFlag(irr::video::EMF_ZWRITE_ENABLE, false);     
        ps[i]->setMaterialTexture(0, driver->getTexture("media/raindrop.png"));     
        ps[i]->setMaterialType(irr::video::EMT_TRANSPARENT_ALPHA_CHANNEL); 

        ps[i]->getMaterial(0).setTextureMatrix(0, irr::core::matrix4().buildTextureTransform(
            0, irr::core::vector2df(0, 0), irr::core::vector2df(1.0f, -1.0f), irr::core::vector2df(1.0f, -1.0f)
        ));

        smgr->getRootSceneNode()->addChild(ps[i]);
    }
}

void Rain::update(irr::f32 ShipPosX, irr::f32 ShipPosY, irr::f32 ShipPosZ, irr::f32 RainLevel)
{

    //Move rain blocks with the ship
    for (unsigned int i = 0; i < 4; i++)
    {
        ps[i]->setPosition(irr::core::vector3df(ShipPosX, 0.1, ShipPosZ));
        irr::scene::IParticleEmitter* em = ps[i]->getEmitter();

        if (RainLevel < RAIN_MIDDLE_INTENSITY)
        {
            em->setMinParticlesPerSecond(RainLevel * 10);
            em->setMaxParticlesPerSecond(RainLevel * 50);
        }
        else
        {
            em->setMinParticlesPerSecond(RainLevel * 50);
            em->setMaxParticlesPerSecond(RainLevel * 200);
        }
    }
}