/*   Bridge Command 5.0 - Virtual Handles Integration
     Copyright (C) 2016 Tim Claudius Stratmann
     tim.claudius.stratmann@uni-oldenburg.de
*/

#ifndef __VIRTUALHANDLES_HPP_INCLUDED__
#define __VIRTUALHANDLES_HPP_INCLUDED__

#include <string>
#include "sio_client.h"
//Forward declarations
class SimulationModel;

class VirtualHandles
{
public:
    VirtualHandles();
    ~VirtualHandles();

    void connectToVirtualHandles(std::string url);
    void setModel(SimulationModel* model);
    void update();

private:
    sio::client host;

    SimulationModel* model;
    void send();
    void sendParameter(std::string key, double val);
    void receive(sio::event &);
};

#endif
