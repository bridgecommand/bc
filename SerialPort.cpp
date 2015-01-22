//TODO: Rename this as NMEA.cpp

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

#include "SerialPort.hpp"
#include "SimulationModel.hpp"
#include "Constants.hpp"
#include <iostream>
#include <string>

SerialPort::SerialPort(SimulationModel* model) //Constructor
{
    //link to model so network can interact with model
    this->model = model; //Link to the model

    messageToSend = "";

    try {
        mySerialPort.setPort("COM3"); //FIXME: Hardcoded.
        mySerialPort.open();
        std::cout << "Serial port opened." << std::endl;
    } catch (std::exception const& e) {
        std::cout << e.what() <<std::endl;
    }

}

SerialPort::~SerialPort()
{

    //Shut down serial port here
    if (mySerialPort.isOpen()) {
        std::cout << "Closing serial port" << std::endl;
        try {
            mySerialPort.close();
        } catch (std::exception const& e) {
            std::cout << e.what() <<std::endl;
        }
    }

}

void SerialPort::updateNMEA() {

    f32 lat = model->getLat();
    f32 lon = model->getLong();
    f32 cog = model->getCOG();
    f32 sog = model->getSOG()*MPS_TO_KTS;
    char eastWest;
    char northSouth;
    if (lat >= 0) {
        northSouth='N';
    } else {
        northSouth='S';
    }
    if (lon >= 0) {
        eastWest='E';
    } else {
        eastWest='W';
    }
    lat = fabs(lat);
    lon = fabs(lon);

    f32 latMinutes = (lat - (int)lat)*60;
    f32 lonMinutes = (lon - (int)lon)*60;
    u8 latDegrees = (int) lat;
    u8 lonDegrees = (int) lon;

    char messageBuffer[256];

    //snprintf(messageBuffer,100,"$GPGLL,%02u%06.3f,%c,%03u%06.3f,%c,110141,A,A",latDegrees,latMinutes,northSouth,lonDegrees,lonMinutes,eastWest);
    snprintf(messageBuffer,100,"$GPRMC,110141,A,%02u%06.3f,%c,%03u%06.3f,%c,%.2f,%2f,010170,,,A",latDegrees,latMinutes,northSouth,lonDegrees,lonMinutes,eastWest,sog,cog); //FIXME: SOG -> knots, COG->degrees

    std::string messageString(messageBuffer);

    messageToSend = addChecksum(messageString);
}

void SerialPort::sendNMEASerial() {
    if (mySerialPort.isOpen()) {
        mySerialPort.write(messageToSend);
    }
}

std::string SerialPort::addChecksum(std::string messageIn) {

    char checksumBuffer[6];

    //Get checksum
    unsigned char checksum=0;

    u8 s = messageIn.length();
    for(int i = 1; i<s; i++) {
        checksum^= messageIn.at(i);
    }
    snprintf(checksumBuffer,sizeof(checksumBuffer),"*%02X\r\n",checksum);

    std::string checksumString(checksumBuffer);
    messageIn.append(checksumString);
    return messageIn;

}
