#include "NavLight.hpp"

using namespace irr;

NavLight::NavLight(irr::scene::ISceneNode* parent, irr::scene::ISceneManager* smgr, irr::core::dimension2d<f32> lightSize, irr::core::vector3df position, irr::video::SColor colour) {

    //Need to add handling for startAngle, endAngle and range.
    lightNode = smgr->addBillboardSceneNode(parent, lightSize, position);
    lightNode->setMaterialFlag(video::EMF_LIGHTING, false);
    //lightNode->setMaterialType(video::EMT_TRANSPARENT_ADD_COLOR);
    lightNode->setColor(colour);
    //lightNode->setMaterialTexture(0, driver->getTexture("/media/particlewhite.bmp"));

}

NavLight::~NavLight() {
}

void NavLight::Update() {
}
