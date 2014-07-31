#ifndef __BUOYS_HPP_INCLUDED__
#define __BUOYS_HPP_INCLUDED__

#include "irrlicht.h"

#include <vector>
#include <string>

//Forward declarations
class SimulationModel;
class Buoy;
class NavLight;
class RadarData;

class Buoys
{
    public:
        Buoys();
        virtual ~Buoys();
        void load(const std::string& worldName, irr::scene::ISceneManager* smgr, SimulationModel* model);
        void update(irr::f32 deltaTime, irr::f32 scenarioTime, irr::f32 tideHeight, irr::core::vector3df viewPosition, irr::u32 lightLevel);
        RadarData getRadarData(irr::u32 number, irr::core::vector3df scannerPosition) const;
        irr::u32 getNumber() const;

    private:
        std::vector<Buoy> buoys;
        std::vector<NavLight> buoysLights;
};

#endif
