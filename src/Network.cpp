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
#include <string>

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

std::string Network::makeNetworkLinesString(SimulationModel* model)
{
    
    std::string stringToSend = "";
    
    for(int number = 0; number < (int)(model->getLines()->getNumberOfLines()); number++ ) {
        stringToSend.append(Utilities::lexical_cast<std::string>(model->getLines()->getLineStartX(number)));
        stringToSend.append(",");
        stringToSend.append(Utilities::lexical_cast<std::string>(model->getLines()->getLineStartY(number)));
        stringToSend.append(",");
        stringToSend.append(Utilities::lexical_cast<std::string>(model->getLines()->getLineStartZ(number)));
        stringToSend.append(",");
        stringToSend.append(Utilities::lexical_cast<std::string>(model->getLines()->getLineEndX(number)));
        stringToSend.append(",");
        stringToSend.append(Utilities::lexical_cast<std::string>(model->getLines()->getLineEndY(number)));
        stringToSend.append(",");
        stringToSend.append(Utilities::lexical_cast<std::string>(model->getLines()->getLineEndZ(number)));
        stringToSend.append(",");
        stringToSend.append(Utilities::lexical_cast<std::string>(model->getLines()->getLineStartType(number)));
        stringToSend.append(",");
        stringToSend.append(Utilities::lexical_cast<std::string>(model->getLines()->getLineEndType(number)));
        stringToSend.append(",");
        stringToSend.append(Utilities::lexical_cast<std::string>(model->getLines()->getLineStartID(number)));
        stringToSend.append(",");
        stringToSend.append(Utilities::lexical_cast<std::string>(model->getLines()->getLineEndID(number)));
        stringToSend.append(",");
        stringToSend.append(Utilities::lexical_cast<std::string>(model->getLines()->getLineNominalLength(number)));
        stringToSend.append(",");
        stringToSend.append(Utilities::lexical_cast<std::string>(model->getLines()->getLineBreakingTension(number)));
        stringToSend.append(",");
        stringToSend.append(Utilities::lexical_cast<std::string>(model->getLines()->getLineBreakingStrain(number)));
        stringToSend.append(",");
        stringToSend.append(Utilities::lexical_cast<std::string>(model->getLines()->getLineNominalShipMass(number)));
        stringToSend.append(",");
        if (model->getLines()->getKeepSlack(number)) {
            stringToSend.append("1");
        } else {
            stringToSend.append("0");
        }
        stringToSend.append(",");
        if (model->getLines()->getHeaveIn(number)) {
            stringToSend.append("1");
        } else {
            stringToSend.append("0");
        }
        
        if (number < (int)model->getLines()->getNumberOfLines()-1) {stringToSend.append("|");}
    }
    return stringToSend;
}
