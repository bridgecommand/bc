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

void Rain::load(irr::scene::ISceneManager* smgr, irr::scene::ISceneNode* parent, irr::IrrlichtDevice* dev)
{
    //Make rain
    this->parent = parent;
    irr::video::IVideoDriver* driver = smgr->getVideoDriver();

    rainIntensity = 0.0;
    //Load rain.x, flip vertexes, and load rain1.jpg texture
    irr::scene::IMesh* rainMesh = smgr->getMesh("media/rain.x");
    //add to scene node
    irr::scene::IMeshManipulator* meshManipulator = smgr->getMeshManipulator();
    if (rainMesh!=0) {
        //meshManipulator->scale(rainMesh,irr::core::vector3df(5.0,5.0,5.0)); //Scale mesh - ToDo: Make this dependent on ship/bridge size
        meshManipulator->flipSurfaces(rainMesh);
        rainNode1 = smgr->addMeshSceneNode( rainMesh);
        rainNode2 = smgr->addMeshSceneNode( rainMesh);
        rainNode1->setScale(irr::core::vector3df(5.0,5.0,5.0));
        rainNode2->setScale(irr::core::vector3df(6.0,5.0,6.0));
    } else {
        //Failed to load mesh - load with dummy and continue - ToDo: should also flag this up to user
        dev->getLogger()->log("Failed to load rain mesh (rain.x)");
        rainNode1 = smgr->addEmptySceneNode();
        rainNode2 = smgr->addEmptySceneNode();
    }

    //set texture
    irr::video::ITexture* texture;
    std::vector<irr::io::path> textureNames;
    textureNames.push_back("./media/rain_0.png");
    textureNames.push_back("./media/rain_1.png");
    textureNames.push_back("./media/rain_2.png");
    textureNames.push_back("./media/rain_3.png");
    textureNames.push_back("./media/rain_4.png");
    textureNames.push_back("./media/rain_5.png");
    textureNames.push_back("./media/rain_6.png");
    textureNames.push_back("./media/rain_7.png");
    textureNames.push_back("./media/rain_8.png");
    textureNames.push_back("./media/rain_9.png");
    textureNames.push_back("./media/rain_10.png");

    for(std::vector<irr::io::path>::iterator it = textureNames.begin(); it != textureNames.end(); ++it) {
        texture = driver->getTexture(*it);
        if (texture!=0) {
            rainTextures.push_back(texture);
        }
    }

    rainNode1->setMaterialType(irr::video::EMT_TRANSPARENT_ALPHA_CHANNEL  );
    rainNode2->setMaterialType(irr::video::EMT_TRANSPARENT_ALPHA_CHANNEL  );
    applyTextures();

}

void Rain::setIntensity(irr::f32 intensity) {

    if (intensity != rainIntensity && intensity <= 10 && intensity >= 0) {

        rainIntensity = intensity;
        applyTextures();
    }
}

void Rain::applyTextures() {
    if (rainTextures.size()==11) { //Check all textures 0-10 are loaded
        //Round one up and one down so we can get half step rain intensity level.
    	irr::u8 texture1 = Utilities::round(rainIntensity+0.25);
    	irr::u8 texture2 = Utilities::round(rainIntensity-0.25);
        rainNode1->setMaterialTexture(0,rainTextures.at(texture1));
        rainNode2->setMaterialTexture(0,rainTextures.at(texture2));
    }
}

void Rain::update(irr::f32 scenarioTime) {
    //update rain animation
        //(Should move to its own class)
        irr::f32 rainAnimation1 = scenarioTime/2.0;
        irr::f32 rainAnimation2 = scenarioTime/2.2;
        rainAnimation1 = rainAnimation1 - (int)rainAnimation1;
        rainAnimation2 = rainAnimation2 - (int)rainAnimation2;
        rainNode1->getMaterial(0).getTextureMatrix(0).setTextureTranslate(0.5,rainAnimation1);
        rainNode2->getMaterial(0).getTextureMatrix(0).setTextureTranslate(0.5,rainAnimation2);

        //update position - this isn't actually set as a child node, as we want position to update, but not rotation
        rainNode1->setPosition(parent->getPosition());
        rainNode2->setPosition(parent->getPosition());
        //End rain
}
