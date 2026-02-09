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

#include "irrlicht.h"

#include <iostream>
#include <cmath> //For fmod()
#include <cstdlib> //For rand()

namespace {
    inline irr::core::vector3df toIrrVec(const bc::graphics::Vec3& v) {
        return irr::core::vector3df(v.x, v.y, v.z);
    }
    inline bc::graphics::Vec3 fromIrrVec(const irr::core::vector3df& v) {
        return bc::graphics::Vec3(v.X, v.Y, v.Z);
    }
    inline irr::video::SColor toIrrColor(const bc::graphics::Color& c) {
        return irr::video::SColor(c.a, c.r, c.g, c.b);
    }
}

NavLight::NavLight(irr::scene::ISceneNode* parent, irr::scene::ISceneManager* smgr, bc::graphics::Vec3 position, bc::graphics::Color colour, float lightStartAngle, float lightEndAngle, float lightRange, std::string lightSequence, uint32_t phaseStart) {

    //Store the scene manager, so we can find the active camera
    this->smgr = smgr;

    float lightSize = 0.5;
    if (parent && parent->getScale().X > 0) {
        lightSize /= parent->getScale().X; //Assume scale in all directions is the same
    }

    lightNode = smgr->addSphereSceneNode(lightSize,4,parent,-1, toIrrVec(position));

    smgr->getMeshManipulator()->setVertexColors(lightNode->getMesh(), toIrrColor(colour));

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
        timeOffset=60.0*((float)std::rand()/RAND_MAX); //Random, 0-60s
    } else {
        timeOffset=(phaseStart-1)*charTime;
    }

    //set initial alpha to implausible value
	currentAlpha = -1;
}

NavLight::~NavLight() {
    //TODO: Understand why NavLights are being created and destroyed during model set-up
}

bc::graphics::Vec3 NavLight::getPosition() const
{
    lightNode->updateAbsolutePosition();//ToDo: This may be needed, but seems odd that it's required
    return fromIrrVec(lightNode->getAbsolutePosition());
}

void NavLight::setPosition(bc::graphics::Vec3 position)
{
    lightNode->setPosition(toIrrVec(position));
}

void NavLight::update(float scenarioTime, uint32_t lightLevel) {

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
    float hFOV = 2*atan(tan(camera->getFOV()/2)*camera->getAspectRatio()); //Convert from VFOV to hFOV
    float zoom = hFOV / (irr::core::PI/2.0); //Zoom compared to standard 90 degree field of view

    //scale so lights appear same size independent of range
    float lightDistance=lightPosition.getDistanceFrom(viewPosition);
    lightNode->setScale(irr::core::vector3df(lightDistance*0.01*zoom,lightDistance*0.01*zoom,lightDistance*0.01*zoom));

    //set light visibility depending on range
    if (lightDistance > range) {
        lightNode->setVisible(false);
    } else {
        lightNode->setVisible(true);
    }

    //set light visibility depending on angle: Set to false if not visible
    float relativeAngleDeg = (viewPosition-lightPosition).getHorizontalAngle().Y; //Degrees: Angle from the light to viewpoint.
    float parentAngleDeg = lightNode->getParent()->getRotation().Y;
    float localRelativeAngleDeg = relativeAngleDeg-parentAngleDeg; //Angle from light to viewpoint, relative to light's parent coordinate system.
    if (!Angles::isAngleBetween(localRelativeAngleDeg,startAngle,endAngle)) {
        lightNode->setVisible(false);
    }

    //set light visibility depending on light sequence
    //find length of sequence
    std::string::size_type sequenceLength = sequence.length();
    if (sequenceLength > 0) {
        float timeInSequence = std::fmod(((scenarioTime+timeOffset) / charTime),sequenceLength);
        uint32_t positionInSequence = timeInSequence;
        if (positionInSequence>=sequenceLength) {positionInSequence = sequenceLength-1;} //Should not be required, but double check we're not off the end of the sequence
        if (sequence[positionInSequence] == 'D' || sequence[positionInSequence] == 'd') {
            lightNode->setVisible(false);
        }
    }

	//set transparency dependent on light level, only changing if required, as this is a slow operation
    uint16_t requiredAlpha = 255 - lightLevel;
	if (requiredAlpha != currentAlpha) {

		//setAlpha((uint8_t)requiredAlpha, lightTexture);
        smgr->getMeshManipulator()->setVertexColorAlpha(lightNode->getMesh(),requiredAlpha);
		currentAlpha = requiredAlpha;
	}

}

/*
bool NavLight::setAlpha(uint8_t alpha, irr::video::ITexture* tex)
//Modified from http://irrlicht.sourceforge.net/forum/viewtopic.php?t=31400
//FIXME: Check how the texture color format is set
{
	if (!tex)
	{
		return false;
	};

	uint32_t size = tex->getSize().Width*tex->getSize().Height;  // get Texture Size
	uint32_t width = tex->getSize().Width;
	uint32_t height = tex->getSize().Height;



	switch (tex->getColorFormat()) //getTexture Format, (nly 2 support alpha)
	{
	case irr::video::ECF_A1R5G5B5: //see video::ECOLOR_FORMAT for more information on the texture formats.
	{
		//  printf("16BIT\n");
		uint16_t* Data = (uint16_t*)tex->lock(); //get Data for 16-bit Texture
		for (uint32_t i = 0; i < width; i++) {
			for (uint32_t j = 0; j < height; j++) {

				float x = (int32_t)i - (int32_t)width / 2;
				float y = (int32_t)j - (int32_t)height / 2;
				float radSq = x*x + y*y;
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
		uint32_t* Data = (uint32_t*)tex->lock();

		irr::video::SColor pixelColor;

		for (uint32_t i = 0; i < width; i++) {
			for (uint32_t j = 0; j < height; j++) {

				float x = (int32_t)i - (int32_t)width / 2;
				float y = (int32_t)j - (int32_t)height / 2;
				float radSq = x*x + y*y;
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

void NavLight::moveNode(float deltaX, float deltaY, float deltaZ)
{
    irr::core::vector3df currentPos = lightNode->getPosition();
    float newPosX = currentPos.X + deltaX;
    float newPosY = currentPos.Y + deltaY;
    float newPosZ = currentPos.Z + deltaZ;

    lightNode->setPosition(irr::core::vector3df(newPosX,newPosY,newPosZ));
}
