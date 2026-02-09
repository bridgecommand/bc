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
    uint32_t portJoystickAxis;
    uint32_t stbdJoystickAxis;
    uint32_t rudderJoystickAxis;
    uint32_t bowThrusterJoystickAxis;
    uint32_t sternThrusterJoystickAxis;
    uint32_t portJoystickNo;
    uint32_t stbdJoystickNo;
    uint32_t rudderJoystickNo;
    uint32_t bowThrusterJoystickNo;
    uint32_t sternThrusterJoystickNo;
    std::vector<float> inputPoints;
    std::vector<float> outputPoints;
    int32_t rudderDirection;
    bool updateAllAxes;
    bool enableMacOSJoystick;
    
    
// DEE 10JAN23 Azimuth Drive Specific vvvv
// for class JoystickSetup

// which joysticks control each lever and control, in the case of azimuth drives the specific controls are     
//    uint32_t azimuth1JoystickNo;
//    uint32_t azimuth2JoystickNo;
    uint32_t portThrustLever_joystickNo;
    uint32_t stbdThrustLever_joystickNo;
    uint32_t portSchottel_joystickNo;
    uint32_t stbdSchottel_joystickNo;

// defines the channel aka axis of the already defined joystick no    
//    uint32_t azimuth1JoystickAxis;
//    uint32_t azimuth2JoystickAxis;
    uint32_t portSchottel_channel;    
    uint32_t stbdSchottel_channel;
    uint32_t portThrustLever_channel;
    uint32_t stbdThrustLever_channel;

// determines if the schottel is inverted
//    int32_t azimuth1Direction;
//    int32_t azimuth2Direction;
    int32_t schottelPortDirection;
    int32_t schottelStbdDirection;    
    int32_t thrustLeverPortDirection;
    int32_t thrustLeverStbdDirection;

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
    uint32_t joystickNoHorn;
    uint32_t joystickButtonHorn;
    uint32_t joystickNoChangeView;
    uint32_t joystickButtonChangeView;
    uint32_t joystickNoChangeAndLockView;
    uint32_t joystickButtonChangeAndLockView;
    uint32_t joystickNoLookStepLeft;
    uint32_t joystickButtonLookStepLeft;
    uint32_t joystickNoLookStepRight;
    uint32_t joystickButtonLookStepRight;
    uint32_t joystickNoIncreaseBowThrust;
    uint32_t joystickButtonIncreaseBowThrust;
    uint32_t joystickNoDecreaseBowThrust;
    uint32_t joystickButtonDecreaseBowThrust;
    uint32_t joystickNoIncreaseSternThrust;
    uint32_t joystickButtonIncreaseSternThrust;
    uint32_t joystickNoDecreaseSternThrust;
    uint32_t joystickButtonDecreaseSternThrust;
    uint32_t joystickNoBearingOn;
    uint32_t joystickButtonBearingOn;
    uint32_t joystickNoBearingOff;
    uint32_t joystickButtonBearingOff;
    uint32_t joystickNoZoomOn;
    uint32_t joystickButtonZoomOn;
    uint32_t joystickNoZoomOff;
    uint32_t joystickButtonZoomOff;
    uint32_t joystickNoLookLeft;
    uint32_t joystickButtonLookLeft;
    uint32_t joystickNoLookRight;
    uint32_t joystickButtonLookRight;
    uint32_t joystickNoLookUp;
    uint32_t joystickButtonLookUp;
    uint32_t joystickNoLookDown;
    uint32_t joystickButtonLookDown;
    uint32_t joystickNoPump1On;
    uint32_t joystickButtonPump1On;
    uint32_t joystickNoPump1Off;
    uint32_t joystickButtonPump1Off;
    uint32_t joystickNoPump2On;
    uint32_t joystickButtonPump2On;
    uint32_t joystickNoPump2Off;
    uint32_t joystickButtonPump2Off;
    uint32_t joystickNoFollowUpOn;
    uint32_t joystickButtonFollowUpOn;
    uint32_t joystickNoFollowUpOff;
    uint32_t joystickButtonFollowUpOff;
    uint32_t joystickNoNFUPort;
    uint32_t joystickButtonNFUPort;
    uint32_t joystickNoNFUStbd;
    uint32_t joystickButtonNFUStbd;
    uint32_t joystickNoAckAlarm;
    uint32_t joystickButtonAckAlarm;
    // DEE 10JAN23 vvvv
    uint32_t joystickButtonAzimuth1Master;
    uint32_t joystickButtonAzimuth2Master;
    // DEE 10JAN23 ^^^^
    uint32_t joystickNoAzimuth1Master;
    uint32_t joystickNoAzimuth2Master;
    uint32_t joystickNoPOV;
    uint16_t joystickPOVLookLeft;
    uint16_t joystickPOVLookRight;
    uint16_t joystickPOVLookUp;
    uint16_t joystickPOVLookDown;

    uint16_t joystickButtonIncreaseClutterSetting;
    uint16_t joystickButtonDecreaseClutterSetting;
    uint16_t joystickButtonIncreaseGainSetting;
    uint16_t joystickButtonDecreaseGainSetting;
    uint16_t joystickButtonIncreaseRainSetting;
    uint16_t joystickButtonDecreaseRainSetting;
    uint16_t joystickButtonDecreaseRange;
    uint16_t joystickButtonIncreaseRange;

    uint16_t joystickNoIncreaseClutterSetting;
    uint16_t joystickNoDecreaseClutterSetting;
    uint16_t joystickNoIncreaseGainSetting;
    uint16_t joystickNoDecreaseGainSetting;
    uint16_t joystickNoIncreaseRainSetting;
    uint16_t joystickNoDecreaseRainSetting;
    uint16_t joystickNoDecreaseRange;
    uint16_t joystickNoIncreaseRange;
};

class MyEventReceiver : public irr::IEventReceiver
{
public:

  MyEventReceiver(irr::IrrlichtDevice* dev, SimulationModel* model, GUIMain* gui, Network* network, VRInterface* vrInterface, JoystickSetup joystickSetup, std::vector<std::string>* logMessages);

    bool OnEvent(const irr::SEvent& event);
    //int32_t GetScrollBarPosSpeed() const;
    //int32_t GetScrollBarPosHeading() const;

private:

    void startShutdown();
    float lookup1D(float lookupValue, std::vector<float> inputPoints, std::vector<float> outputPoints);
    std::wstring f32To3dp(float value, bool stripZeros = false);
    bool IsButtonPressed(uint32_t button, uint32_t buttonBitmap) const;
    void handleMooringLines(irr::core::line3df rayForLines);

    SimulationModel* model;
    GUIMain* gui;
    VRInterface* vrInterface;
    Network* net;
    bool leftMouseDown;
    bool rightMouseDown;
    irr::IrrlichtDevice* device;
    int32_t scrollBarPosSpeed;
    int32_t scrollBarPosHeading;
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

    int32_t mouseClickX;
    int32_t mouseClickY;

    JoystickSetup joystickSetup;
    std::vector<uint32_t> joystickPreviousButtonStates;
    std::vector<std::string>* logMessages;
    bool shutdownDialogActive;
    uint32_t lastShownJoystickStatus;
    uint32_t lastTimeAzimuth1MasterChanged;
    uint32_t lastTimeAzimuth2MasterChanged;

    uint16_t previousJoystickPOV;
    bool previousJoystickPOVInitialised;

    uint32_t linesMode; // 0 = none, 1 = own ship end, 2 = other end
};

#endif
