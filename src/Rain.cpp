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

    for (unsigned int i = 0; i < 4; i++)
    {
        if (i == 0) {
            a = ShipBreadth /1.5;
            b = 200;
            c = -200;
            d = 200;
        }
        else if (i == 1) {
            a = -ShipBreadth / 1.5;
            b = -200;
            c = -200;
            d = 200;
        }
        else if (i == 2) {
            a = ShipBreadth / 1.5;
            b = -ShipBreadth / 1.5;
            c = -ShipLength / 1.5;
            d = -80000/ ShipBreadth;
        }
        else if (i == 3) {
            a = ShipBreadth / 1.5;
            b = -ShipBreadth / 1.5;
            c = ShipLength / 1.5;
            d = 80000/ ShipBreadth;
        }

        ps[i] = smgr->addParticleSystemSceneNode(false);

        irr::scene::IParticleEmitter* em = ps[i]->createBoxEmitter(
            irr::core::aabbox3d<irr::f32>(a, 0, c, b, 100, d), 
            irr::core::vector3df(ShipPosX, -0.1, ShipPosZ),   
            0, 0,                            
            irr::video::SColor(0, 255, 255, 255),       
            irr::video::SColor(0, 255, 255, 255),      
            700, 1000, 0,                         
            irr::core::dimension2df(0.2f, 0.2f),         
            irr::core::dimension2df(0.5f, 0.5f));        

        ps[i]->setEmitter(em);
        em->drop();

        irr::scene::IParticleAffector* paf = ps[i]->createGravityAffector(irr::core::vector3df(0, -0.1f, 0), 2000);
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
    for (unsigned int i = 0; i < 4; i++)
    {
        ps[i]->setPosition(irr::core::vector3df(ShipPosX, 0.1, ShipPosZ));
        irr::scene::IParticleEmitter* em = ps[i]->getEmitter();

        if (RainLevel < 5)
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