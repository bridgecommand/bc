/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2015 James Packer

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

#ifndef __EVENTRECEIVER_HPP_INCLUDED__
#define __EVENTRECEIVER_HPP_INCLUDED__

#include "irrlicht.h"

//forward declarations
class GUIMain;
class ControllerModel;
class Network;

class EventReceiver : public irr::IEventReceiver
{
public:

    EventReceiver(irr::IrrlichtDevice* device, ControllerModel* model, GUIMain* gui, Network* network);
    bool OnEvent(const irr::SEvent& event);

    bool isMouseDown() const;

private:

    irr::IrrlichtDevice* device;
    ControllerModel* model;
    GUIMain* gui;
    Network* network;

};

#endif
