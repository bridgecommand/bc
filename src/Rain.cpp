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

//using namespace irr;

Rain::Rain() {
}

Rain::~Rain() {
}

void Rain::load(irr::scene::ISceneManager* smgr, irr::scene::ISceneNode* parent, irr::IrrlichtDevice* dev, irr::f32 ShipPosX, irr::f32 ShipPosY, irr::f32 ShipPosZ)
{
    //Make rain
    this->parent = parent;
    irr::video::IVideoDriver* driver = smgr->getVideoDriver();

    // Création du système de particules
    ps = smgr->addParticleSystemSceneNode(false);

    irr::scene::IParticleEmitter* em = ps->createSphereEmitter(
        irr::core::vector3df(ShipPosX, ShipPosY, ShipPosZ),    // Position du centre de la sphère
        500.0f,                       // Rayon de la sphère
        irr::core::vector3df(0.0f, -0.3f, 0.0f),  // Direction des particules
        2800, 3000,                   // Particules par seconde
        irr::video::SColor(255, 200, 200, 255),  // Couleur minimale
        irr::video::SColor(255, 255, 255, 255),  // Couleur maximale
        2000, 3000,                   // Durée de vie des particules
        0,                           // Angle d'émission (0 pour collimaté)
        irr::core::dimension2df(0.5f, 0.5f),   // Taille minimale des particules
        irr::core::dimension2df(1.0f, 1.0f));  // Taille maximale des particules

    ps->setEmitter(em);
    em->drop();

    // Ajout d'un affecteur de gravité pour les particules
    irr::scene::IParticleAffector* paf = ps->createGravityAffector(irr::core::vector3df(0, -0.1f, 0), 2000);
    ps->addAffector(paf);
    paf->drop();

    // Configuration du matériau des particules
    ps->setMaterialFlag(irr::video::EMF_BACK_FACE_CULLING, true);
    ps->setMaterialFlag(irr::video::EMF_LIGHTING, false);          // insensible a la lumiere
    ps->setMaterialFlag(irr::video::EMF_ZWRITE_ENABLE, false);     // desactive zbuffer pour surfaces derriere
    ps->setMaterialTexture(0, driver->getTexture("media/raindrop.png"));     // on colle une texture
    ps->setMaterialType(irr::video::EMT_TRANSPARENT_ALPHA_CHANNEL); // application transparence

    ps->getMaterial(0).setTextureMatrix(0, irr::core::matrix4().buildTextureTransform(
        0, irr::core::vector2df(0, 0), irr::core::vector2df(1.0f, -1.0f), irr::core::vector2df(1.0f, -1.0f)
    ));

    smgr->getRootSceneNode()->addChild(ps);
}

void Rain::setPos(irr::f32 ShipPosX, irr::f32 ShipPosY, irr::f32 ShipPosZ)
{

    ps->setPosition(irr::core::vector3df(ShipPosX, ShipPosY, ShipPosZ));

}