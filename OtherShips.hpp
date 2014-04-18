#ifndef __OTHERSHIPS_HPP_INCLUDED__
#define __OTHERSHIPS_HPP_INCLUDED__

#include "irrlicht.h"

#include <vector>
#include <string>

//Forward declarations
class SimulationModel;
class OtherShip;

class OtherShips
{
    public:
        OtherShips();
        virtual ~OtherShips();
        void loadOtherShips(const std::string& scenarioOtherShipsFilename, irr::scene::ISceneManager* smgr, SimulationModel* model);

    private:
        std::vector<OtherShip> otherShips;
};

#endif
