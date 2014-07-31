#ifndef __OWNSHIP_HPP_INCLUDED__
#define __OWNSHIP_HPP_INCLUDED__

#include "irrlicht.h"

#include <vector>

#include "Ship.hpp"
#include "Terrain.hpp"

//Forward declarations
class SimulationModel;

class OwnShip : public Ship
{
    public:

        void load(const std::string& scenarioName, irr::scene::ISceneManager* smgr, SimulationModel* model, Terrain* terrain);
        void update(irr::f32 deltaTime, irr::f32 tideHeight);
        std::vector<irr::core::vector3df> getCameraViews() const;
        irr::f32 getDepth();

    protected:
    private:
        std::vector<irr::core::vector3df> views; //The offset of the camera origin from the own ship origin
        Terrain* terrain;
};

#endif // __OWNSHIP_HPP_INCLUDED__
