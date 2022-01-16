/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2014 James Packer

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License version 2 as
     published by the Free Software Foundation

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY Or FITNESS For A PARTICULAR PURPOSE.  See the
     GNU General Public License For more details.

     You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA. */

#ifndef __MYEVENTRECEIVER_HPP_INCLUDED__
#define __MYEVENTRECEIVER_HPP_INCLUDED__

#include "irrlicht.h"
#include <string>
#include <vector>

//forward declarations
class GUIMain;
class SimulationModel;

//Data about joystick setup
class JoystickSetup {
public:
    irr::u32 portJoystickAxis;
    irr::u32 stbdJoystickAxis;
    irr::u32 rudderJoystickAxis;
    irr::u32 bowThrusterJoystickAxis;
    irr::u32 sternThrusterJoystickAxis;
    irr::u32 portJoystickNo;
    irr::u32 stbdJoystickNo;
    irr::u32 rudderJoystickNo;
    irr::u32 bowThrusterJoystickNo;
    irr::u32 sternThrusterJoystickNo;
    std::vector<irr::f32> inputPoints;
    std::vector<irr::f32> outputPoints;
    irr::s32 rudderDirection;
    //Buttons:
    irr::u32 joystickNoHorn;
    irr::u32 joystickButtonHorn;
    irr::u32 joystickNoChangeView;
    irr::u32 joystickButtonChangeView;
    irr::u32 joystickNoLookStepLeft;
    irr::u32 joystickButtonLookStepLeft;
    irr::u32 joystickNoLookStepRight;
    irr::u32 joystickButtonLookStepRight;
    irr::u32 joystickNoIncreaseBowThrust;
    irr::u32 joystickButtonIncreaseBowThrust;
    irr::u32 joystickNoDecreaseBowThrust;
    irr::u32 joystickButtonDecreaseBowThrust;
    irr::u32 joystickNoIncreaseSternThrust;
    irr::u32 joystickButtonIncreaseSternThrust;
    irr::u32 joystickNoDecreaseSternThrust;
    irr::u32 joystickButtonDecreaseSternThrust;
    irr::u32 joystickNoBearingOn;
    irr::u32 joystickButtonBearingOn;
    irr::u32 joystickNoBearingOff;
    irr::u32 joystickButtonBearingOff;
    irr::u32 joystickNoZoomOn;
    irr::u32 joystickButtonZoomOn;
    irr::u32 joystickNoZoomOff;
    irr::u32 joystickButtonZoomOff;
    irr::u32 joystickNoLookLeft;
    irr::u32 joystickButtonLookLeft;
    irr::u32 joystickNoLookRight;
    irr::u32 joystickButtonLookRight;
    irr::u32 joystickNoLookUp;
    irr::u32 joystickButtonLookUp;
    irr::u32 joystickNoLookDown;
    irr::u32 joystickButtonLookDown;
    irr::u32 joystickNoPump1On;
    irr::u32 joystickButtonPump1On;
    irr::u32 joystickNoPump1Off;
    irr::u32 joystickButtonPump1Off;
    irr::u32 joystickNoPump2On;
    irr::u32 joystickButtonPump2On;
    irr::u32 joystickNoPump2Off;
    irr::u32 joystickButtonPump2Off;
    irr::u32 joystickNoFollowUpOn;
    irr::u32 joystickButtonFollowUpOn;
    irr::u32 joystickNoFollowUpOff;
    irr::u32 joystickButtonFollowUpOff;
};

class MyEventReceiver : public irr::IEventReceiver
{
public:

    MyEventReceiver(irr::IrrlichtDevice* dev, SimulationModel* model, GUIMain* gui, JoystickSetup joystickSetup, std::vector<std::string>* logMessages);

    bool OnEvent(const irr::SEvent& event);
    //irr::s32 GetScrollBarPosSpeed() const;
    //irr::s32 GetScrollBarPosHeading() const;

private:

    SimulationModel* model;
    GUIMain* gui;
    bool leftMouseDown;
    bool rightMouseDown;
    irr::IrrlichtDevice* device;
    irr::s32 scrollBarPosSpeed;
    irr::s32 scrollBarPosHeading;
    irr::core::array<irr::SJoystickInfo> joystickInfo;

    irr::f32 previousJoystickPort;
    irr::f32 previousJoystickStbd;
    irr::f32 previousJoystickRudder;
    irr::f32 previousJoystickBowThruster;
    irr::f32 previousJoystickSternThruster;

    irr::s32 mouseClickX;
    irr::s32 mouseClickY;

    JoystickSetup joystickSetup;
    std::vector<irr::u32> joystickPreviousButtonStates;
    std::vector<std::string>* logMessages;
    bool shutdownDialogActive;
    irr::u32 lastShownJoystickStatus;

    void startShutdown();
    irr::f32 lookup1D(irr::f32 lookupValue, std::vector<irr::f32> inputPoints, std::vector<irr::f32> outputPoints);
    std::wstring f32To3dp(irr::f32 value, bool stripZeros=false);
    bool IsButtonPressed(irr::u32 button, irr::u32 buttonBitmap) const;
};

#endif
