#ifndef __OWNSHIP_HPP_INCLUDED__
#define __OWNSHIP_HPP_INCLUDED__

#include "irrlicht.h"

//Forward declarations
class SimulationModel;

class OwnShip
{
    public:
        OwnShip();
        virtual ~OwnShip();

        void loadModel(const std::string& scenarioOwnShipFilename, irr::f32& xPos, irr::f32& yPos, irr::f32& zPos, irr::scene::ISceneManager* smgr, SimulationModel* model);
        irr::scene::IMeshSceneNode* getSceneNode();

        void setPosition(irr::core::vector3df position);
        void setRotation(irr::core::vector3df rotation);
        irr::core::vector3df getRotation();
        irr::core::vector3df getPosition();
        irr::core::vector3df getCameraOffset();

    protected:
    private:
        irr::scene::IMeshSceneNode* ownShip; //The scene node for the own ship.
        irr::core::vector3df cameraOffset; //The offset of the camera origin from the own ship origin
};

#endif // __OWNSHIP_HPP_INCLUDED__
