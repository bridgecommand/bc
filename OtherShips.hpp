#ifndef __OTHERSHIPS_HPP_INCLUDED__
#define __OTHERSHIPS_HPP_INCLUDED__

#include "irrlicht.h"

#include <vector>
#include <string>

#include "Leg.hpp"

//Forward declarations
class SimulationModel;
class OtherShip;

class OtherShips
{
    public:
        OtherShips();
        virtual ~OtherShips();
        void load(const std::string& scenarioOtherShipsFilename, irr::scene::ISceneManager* smgr, SimulationModel* model);
        void update(irr::f32 deltaTime);

    private:
        std::vector<OtherShip> otherShips;
};

#endif
