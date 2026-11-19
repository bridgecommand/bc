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
#include "NetworkPrimary.hpp"
#include <string>
#include <vector>

//forward declarations
class GUIMain;
class SimulationModel;
class Lines;
class VRInterface;

//Data about joystick setup
class JoystickSetup {
public:
    unsigned int portJoystickAxis;
    unsigned int stbdJoystickAxis;
    unsigned int rudderJoystickAxis;
    unsigned int bowThrusterJoystickAxis;
    unsigned int sternThrusterJoystickAxis;
    unsigned int portJoystickNo;
    unsigned int stbdJoystickNo;
    unsigned int rudderJoystickNo;
    unsigned int bowThrusterJoystickNo;
    unsigned int sternThrusterJoystickNo;
    std::vector<float> inputPoints;
    std::vector<float> outputPoints;
    signed int rudderDirection;
    bool updateAllAxes;
    bool enableMacOSJoystick;
    
    
// DEE 10JAN23 Azimuth Drive Specific vvvv
// for class JoystickSetup

// which joysticks control each lever and control, in the case of azimuth drives the specific controls are     
//    unsigned int azimuth1JoystickNo;
//    unsigned int azimuth2JoystickNo;
    unsigned int portThrustLever_joystickNo;
    unsigned int stbdThrustLever_joystickNo;
    unsigned int portSchottel_joystickNo;
    unsigned int stbdSchottel_joystickNo;

// defines the channel aka axis of the already defined joystick no    
//    unsigned int azimuth1JoystickAxis;
//    unsigned int azimuth2JoystickAxis;
    unsigned int portSchottel_channel;    
    unsigned int stbdSchottel_channel;
    unsigned int portThrustLever_channel;
    unsigned int stbdThrustLever_channel;

// determines if the schottel is inverted
//    signed int azimuth1Direction;
//    signed int azimuth2Direction;
    signed int schottelPortDirection;
    signed int schottelStbdDirection;    
    signed int thrustLeverPortDirection;
    signed int thrustLeverStbdDirection;

// scaling and offset values for the schottel    
//    float azimuth1Scaling;
//    float azimuth2Scaling;
//    float azimuth1Offset;
//    float azimuth2Offset;
    float schottelPortScaling;
    float schottelStbdScaling;
    float schottelPortOffset;
    float schottelStbdOffset;
    
// scaling and offset for the thrust levers
    float thrustLeverPortScaling;
    float thrustLeverStbdScaling;
    float thrustLeverPortOffset;
    float thrustLeverStbdOffset;
    
    
// DEE 10JAN2023 ^^^^





//Buttons:
    unsigned int joystickNoHorn;
    unsigned int joystickButtonHorn;
    unsigned int joystickNoChangeView;
    unsigned int joystickButtonChangeView;
    unsigned int joystickNoChangeAndLockView;
    unsigned int joystickButtonChangeAndLockView;
    unsigned int joystickNoLookStepLeft;
    unsigned int joystickButtonLookStepLeft;
    unsigned int joystickNoLookStepRight;
    unsigned int joystickButtonLookStepRight;
    unsigned int joystickNoIncreaseBowThrust;
    unsigned int joystickButtonIncreaseBowThrust;
    unsigned int joystickNoDecreaseBowThrust;
    unsigned int joystickButtonDecreaseBowThrust;
    unsigned int joystickNoIncreaseSternThrust;
    unsigned int joystickButtonIncreaseSternThrust;
    unsigned int joystickNoDecreaseSternThrust;
    unsigned int joystickButtonDecreaseSternThrust;
    unsigned int joystickNoBearingOn;
    unsigned int joystickButtonBearingOn;
    unsigned int joystickNoBearingOff;
    unsigned int joystickButtonBearingOff;
    unsigned int joystickNoZoomOn;
    unsigned int joystickButtonZoomOn;
    unsigned int joystickNoZoomOff;
    unsigned int joystickButtonZoomOff;
    unsigned int joystickNoLookLeft;
    unsigned int joystickButtonLookLeft;
    unsigned int joystickNoLookRight;
    unsigned int joystickButtonLookRight;
    unsigned int joystickNoLookUp;
    unsigned int joystickButtonLookUp;
    unsigned int joystickNoLookDown;
    unsigned int joystickButtonLookDown;
    unsigned int joystickNoPump1On;
    unsigned int joystickButtonPump1On;
    unsigned int joystickNoPump1Off;
    unsigned int joystickButtonPump1Off;
    unsigned int joystickNoPump2On;
    unsigned int joystickButtonPump2On;
    unsigned int joystickNoPump2Off;
    unsigned int joystickButtonPump2Off;
    unsigned int joystickNoFollowUpOn;
    unsigned int joystickButtonFollowUpOn;
    unsigned int joystickNoFollowUpOff;
    unsigned int joystickButtonFollowUpOff;
    unsigned int joystickNoNFUPort;
    unsigned int joystickButtonNFUPort;
    unsigned int joystickNoNFUStbd;
    unsigned int joystickButtonNFUStbd;
    unsigned int joystickNoAckAlarm;
    unsigned int joystickButtonAckAlarm;
    // DEE 10JAN23 vvvv
    unsigned int joystickButtonAzimuth1Master;
    unsigned int joystickButtonAzimuth2Master;
    // DEE 10JAN23 ^^^^
    unsigned int joystickNoAzimuth1Master;
    unsigned int joystickNoAzimuth2Master;
    unsigned int joystickNoPOV;
    irr::u16 joystickPOVLookLeft;
    irr::u16 joystickPOVLookRight;
    irr::u16 joystickPOVLookUp;
    irr::u16 joystickPOVLookDown;

    irr::u16 joystickButtonIncreaseClutterSetting;
    irr::u16 joystickButtonDecreaseClutterSetting;
    irr::u16 joystickButtonIncreaseGainSetting;
    irr::u16 joystickButtonDecreaseGainSetting;
    irr::u16 joystickButtonIncreaseRainSetting;
    irr::u16 joystickButtonDecreaseRainSetting;
    irr::u16 joystickButtonDecreaseRange;
    irr::u16 joystickButtonIncreaseRange;

    irr::u16 joystickNoIncreaseClutterSetting;
    irr::u16 joystickNoDecreaseClutterSetting;
    irr::u16 joystickNoIncreaseGainSetting;
    irr::u16 joystickNoDecreaseGainSetting;
    irr::u16 joystickNoIncreaseRainSetting;
    irr::u16 joystickNoDecreaseRainSetting;
    irr::u16 joystickNoDecreaseRange;
    irr::u16 joystickNoIncreaseRange;
};

class MyEventReceiver : public irr::IEventReceiver
{
public:

  MyEventReceiver(irr::IrrlichtDevice* dev, SimulationModel* model, GUIMain* gui, Network* network, VRInterface* vrInterface, JoystickSetup joystickSetup, std::vector<std::string>* logMessages);

    bool OnEvent(const irr::SEvent& event);
    //signed int GetScrollBarPosSpeed() const;
    //signed int GetScrollBarPosHeading() const;

private:

    void startShutdown();
    float lookup1D(float lookupValue, std::vector<float> inputPoints, std::vector<float> outputPoints);
    std::wstring f32To3dp(float value, bool stripZeros = false);
    bool IsButtonPressed(unsigned int button, unsigned int buttonBitmap) const;
    void handleMooringLines(irr::core::line3df rayForLines);

    SimulationModel* model;
    GUIMain* gui;
    VRInterface* vrInterface;
    Network* net;
    bool leftMouseDown;
    bool rightMouseDown;
    irr::IrrlichtDevice* device;
    signed int scrollBarPosSpeed;
    signed int scrollBarPosHeading;
    irr::core::array<irr::SJoystickInfo> joystickInfo;

    float previousJoystickPort;
    float previousJoystickStbd;
    float previousJoystickRudder;
// DEE 10JAN23 vvvv
//    float previousJoystickAzimuthAngPort;
//    float previousJoystickAzimuthAngStbd;

    float previousJoystickThrustLeverPort;
    float previousJoystickThrustLeverStbd;
    float newJoystickThrustLeverPort;
    float newJoystickThrustLeverStbd;

    float previousJoystickSchottelPort;
    float previousJoystickSchottelStbd;
    float newJoystickSchottelPort;
    float newJoystickSchottelStbd;

// DEE 10JAN23 ^^^^
    float previousJoystickBowThruster;
    float previousJoystickSternThruster;

    signed int mouseClickX;
    signed int mouseClickY;

    JoystickSetup joystickSetup;
    std::vector<unsigned int> joystickPreviousButtonStates;
    std::vector<std::string>* logMessages;
    bool shutdownDialogActive;
    unsigned int lastShownJoystickStatus;
    unsigned int lastTimeAzimuth1MasterChanged;
    unsigned int lastTimeAzimuth2MasterChanged;

    irr::u16 previousJoystickPOV;
    bool previousJoystickPOVInitialised;

    unsigned int linesMode; // 0 = none, 1 = own ship end, 2 = other end

    float preShutdownAccelerator;
};

#endif
