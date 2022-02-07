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

#ifndef __NMEA_HPP_INCLUDED__
#define __NMEA_HPP_INCLUDED__

#include "irrlicht.h" //For logger only
#include "libs/serial/serial.h"
#include <string>
#include <asio.hpp> //For UDP

//Forward declarations
class SimulationModel;

class NMEA {

public:

    NMEA(SimulationModel* model, std::string serialPortName, irr::u32 serialBaudrate, std::string udpHostname, std::string udpPortName, irr::IrrlichtDevice* dev);
    ~NMEA();
    void updateNMEA();
    void sendNMEASerial();
    void sendNMEAUDP();
    enum NMEAMessage { RMC=0, GLL, GGA, RSA, RPM, TTM, /*RSD,*/ ZDA, /*OSD, POS,*/ DTM, HDT, DPT, ROT/*, VTG, HRM, VDM, VDO, HBT*/ };

private:
    irr::IrrlichtDevice* device;
    SimulationModel* model;
    serial::Serial mySerialPort;
    std::string messageToSend;
    std::string addChecksum(std::string messageIn);
    const int maxMessages = (ROT - RMC) + 1; // how many messages are defined
	static const int maxSentenceChars = 79+1+1; // iaw EN 61162-1:2011 + start char + null termination
    const char northing[2] = {'N', 'S'};
    const char easting[2] = {'E', 'W'};
    int currentMessageType; // sequentially send different sentences
    asio::io_service io_service;
    asio::ip::udp::endpoint receiver_endpoint;
    //asio::ip::udp::socket* socket;
};

#endif // __NMEA_HPP_INCLUDED__
