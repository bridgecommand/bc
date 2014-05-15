#include "NavLight.hpp"

using namespace irr;

NavLight::NavLight(irr::scene::ISceneNode* parent, irr::scene::ISceneManager* smgr, irr::core::dimension2d<f32> lightSize, irr::core::vector3df position, irr::video::SColor colour, irr::u32 lightStartAngle, irr::u32 lightEndAngle, irr::u32 lightRange) {

    lightNode = smgr->addBillboardSceneNode(parent, lightSize, position);
    lightNode->setMaterialFlag(video::EMF_LIGHTING, false);
    //lightNode->setMaterialType(video::EMT_TRANSPARENT_ADD_COLOR);
    lightNode->setColor(colour);
    //lightNode->setMaterialTexture(0, driver->getTexture("/media/particlewhite.bmp"));

    //store extra information
    startAngle = lightStartAngle;
    endAngle = lightEndAngle;
    range = lightRange;
}

NavLight::~NavLight() {
}

void NavLight::update(irr::f32 scenarioTime, irr::core::vector3df viewPosition) {
    //Need to add handling for startAngle, endAngle and range.

    f32 lightDistance=lightNode->getAbsolutePosition().getDistanceFrom(viewPosition);
    lightNode->setSize(core::dimension2df(lightDistance*0.01,lightDistance*0.01));

}
