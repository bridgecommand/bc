#include "NavLight.hpp"

using namespace irr;

NavLight::NavLight(irr::scene::ISceneNode* parent, irr::scene::ISceneManager* smgr, irr::core::dimension2d<f32> lightSize, irr::core::vector3df position, irr::video::SColor colour, irr::f32 lightStartAngle, irr::f32 lightEndAngle, irr::f32 lightRange, std::string lightSequence) {

    lightNode = smgr->addBillboardSceneNode(parent, lightSize, position);
    lightNode->setMaterialFlag(video::EMF_LIGHTING, false);
    //lightNode->setMaterialType(video::EMT_TRANSPARENT_ADD_COLOR);
    lightNode->setColor(colour);
    //lightNode->setMaterialTexture(0, driver->getTexture("/media/particlewhite.bmp"));

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

void NavLight::update(irr::f32 scenarioTime, irr::core::vector3df viewPosition) {

    //find light position
    lightNode->updateAbsolutePosition();//Fixme: This is needed, but seems odd that it's required
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
    if (!isAngleBetween(localRelativeAngleDeg,startAngle,endAngle)) {
        lightNode->setVisible(false);
    }

    //set light visibility depending on light sequence
    //find length of sequence
    int sequenceLength = sequence.length();// FIXME: Should be size type
    if (sequenceLength > 0) {
        f32 charTime = 0.25; //where each character represents 0.25s of time
        f32 timeInSequence = std::fmod(((scenarioTime+timeOffset) / charTime),sequenceLength);
        u32 positionInSequence = timeInSequence;
        if (positionInSequence>=sequenceLength) {positionInSequence = sequenceLength-1;} //Should not be required, but double check we're not off the end of the sequence
        if (sequence[positionInSequence] == 'D') {
            lightNode->setVisible(false);
        }
    }
}

bool NavLight::isAngleBetween(irr::f32 angle, irr::f32 startAng, irr::f32 endAng) {
    if(startAng < 0 || startAng > 360 || endAng < 0 || endAng > 720) {//Invalid angles
        return false;
    }

    //normalise angle
    angle = normaliseAngle(angle);

    //std::cout << angle << " " << startAng << " " << endAng << std::endl;

    if(endAngle <= 360) { //Simple case
        return (angle >= startAng && angle <=endAng);
    } else { //End angle > 360
        return (angle >= startAng || angle <= normaliseAngle(endAng));
    }
}

irr::f32 NavLight::normaliseAngle(irr::f32 angle) { //ensure angle is in range 0-360
    while (angle < 0) {
        angle+=360;
    }

    while (angle >= 360) {
        angle-=360;
    }

    return angle;
}
