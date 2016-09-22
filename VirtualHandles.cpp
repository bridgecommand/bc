/*   Bridge Command 5.0 - Virtual Handles Integration
     Copyright (C) 2016 Tim Claudius Stratmann
     tim.claudius.stratmann@uni-oldenburg.de
*/

#include "VirtualHandles.hpp"

//#include "sio_client.h"

#include "SimulationModel.hpp"
#include "Utilities.hpp"
#include "Constants.hpp"

VirtualHandles::VirtualHandles() //Constructor
{
	model=0; //Not linked at the moment
}

VirtualHandles::~VirtualHandles() //Destructor
{
    //shut down socket.io connection
	// TODO implement

    std::cout << "Shut down VirtualHandles connection\n";
}

void VirtualHandles::connectToVirtualHandles(std::string url)
{
	
}

void VirtualHandles::setModel(SimulationModel* model)
{
	this->model = model;
}

void VirtualHandles::update()
{
	receive();
	send();
}

void VirtualHandles::send()
{
	/* type: irr::f32 
	model->getLat()
	model->getLong()
	model->getCOG() //FIXME: currently the same as getHeading(), because current is not implemented, yet
	model->getSOG() //FIXME: currently the same as getSpeed(), because current is not implemented, yet
	model->getSpeed() // speed through water
	model->getHeading() // compass course
	model->getRudder()
	model->getRateOfTurn()
	model->getPortEngineRPM()
	model->getStbdEngineRPM()
	model->getPortEngine() //FIXME: currently commented out in SimulationModel
	model->getStbdEngine() //FIXME: currently commented out in SimulationModel
	
	*/
}

void VirtualHandles::receive()
{
	/* type: irr::f32
	model->setRudder(irr::f32 rudder)
	model->setPortEngine(irr::f32 port)
	model->setStbdEngine(irr::f32 stbd)
	*/
}

