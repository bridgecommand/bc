/*   Bridge Command 5.0 - Virtual Handles Integration
     Copyright (C) 2016 Tim Claudius Stratmann
     tim.claudius.stratmann@uni-oldenburg.de
*/

#ifndef __VIRTUALHANDLES_HPP_INCLUDED__
#define __VIRTUALHANDLES_HPP_INCLUDED__

#include <string>

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
    SimulationModel* model;
    void send();
    void receive();
};

#endif
