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

#include "Sky.hpp"
#include "Constants.hpp"

//using namespace irr;

Sky::Sky(irr::scene::ISceneManager* smgr)
{
    irr::video::IVideoDriver* driver = smgr->getVideoDriver();
    driver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, false);
    skyNode=smgr->addSkyDomeSceneNode(driver->getTexture("media/Sky_horiz_9.jpg"),16,16,1.0f,1.05f,3.5*M_IN_NM); //Fixme: Range should probably be dependent on fog & camera range.
    driver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, true);
    skyNode->setMaterialFlag(irr::video::EMF_FOG_ENABLE, true);
    skyNode->setMaterialFlag(irr::video::EMF_LIGHTING, true); //Turn on lighting, so the sky gets dark as ambient light goes down

}

Sky::~Sky()
{
    //dtor
}
