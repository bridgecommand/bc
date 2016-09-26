/*   Bridge Command 5.0 - Virtual Handles Integration
     Copyright (C) 2016 Tim Claudius Stratmann
     tim.claudius.stratmann@uni-oldenburg.de
*/

#include "VirtualHandles.hpp"

//#include "sio_client.h"

#include "SimulationModel.hpp"
#include "Utilities.hpp"
#include "Constants.hpp"
#include <string.h>

#include "json.hpp"

// for convenience
using json = nlohmann::json;
using namespace std;


void receive2(sio::event& event){

}

VirtualHandles::VirtualHandles() //Constructor
{
    model=0; //Not linked at the moment
    host.connect("http://127.0.0.1:8080");
    std::string const msgString = "data";

    //typedef std::function<void(event& event)> event_listener;
    //std::function<void(sio::event& event)> *bla = &this->receive;
    //std::function<void(sio::event& event)> *bla = std::bind(&VirtualHandles::receive, this, _1);
    std::function<void(sio::event&)> eventHandlerMethod = [this](sio::event& event) {
        this->receive(event);
    };

    host.socket()->on(msgString, eventHandlerMethod);
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
std::string message2;

void VirtualHandles::update()
{
    //receive();
    send();
}

void VirtualHandles::send()
{
    json jMsg;

    jMsg["rudder-angle-indicator"] = to_string(model->getRudder());
    jMsg["turn-rate"] = to_string(model->getRateOfTurn());
    jMsg["heading"] = to_string(model->getHeading());
    jMsg["rudder"] = to_string(model->getRudder());
    jMsg["echolot"] = to_string(model->getLat());
    jMsg["gps-speed"] = to_string(model->getSOG());
    jMsg["gps-course"] = to_string(model->getCOG());



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
    std::string message = jMsg.dump();
    printf("\nSend message:");
    printf(message.c_str());
    host.socket()->emit("data", message);
    message2 = message;
}

void VirtualHandles::receive(sio::event& event)
{
    std::string msgString = event.get_nsp();
    printf("\nmsg: ");
    printf(msgString.c_str());



    try{
    auto msgJson = json::parse(message2);
    //auto msgJson = json::parse(msgString);

        if (msgJson.find("rudder") != msgJson.end() && msgJson["rudder"].is_number()) {
            double rudder = msgJson["rudder"];
            model->setRudder(rudder);
        }

    }catch (const std::invalid_argument& e) {
        printf("invalid argument");
    }
    /* type: irr::f32
    model->setRudder(irr::f32 rudder)
    model->setPortEngine(irr::f32 port)
    model->setStbdEngine(irr::f32 stbd)
    */
}
