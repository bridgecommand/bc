#ifndef __STARTUPEVENTRECEIVER_HPP_INCLUDED__
#define __STARTUPEVENTRECEIVER_HPP_INCLUDED__

#include "irrlicht.h"

class StartupEventReceiver : public irr::IEventReceiver
{
public:

    StartupEventReceiver(irr::gui::IGUIListBox* scenarioListBox, irr::s32 listBoxID, irr::s32 okButtonID);
    bool OnEvent(const irr::SEvent& event);

    irr::s32 getScenarioSelected();
    //irr::s32 GetScrollBarPosSpeed() const;
    //irr::s32 GetScrollBarPosHeading() const;

private:

    irr::gui::IGUIListBox* scenarioListBox;
    irr::s32 listBoxID;
    irr::s32 okButtonID;
    irr::s32 scenarioSelected;

};

#endif

