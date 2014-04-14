#ifndef __BUOYS_HPP_INCLUDED__
#define __BUOYS_HPP_INCLUDED__

#include "irrlicht.h"

#include <vector>

#include "Buoy.hpp"

class Buoys
{
    public:
        Buoys();
        virtual ~Buoys();
        void loadBuoys(const irr::io::path& scenarioBuoyFilename, irr::scene::ISceneManager* smgr);

    private:
        std::vector<Buoy> buoys;
        const irr::f32 longToX(irr::f32 longitude);
        const irr::f32 latToZ(irr::f32 latitude);
};

#endif
