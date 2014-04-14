#ifndef __BUOYS_HPP_INCLUDED__
#define __BUOYS_HPP_INCLUDED__

#include "irrlicht.h"

#include <vector>

//Forward declarations
class SimulationModel;
class Buoy;

class Buoys
{
    public:
        Buoys();
        virtual ~Buoys();
        void loadBuoys(const irr::io::path& scenarioBuoyFilename, irr::scene::ISceneManager* smgr, SimulationModel* model);

    private:
        std::vector<Buoy> buoys;
};

#endif
