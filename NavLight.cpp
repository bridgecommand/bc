#include "NavLight.hpp"
#include "Angles.hpp"

#include <iostream>

using namespace irr;

NavLight::NavLight(irr::scene::ISceneNode* parent, irr::scene::ISceneManager* smgr, irr::core::dimension2d<f32> lightSize, irr::core::vector3df position, irr::video::SColor colour, irr::f32 lightStartAngle, irr::f32 lightEndAngle, irr::f32 lightRange, std::string lightSequence) {

    lightNode = smgr->addBillboardSceneNode(parent, lightSize, position);
    lightNode->setColor(colour);
    lightNode->setMaterialTexture(0, smgr->getVideoDriver()->getTexture("media/particlewhite.png"));
    lightNode->setMaterialType(video::EMT_TRANSPARENT_ALPHA_CHANNEL);
    lightNode->setMaterialFlag(video::EMF_LIGHTING, false);

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
    sequence = lightSequence;
    timeOffset=60.0*((irr::f32)std::rand()/RAND_MAX); //Random, 0-60s
}

NavLight::~NavLight() {
}

void NavLight::update(irr::f32 scenarioTime, irr::core::vector3df viewPosition, irr::u32 lightLevel) {

    //find light position
    lightNode->updateAbsolutePosition(); //ToDo: This is needed, but seems odd that it's required
    core::vector3df lightPosition = lightNode->getAbsolutePosition();

    //std::cout << "Light pos: " << lightPosition.X << "," << lightPosition.Y << "," << lightPosition.Z << std::endl;
    //std::cout << "View pos: " << viewPosition.X << "," << viewPosition.Z << std::endl;

    //scale so lights appear same size independent of range
    f32 lightDistance=lightPosition.getDistanceFrom(viewPosition);
    lightNode->setSize(core::dimension2df(lightDistance*0.01,lightDistance*0.01));

    //set light visibility depending on range
    if (lightDistance > range) {
        lightNode->setVisible(false);
    } else {
        lightNode->setVisible(true);
    }

    //set light visibility depending on angle: Set to false if not visible
    f32 relativeAngleDeg = (viewPosition-lightPosition).getHorizontalAngle().Y; //Degrees: Angle from the light to viewpoint.
    f32 parentAngleDeg = lightNode->getParent()->getRotation().Y;
    f32 localRelativeAngleDeg = relativeAngleDeg-parentAngleDeg; //Angle from light to viewpoint, relative to light's parent coordinate system.
    //std::cout << relativeAngleDeg << " " <<  parentAngleDeg << " " << localRelativeAngleDeg <<std::endl;
    if (!Angles::isAngleBetween(localRelativeAngleDeg,startAngle,endAngle)) {
        lightNode->setVisible(false);
    }

    //set light visibility depending on light sequence
    //find length of sequence
    std::string::size_type sequenceLength = sequence.length();
    if (sequenceLength > 0) {
        f32 charTime = 0.25; //where each character represents 0.25s of time
        f32 timeInSequence = std::fmod(((scenarioTime+timeOffset) / charTime),sequenceLength);
        u32 positionInSequence = timeInSequence;
        if (positionInSequence>=sequenceLength) {positionInSequence = sequenceLength-1;} //Should not be required, but double check we're not off the end of the sequence
        if (sequence[positionInSequence] == 'D') {
            lightNode->setVisible(false);
        }
    }

    //set transparency dependent on light level
    //std::cout << lightLevel << std::endl;
    setAlpha(255-lightLevel, lightNode->getMaterial(0).getTexture(0));
}

bool NavLight::setAlpha(irr::u8 alpha, irr::video::ITexture* tex)
//Modified from http://irrlicht.sourceforge.net/forum/viewtopic.php?t=31400
//FIXME: Check how the texture color format is set
{
    if(!tex)
    {
        return false;
    };

    u32 size = tex->getSize().Width*tex->getSize().Height;  // get Texture Size

    switch(tex->getColorFormat()) //getTexture Format, (nly 2 support alpha)
    {
        case video::ECF_A1R5G5B5: //see video::ECOLOR_FORMAT for more information on the texture formats.
        {
          //  printf("16BIT\n");
            u16* Data = (u16*)tex->lock(); //get Data for 16-bit Texture
            for(u32 i = 0; i < size ; i++)
            {
                u8 alphaToUse = (u8)video::getAlpha(Data[i])==0 ? 0 : alpha; //If already transparent, leave as-is
                Data[i] = video::RGBA16(video::getRed(Data[i]), video::getGreen(Data[i]), video::getBlue(Data[i]), alphaToUse);
            }
            tex->unlock();
            break;
        };
        case video::ECF_A8R8G8B8:
        {
            u32* Data = (u32*)tex->lock();
            for( u32 i = 0 ; i < size ; i++)
            {
                //u8 minAlpha = std::min(((u8*)&Data[i])[3],alpha);
                u8 alphaToUse = ((u8*)&Data[i])[3] == 0 ? 0 : alpha; //If already transparent, leave as-is
                ((u8*)&Data[i])[3] = alphaToUse;//get Data for 32-bit Texture
            }
            tex->unlock();
            break;
        };
        default:
            return false;
    };
    return true;
}

void NavLight::moveNode(irr::f32 deltaX, irr::f32 deltaY, irr::f32 deltaZ)
{
    core::vector3df currentPos = lightNode->getPosition();
    irr::f32 newPosX = currentPos.X + deltaX;
    irr::f32 newPosY = currentPos.Y + deltaY;
    irr::f32 newPosZ = currentPos.Z + deltaZ;

    lightNode->setPosition(core::vector3df(newPosX,newPosY,newPosZ));
}
