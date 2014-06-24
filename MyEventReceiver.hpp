#ifndef __MYEVENTRECEIVER_HPP_INCLUDED__
#define __MYEVENTRECEIVER_HPP_INCLUDED__

#include "irrlicht.h"

//forward declarations
class GUIMain;
class SimulationModel;

class MyEventReceiver : public irr::IEventReceiver
{
public:

    MyEventReceiver(SimulationModel* model, GUIMain* gui);

    bool OnEvent(const irr::SEvent& event);
    irr::s32 GetScrollBarPosSpeed() const;
    irr::s32 GetScrollBarPosHeading() const;

private:

    SimulationModel* model;
    GUIMain* gui;
    irr::s32 scrollBarPosSpeed;
    irr::s32 scrollBarPosHeading;

};

#endif
