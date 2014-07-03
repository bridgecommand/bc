#ifndef __LANDOBJECTS_HPP_INCLUDED__
#define __LANDOBJECTS_HPP_INCLUDED__

#include "irrlicht.h"

#include <vector>
#include <string>

//Forward declarations
class SimulationModel;
class Terrain;
class LandObject;

class LandObjects
{
    public:
        LandObjects();
        virtual ~LandObjects();
        void load(const std::string& worldName, irr::scene::ISceneManager* smgr, SimulationModel* model, const Terrain& terrain);
        irr::u32 getNumber() const;

    private:
        std::vector<LandObject> landObjects;
};

#endif
