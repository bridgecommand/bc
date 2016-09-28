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
#include <iostream>
#include <sstream>
#include <exception>
// for convenience
using json = nlohmann::json;
using namespace std;
using namespace sio;

 double string_to_double( const std::string& s )
 {
   std::istringstream i(s);
   double x;
   if (!(i >> x))
     throw 20; // TODO: sinnvolle exception werfens
   return x;
 }

VirtualHandles::VirtualHandles() //Constructor
{
    model=0; //Not linked at the moment
    host.connect("http://10.53.1.83:8080");
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

void VirtualHandles::update()
{
    //receive();
    send();
}
std::string const nameString = "name";
std::string const valueString = "value";

void VirtualHandles::sendParameter(string key, double val){

    //cout<<key << ": " << val<< "\n";

    object_message::ptr data = object_message::create();
    message::ptr name = string_message::create(key);
    message::ptr value = double_message::create(val);

    data->get_map()[nameString]= name;
    data->get_map()[valueString]= value;
    host.socket()->emit("data", data);
}

std::string const rudderAngle = "rudder-angle-indicator";
std::string const turnRate = "turn-rate";
std::string const heading = "heading";
std::string const rudder = "rudder";
std::string const echolot = "echolot";
std::string const gpsSpeed = "gps-speed";
std::string const gpsCourse = "gps-course";

void VirtualHandles::send()
{
    //sio::message const& data =

    //void insert(const std::string & key,message::ptr const& message)


    sendParameter(rudderAngle, model->getRudder());
    sendParameter(turnRate, model->getRateOfTurn());
    sendParameter(heading, model->getHeading());
    sendParameter(rudder, model->getRudder());
    sendParameter(echolot, model->getLat());
    sendParameter(gpsSpeed, model->getSOG());
    sendParameter(gpsCourse, model->getCOG());

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

void VirtualHandles::receive(sio::event& event)
{
    sio::message::ptr const& data = event.get_message();

    auto m =  data->get_map();
    for(auto iterator = data->get_map().begin(); iterator != data->get_map().end(); iterator++){
        if(iterator->second->get_flag()==2){
        std::cout<<"\n k: " << iterator->first << " ,val:" << iterator->second->get_string();}else{
        std::cout<<"\n k: " << iterator->first << " ,val:" << iterator->second->get_double();}
    }

    auto nameIterator = m.find("name");

    if(nameIterator != m.end()){
        // key "name" exists, therefore a "value" must exist as well
        auto valueIterator = m.find("value");

        std::string name = nameIterator->second->get_string();
        if(name.compare("rudder") == 0){
            double rudderVal= valueIterator->second->get_double();
            model->setRudder(rudderVal);
        }else if(name.compare("machine-telegraph-left") == 0){
            string stringVal= valueIterator->second->get_string();
            try
            {
                double val = string_to_double(stringVal);
                model->setPortEngine(val);
                cout<<"rec name: " << name << " : " << val << " \n";
            }
            catch (exception& e)
            {
                cout<<stringVal << " is not a valid value for \"machine-telegraph\" to parse an double";
            }
        }else if(name.compare("machine-telegraph-right") == 0){
            string stringVal= valueIterator->second->get_string();
            try
            {
                double val = string_to_double(stringVal);
                model->setStbdEngine(val);
                cout<<"rec name: " << name << " : " << val << " \n";
            }
            catch (exception& e)
            {
                cout<<stringVal << " is not a valid value for \"machine-telegraph\" to parse an double";
            }        }else{
        }
    }

    /* type: irr::f32
    model->setRudder(irr::f32 rudder)
    model->setPortEngine(irr::f32 port)
    model->setStbdEngine(irr::f32 stbd)
    */
}
