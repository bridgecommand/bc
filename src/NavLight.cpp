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

#include "NavLight.hpp"
#include "Angles.hpp"

#include <iostream>
#include <cmath> //For fmod()
#include <cstdlib> //For rand()

//using namespace irr;

NavLight::NavLight(irr::scene::ISceneNode* parent, irr::scene::ISceneManager* smgr, irr::core::vector3df position, irr::video::SColor colour, irr::f32 lightStartAngle, irr::f32 lightEndAngle, irr::f32 lightRange, std::string lightSequence, irr::u32 phaseStart) {

    //Store the scene manager, so we can find the active camera
    this->smgr = smgr;

    irr::f32 lightSize = 0.5;
    if (parent && parent->getScale().X > 0) {
        lightSize /= parent->getScale().X; //Assume scale in all directions is the same
    }

    lightNode = smgr->addSphereSceneNode(lightSize,4,parent,-1, position);

    smgr->getMeshManipulator()->setVertexColors(lightNode->getMesh(),colour);

    lightNode->setMaterialType(irr::video::EMT_TRANSPARENT_VERTEX_ALPHA);
	lightNode->setMaterialFlag(irr::video::EMF_LIGHTING, false);
	lightNode->setMaterialFlag(irr::video::EMF_FOG_ENABLE, true);

    //Fix angles if start is negative
    while (lightStartAngle < 0) {
        lightStartAngle +=360;
        lightEndAngle +=360;
    }

    //store extra information
    startAngle = lightStartAngle;
    endAngle = lightEndAngle;
    range = lightRange;

    //initialise light sequence information
    charTime = 0.25; //where each character represents 0.25s of time
    sequence = lightSequence;
    if (phaseStart==0) {
        timeOffset=60.0*((irr::f32)std::rand()/RAND_MAX); //Random, 0-60s
    } else {
        timeOffset=(phaseStart-1)*charTime;
    }

    //set initial alpha to implausible value
	currentAlpha = -1;
}

NavLight::~NavLight() {
    //TODO: Understand why NavLights are being created and destroyed during model set-up
}

irr::core::vector3df NavLight::getPosition() const
{
    lightNode->updateAbsolutePosition();//ToDo: This may be needed, but seems odd that it's required
    return lightNode->getAbsolutePosition();
}

void NavLight::setPosition(irr::core::vector3df position)
{
    lightNode->setPosition(position);
}

void NavLight::update(irr::f32 scenarioTime, irr::u32 lightLevel) {

    //FIXME: Remove viewPosition being passed in (now from Camera), and check if camera is null.

    //find light position
    lightNode->updateAbsolutePosition(); //ToDo: This is needed, but seems odd that it's required
    irr::core::vector3df lightPosition = lightNode->getAbsolutePosition();

    //Find the active camera
    irr::scene::ICameraSceneNode* camera = smgr->getActiveCamera();
    if (camera == 0) {
        return; //If we don't know where the camera is, we can't update lights etc, so give up here.
    }
    camera->updateAbsolutePosition();
    irr::core::vector3df viewPosition = camera->getAbsolutePosition();

    //find the HFOV
    irr::f32 hFOV = 2*atan(tan(camera->getFOV()/2)*camera->getAspectRatio()); //Convert from VFOV to hFOV
    irr::f32 zoom = hFOV / (irr::core::PI/2.0); //Zoom compared to standard 90 degree field of view

    //scale so lights appear same size independent of range
    irr::f32 lightDistance=lightPosition.getDistanceFrom(viewPosition);
    lightNode->setScale(irr::core::vector3df(lightDistance*0.01*zoom,lightDistance*0.01*zoom,lightDistance*0.01*zoom));

    //set light visibility depending on range
    if (lightDistance > range) {
        lightNode->setVisible(false);
    } else {
        lightNode->setVisible(true);
    }

    //set light visibility depending on angle: Set to false if not visible
    irr::f32 relativeAngleDeg = (viewPosition-lightPosition).getHorizontalAngle().Y; //Degrees: Angle from the light to viewpoint.
    irr::f32 parentAngleDeg = lightNode->getParent()->getRotation().Y;
    irr::f32 localRelativeAngleDeg = relativeAngleDeg-parentAngleDeg; //Angle from light to viewpoint, relative to light's parent coordinate system.
    if (!Angles::isAngleBetween(localRelativeAngleDeg,startAngle,endAngle)) {
        lightNode->setVisible(false);
    }

    //set light visibility depending on light sequence
    //find length of sequence
    std::string::size_type sequenceLength = sequence.length();
    if (sequenceLength > 0) {
        irr::f32 timeInSequence = std::fmod(((scenarioTime+timeOffset) / charTime),sequenceLength);
        irr::u32 positionInSequence = timeInSequence;
        if (positionInSequence>=sequenceLength) {positionInSequence = sequenceLength-1;} //Should not be required, but double check we're not off the end of the sequence
        if (sequence[positionInSequence] == 'D' || sequence[positionInSequence] == 'd') {
            lightNode->setVisible(false);
        }
    }

	//set transparency dependent on light level, only changing if required, as this is a slow operation
    irr::u16 requiredAlpha = 255 - lightLevel;
	if (requiredAlpha != currentAlpha) {

		//setAlpha((irr::u8)requiredAlpha, lightTexture);
        smgr->getMeshManipulator()->setVertexColorAlpha(lightNode->getMesh(),requiredAlpha);
		currentAlpha = requiredAlpha;
	}

}

/*
bool NavLight::setAlpha(irr::u8 alpha, irr::video::ITexture* tex)
//Modified from http://irrlicht.sourceforge.net/forum/viewtopic.php?t=31400
//FIXME: Check how the texture color format is set
{
	if (!tex)
	{
		return false;
	};

	irr::u32 size = tex->getSize().Width*tex->getSize().Height;  // get Texture Size
	irr::u32 width = tex->getSize().Width;
	irr::u32 height = tex->getSize().Height;

	

	switch (tex->getColorFormat()) //getTexture Format, (nly 2 support alpha)
	{
	case irr::video::ECF_A1R5G5B5: //see video::ECOLOR_FORMAT for more information on the texture formats.
	{
		//  printf("16BIT\n");
		irr::u16* Data = (irr::u16*)tex->lock(); //get Data for 16-bit Texture
		for (irr::u32 i = 0; i < width; i++) {
			for (irr::u32 j = 0; j < height; j++) {
				
				irr::f32 x = (irr::s32)i - (irr::s32)width / 2;
				irr::f32 y = (irr::s32)j - (irr::s32)height / 2;
				irr::f32 radSq = x*x + y*y;
				if (radSq <= width*width / 4) {
					Data[i + j*width] = irr::video::RGBA16(255, 255, 255, alpha);
				} else {
					Data[i + j*width] = irr::video::RGBA16(255, 255, 255, 0);
				}
				 

			}
		}
		tex->unlock();
		break;
	};
	case irr::video::ECF_A8R8G8B8:
	{
		irr::u32* Data = (irr::u32*)tex->lock();
		
		irr::video::SColor pixelColor;

		for (irr::u32 i = 0; i < width; i++) {
			for (irr::u32 j = 0; j < height; j++) {
				
				irr::f32 x = (irr::s32)i - (irr::s32)width / 2;
				irr::f32 y = (irr::s32)j - (irr::s32)height / 2;
				irr::f32 radSq = x*x + y*y;
				if (radSq <= width*width / 4) {
					pixelColor.set(alpha, 255, 255, 255);
				}
				else {
					pixelColor.set(0, 255, 255, 255);
				}
				
				Data[i + j*width] = pixelColor.color;
			}
		}
		
			//u8 alphaToUse = ((u8*)&Data[i])[3] == 0 ? 0 : alpha; //If already transparent, leave as-is
			//((u8*)&Data[i])[3] = alphaToUse;//get Data for 32-bit Texture
			
		tex->unlock();
		break;
	};
	default:
		return false;
	};
	return true;
}
*/

void NavLight::moveNode(irr::f32 deltaX, irr::f32 deltaY, irr::f32 deltaZ)
{
    irr::core::vector3df currentPos = lightNode->getPosition();
    irr::f32 newPosX = currentPos.X + deltaX;
    irr::f32 newPosY = currentPos.Y + deltaY;
    irr::f32 newPosZ = currentPos.Z + deltaZ;

    lightNode->setPosition(irr::core::vector3df(newPosX,newPosY,newPosZ));
}
