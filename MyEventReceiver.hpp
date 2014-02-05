#ifndef __MYEVENTRECEIVER_HPP_INCLUDED__
#define __MYEVENTRECEIVER_HPP_INCLUDED__

#include "irrlicht.h"

#include "GUIMain.hpp"
#include "SimulationModel.hpp"

using namespace irr;

class MyEventReceiver : public IEventReceiver
{
public:

    MyEventReceiver(SimulationModel* mdl, GUIMain* gui);

    virtual bool OnEvent(const SEvent& event);
    virtual s32 GetScrollBarPosSpeed() const;
    virtual s32 GetScrollBarPosHeading() const;

private:

    SimulationModel* model;
    GUIMain* guiMain;
    s32 scrollBarPosSpeed;
    s32 scrollBarPosHeading;

};

#endif
