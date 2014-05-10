#ifndef __BUOYS_HPP_INCLUDED__
#define __BUOYS_HPP_INCLUDED__

#include "irrlicht.h"

#include <vector>
#include <string>

//Forward declarations
class SimulationModel;
class Buoy;

class Buoys
{
    public:
        Buoys();
        virtual ~Buoys();
        void load(const std::string& scenarioBuoyFilename, irr::scene::ISceneManager* smgr, SimulationModel* model);

    private:
        std::vector<Buoy> buoys;
};

#endif
