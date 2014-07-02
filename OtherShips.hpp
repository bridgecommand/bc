#ifndef __OTHERSHIPS_HPP_INCLUDED__
#define __OTHERSHIPS_HPP_INCLUDED__

#include "irrlicht.h"

#include <vector>
#include <string>

#include "Leg.hpp"

//Forward declarations
class SimulationModel;
class OtherShip;
class RadarData;

class OtherShips
{
    public:
        OtherShips();
        virtual ~OtherShips();
        void load(const std::string& scenarioName, irr::f32 scenarioStartTime, irr::scene::ISceneManager* smgr, SimulationModel* model);
        void update(irr::f32 deltaTime, irr::f32 scenarioTime, irr::core::vector3df viewPosition, irr::u32 lightLevel);
        RadarData getRadarData(irr::u32 number, irr::core::vector3df scannerPosition) const;
        irr::u32 getNumber() const;

    private:
        std::vector<OtherShip> otherShips;
};

#endif
