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
    
    
// DEE 10JAN23 Azimuth Drive Specific vvvv
// for class JoystickSetup

// which joysticks control each lever and control, in the case of azimuth drives the specific controls are     
//    irr::u32 azimuth1JoystickNo;
//    irr::u32 azimuth2JoystickNo;
    irr::u32 portThrustLever_joystickNo;
    irr::u32 stbdThrustLever_joystickNo;
    irr::u32 portSchottel_joystickNo;
    irr::u32 stbdSchottel_joystickNo;

// defines the channel aka axis of the already defined joystick no    
//    irr::u32 azimuth1JoystickAxis;
//    irr::u32 azimuth2JoystickAxis;
    irr::u32 portSchottel_channel;    
    irr::u32 stbdSchottel_channel;
    irr::u32 portThrustLever_channel;
    irr::u32 stbdThrustLever_channel;

// determines if the schottel is inverted
//    irr::s32 azimuth1Direction;
//    irr::s32 azimuth2Direction;
    irr::s32 schottelPortDirection;
    irr::s32 schottelStbdDirection;    
    irr::s32 thrustLeverPortDirection;
    irr::s32 thrustLeverStbdDirection;

// scaling and offset values for the schottel    
//    irr::f32 azimuth1Scaling;
//    irr::f32 azimuth2Scaling;
//    irr::f32 azimuth1Offset;
//    irr::f32 azimuth2Offset;
    irr::f32 schottelPortScaling;
    irr::f32 schottelStbdScaling;
    irr::f32 schottelPortOffset;
    irr::f32 schottelStbdOffset;
    
// scaling and offset for the thrust levers
    irr::f32 thrustLeverPortScaling;
    irr::f32 thrustLeverStbdScaling;
    irr::f32 thrustLeverPortOffset;
    irr::f32 thrustLeverStbdOffset;
    
    
// DEE 10JAN2023 ^^^^





//Buttons:
    irr::u32 joystickNoHorn;
    irr::u32 joystickButtonHorn;
    irr::u32 joystickNoChangeView;
    irr::u32 joystickButtonChangeView;
    irr::u32 joystickNoChangeAndLockView;
    irr::u32 joystickButtonChangeAndLockView;
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
    irr::u32 joystickNoNFUPort;
    irr::u32 joystickButtonNFUPort;
    irr::u32 joystickNoNFUStbd;
    irr::u32 joystickButtonNFUStbd;
    irr::u32 joystickNoAckAlarm;
    irr::u32 joystickButtonAckAlarm;
    // DEE 10JAN23 vvvv
    irr::u32 joystickButtonAzimuth1Master;
    irr::u32 joystickButtonAzimuth2Master;
    // DEE 10JAN23 ^^^^
    irr::u32 joystickNoAzimuth1Master;
    irr::u32 joystickNoAzimuth2Master;
    irr::u32 joystickNoPOV;
    irr::u16 joystickPOVLookLeft;
    irr::u16 joystickPOVLookRight;
    irr::u16 joystickPOVLookUp;
    irr::u16 joystickPOVLookDown;
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
// DEE 10JAN23 vvvv
//    irr::f32 previousJoystickAzimuthAngPort;
//    irr::f32 previousJoystickAzimuthAngStbd;

    irr::f32 previousJoystickThrustLeverPort;
    irr::f32 previousJoystickThrustLeverStbd;
    irr::f32 newJoystickThrustLeverPort;
    irr::f32 newJoystickThrustLeverStbd;

    irr::f32 previousJoystickSchottelPort;
    irr::f32 previousJoystickSchottelStbd;
    irr::f32 newJoystickSchottelPort;
    irr::f32 newJoystickSchottelStbd;

// DEE 10JAN23 ^^^^
    irr::f32 previousJoystickBowThruster;
    irr::f32 previousJoystickSternThruster;

    irr::s32 mouseClickX;
    irr::s32 mouseClickY;

    JoystickSetup joystickSetup;
    std::vector<irr::u32> joystickPreviousButtonStates;
    std::vector<std::string>* logMessages;
    bool shutdownDialogActive;
    irr::u32 lastShownJoystickStatus;
    irr::u32 lastTimeAzimuth1MasterChanged;
    irr::u32 lastTimeAzimuth2MasterChanged;

    irr::u16 previousJoystickPOV;
    bool previousJoystickPOVInitialised;

    void startShutdown();
    irr::f32 lookup1D(irr::f32 lookupValue, std::vector<irr::f32> inputPoints, std::vector<irr::f32> outputPoints);
    std::wstring f32To3dp(irr::f32 value, bool stripZeros=false);
    bool IsButtonPressed(irr::u32 button, irr::u32 buttonBitmap) const;
};

#endif
