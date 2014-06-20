#ifndef __SKY_HPP_INCLUDED__
#define __SKY_HPP_INCLUDED__

#include "irrlicht.h"

class Sky
{
    public:
        Sky(irr::scene::ISceneManager* smgr);
        virtual ~Sky();

    private:
        irr::scene::ISceneNode* skyNode;
};

#endif


