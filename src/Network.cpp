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

#include "Network.hpp"
#include "NetworkPrimary.hpp"
#include "NetworkSecondary.hpp"

#include "SimulationModel.hpp"
#include "Utilities.hpp"
#include "Constants.hpp"
#include "Leg.hpp"
#include <iostream>
#include <cstdio>

Network::~Network() //Virtual destructor
{
}

Network* Network::createNetwork(OperatingMode::Mode mode, int port, irr::IrrlichtDevice* dev) //Factory class, create a primary or secondary network object, and return a pointer
{
    if (mode != OperatingMode::Normal) {
        return new NetworkSecondary(port, mode, dev);
    } else {
        return new NetworkPrimary(port, dev);
    }
}
