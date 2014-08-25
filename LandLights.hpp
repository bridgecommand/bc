#ifndef __LANDLIGHTS_HPP_INCLUDED__
#define __LANDLIGHTS_HPP_INCLUDED__

#include "irrlicht.h"

#include <vector>
#include <string>

//Forward declarations
class SimulationModel;
class NavLight;
class Terrain;

class LandLights
{
    public:
        LandLights();
        virtual ~LandLights();
        void load(const std::string& worldName, irr::scene::ISceneManager* smgr, SimulationModel* model, const Terrain& terrain);
        void update(irr::f32 deltaTime, irr::f32 scenarioTime, irr::core::vector3df viewPosition, irr::u32 lightLevel);
        irr::u32 getNumber() const;
        void moveNode(irr::f32 deltaX, irr::f32 deltaY, irr::f32 deltaZ);
    private:
        std::vector<NavLight> landLights;
};

#endif

