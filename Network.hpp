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

#ifndef __NETWORK_HPP_INCLUDED__
#define __NETWORK_HPP_INCLUDED__

#include <string>

#include "irrlicht.h"
#include <enet/enet.h>
#include "OperatingModeEnum.hpp"

//Forward declarations
class SimulationModel;

class Network
{
    public:
    //Factory method
    static Network* createNetwork(OperatingMode::Mode mode, int port, irr::IrrlichtDevice* dev); //remember to use 'delete' later.
    virtual void connectToServer(std::string hostnames) = 0;
    virtual void setModel(SimulationModel* model) = 0;
    virtual void getScenarioFromNetwork(std::string& dataString) = 0; //Not used by primary
    virtual void update() = 0;
    virtual int getPort() = 0;
    virtual ~Network();
};

#endif
