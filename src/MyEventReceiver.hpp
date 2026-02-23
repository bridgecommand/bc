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
#include "Network.hpp"

//forward declarations
class GUIMain;
class Lines;
class VRInterface;

class MyEventReceiver : public irr::IEventReceiver
{
public:

  MyEventReceiver(irr::IrrlichtDevice* dev, void* aModel, GUIMain* gui, Network* network, VRInterface* vrInterface, std::vector<std::string>* logMessages);

    bool OnEvent(const irr::SEvent& event);
    //irr::s32 GetScrollBarPosSpeed() const;
    //irr::s32 GetScrollBarPosHeading() const;

private:

    void startShutdown();
    irr::f32 lookup1D(irr::f32 lookupValue, std::vector<irr::f32> inputPoints, std::vector<irr::f32> outputPoints);
    std::wstring f32To3dp(irr::f32 value, bool stripZeros = false);
    bool IsButtonPressed(irr::u32 button, irr::u32 buttonBitmap) const;
    void handleMooringLines(irr::core::line3df rayForLines);

  void* mModel;

  GUIMain* gui;
    VRInterface* vrInterface;
    Network* net;
    bool leftMouseDown;
    bool rightMouseDown;
    irr::IrrlichtDevice* device;
    irr::s32 scrollBarPosSpeed;
    irr::s32 scrollBarPosHeading;

    irr::s32 mouseClickX;
    irr::s32 mouseClickY;

    std::vector<std::string>* logMessages;
    bool shutdownDialogActive;
   

    irr::u32 linesMode; // 0 = none, 1 = own ship end, 2 = other end
};

#endif
