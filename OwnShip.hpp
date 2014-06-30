#ifndef __OWNSHIP_HPP_INCLUDED__
#define __OWNSHIP_HPP_INCLUDED__

#include "irrlicht.h"

#include <vector>

#include "Ship.hpp"

//Forward declarations
class SimulationModel;

class OwnShip : public Ship
{
    public:

        void load(const std::string& scenarioName, irr::scene::ISceneManager* smgr, SimulationModel* model);
        void update(irr::f32 deltaTime);
        std::vector<irr::core::vector3df> getCameraViews() const;

    protected:
    private:
        std::vector<irr::core::vector3df> views; //The offset of the camera origin from the own ship origin
};

#endif // __OWNSHIP_HPP_INCLUDED__
