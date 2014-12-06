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

using namespace irr;

RadarScreen::RadarScreen()
{

}

RadarScreen::~RadarScreen()
{
    //dtor
}


void RadarScreen::load(irr::scene::ISceneManager* smgr, irr::scene::ISceneNode* parent, irr::core::vector3df offset)
{
    driver = smgr->getVideoDriver(); //store video driver so we can work with textures

    /* For future investigation - make radar screen from sets of triangles for each cell, so you set the cell colour by setting the material colour
    irr::scene::SMesh* mesh = 0;
    irr::scene::SMeshBuffer *buf = 0;

    // create new buffer
    mesh = new irr::scene::SMesh();
    buf = new irr::scene::SMeshBuffer();

    //
    mesh->addMeshBuffer(buf);
    smgr->addMeshSceneNode(mesh);

    //add vertices
    buf->Vertices.set_used(3);

    buf->Vertices[0].Pos.set(0,0,0);
    buf->Vertices[0].TCoords.set(0,0);

    buf->Vertices[1].Pos.set(0,100,-100);
    buf->Vertices[1].TCoords.set(0,0);

    buf->Vertices[2].Pos.set(0,100,100);
    buf->Vertices[2].TCoords.set(0,0);

    buf->Indices.set_used(3);
    //Create triangles by setting
    buf->Indices[0]=0;
    buf->Indices[1]=1;
    buf->Indices[2]=2;


    buf->recalculateBoundingBox();
    buf->drop();
    mesh->setDirty();
    mesh->recalculateBoundingBox();
    */

    radarScreen = smgr->addBillboardSceneNode();
    radarScreen->setMaterialFlag(video::EMF_LIGHTING, false);
    radarScreen->setSize(core::dimension2d<f32>(0.5f, 0.5f)); //FIXME: Hardcoded size
    this->parent = parent;
    this->offset = offset;
}

void RadarScreen::update(video::IImage* radarImage)
{
     //link camera rotation to shipNode
    // get transformation matrix of node
    core::matrix4 m;
    m.setRotationDegrees(parent->getRotation());

    // transform offset('offset' is relative to the local ship coordinates, and stays the same.)
    //'offsetTransformed' is transformed into the global coordinates
    core::vector3df offsetTransformed;
    m.transformVect(offsetTransformed,offset);

    //move screen
    radarScreen->setPosition(parent->getPosition() + offsetTransformed);

    //Get old texture if it exists
    video::ITexture* oldTexture = 0;
    if (radarScreen->getMaterialCount()>0) {
        oldTexture = radarScreen->getMaterial(0).getTexture(0);
    }
    //make texture from image and apply to the screen
    radarScreen->setMaterialTexture(0,driver->addTexture("RadarImage",radarImage));
    //Remove old texture if it exists
    if (oldTexture!=0) {
            driver->removeTexture(oldTexture);
    }

}

irr::scene::ISceneNode* RadarScreen::getSceneNode() const
{
    return radarScreen;
}

