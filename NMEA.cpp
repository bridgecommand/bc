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

#include "NMEA.hpp"
#include "SimulationModel.hpp"
#include "Constants.hpp"
#include "Utilities.hpp"
#include <iostream>
#include <string>

NMEA::NMEA(SimulationModel* model, std::string serialPortName) //Constructor
{
    //link to model so network can interact with model
    this->model = model; //Link to the model

    maxMessages=6;

    messageToSend = "";
    currentMessageType=0;

    if (!serialPortName.empty()){
        try {
            serial::Timeout timeout = serial::Timeout::simpleTimeout(50);

            mySerialPort.setPort(serialPortName);
            mySerialPort.setBaudrate(4800);//FIXME: Hardcoded
            mySerialPort.setTimeout(timeout);

            mySerialPort.open();
            std::cout << "Serial port opened." << std::endl;

        } catch (std::exception const& e) {
            std::cout << e.what() <<std::endl;
        }
    }

}

NMEA::~NMEA()
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

void NMEA::updateNMEA() {

    std::string timeString = Utilities::timestampToString(
        model->getTimestamp(), "%H%M%S");
    std::string dateString = Utilities::timestampToString(
        model->getTimestamp(), "%d%m%y");

    int rudderAngle = Utilities::round(model->getRudder());
    int portRPM = Utilities::round(model->getPortEngineRPM());
    int stbdRPM = Utilities::round(model->getStbdEngineRPM());

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

    //Todo: Replace with select/case block:
    if (currentMessageType        == 0) {
        snprintf(messageBuffer,100,"$GPRMC,%s,A,%02u%06.3f,%c,%03u%06.3f,%c,%.2f,%2f,%s,,,A",timeString.c_str(),latDegrees,latMinutes,northSouth,lonDegrees,lonMinutes,eastWest,sog,cog,dateString.c_str()); //FIXME: SOG -> knots, COG->degrees
    } else if (currentMessageType == 1) {
        snprintf(messageBuffer,100,"$GPGLL,%02u%06.3f,%c,%03u%06.3f,%c,%s,A,A",latDegrees,latMinutes,northSouth,lonDegrees,lonMinutes,eastWest,timeString.c_str());
    } else if (currentMessageType == 2) {
        snprintf(messageBuffer,100,"$GPGGA,%s,%02u%06.3f,%c,%03u%06.3f,%c,8,8,0.9,0.0,M,0.0,M,,",timeString.c_str(),latDegrees,latMinutes,northSouth,lonDegrees,lonMinutes,eastWest); //Hardcoded NMEA Quality 8, Satellites 8, HDOP 0.9
    } else if (currentMessageType == 3) {
        snprintf(messageBuffer,100,"$IIRSA,%d,A,,",rudderAngle);
    } else if (currentMessageType == 4) {
        snprintf(messageBuffer,100,"$IIRPM,S,1,%d,100,A",portRPM); //'S' is for shaft, '100' is pitch
    } else if (currentMessageType == 5) {
        snprintf(messageBuffer,100,"$IIRPM,S,2,%d,100,A",stbdRPM);
    }

    currentMessageType++;
    if (currentMessageType == maxMessages) {
        currentMessageType = 0;
    }

    std::string messageString(messageBuffer);

    messageToSend = addChecksum(messageString);
}

void NMEA::sendNMEASerial() {
    if (mySerialPort.isOpen()) {
        mySerialPort.write(messageToSend);
    }
}

std::string NMEA::addChecksum(std::string messageIn) {

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
