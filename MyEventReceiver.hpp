#ifndef __MYEVENTRECEIVER_HPP_INCLUDED__
#define __MYEVENTRECEIVER_HPP_INCLUDED__

#include "irrlicht.h"

//forward declarations
class GUIMain;
class SimulationModel;

class MyEventReceiver : public irr::IEventReceiver
{
public:

    MyEventReceiver(SimulationModel* mdl, GUIMain* gui);

    virtual bool OnEvent(const irr::SEvent& event);
    virtual const irr::s32 GetScrollBarPosSpeed();
    virtual const irr::s32 GetScrollBarPosHeading();

private:

    SimulationModel* model;
    GUIMain* guiMain;
    irr::s32 scrollBarPosSpeed;
    irr::s32 scrollBarPosHeading;

};

#endif
