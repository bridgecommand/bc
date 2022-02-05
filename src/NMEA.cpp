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

NMEA::NMEA(SimulationModel* model, std::string serialPortName, std::string serialBaudrate, std::string udpHostname, std::string udpPortName, irr::IrrlichtDevice* dev) //Constructor
{
    //link to model so network can interact with model
    this->model = model; //Link to the model
    device = dev; //Store pointer to irrlicht device

    messageToSend = "";
    currentMessageType = 0;

    //Set up UDP

    //TODO: Check guide at http://stripydog.blogspot.co.uk/2015/03/nmea-0183-over-ip-unwritten-rules-for.html
    try {
        asio::ip::udp::resolver resolver(io_service);
        asio::ip::udp::resolver::query query(asio::ip::udp::v4(), udpHostname, udpPortName);
        receiver_endpoint = *resolver.resolve(query);
    } catch (std::exception& e) {
        device->getLogger()->log(e.what());
    }

    //Set up serial
    uint32_t baudrate = std::stoi(serialBaudrate)
    if (!serialPortName.empty() || (baudrate > 0))
    {
        try
        {
            serial::Timeout timeout = serial::Timeout::simpleTimeout(50);

            mySerialPort.setPort(serialPortName);
            mySerialPort.setBaudrate(baudrate);//FIXME: Hardcoded
            mySerialPort.setTimeout(timeout);

            mySerialPort.open();
            device->getLogger()->log("Serial port opened.");

        }
        catch (std::exception const& e)
        {
            device->getLogger()->log(e.what());
        }
    }

}

NMEA::~NMEA()
{

    //Shut down serial port here
    if (mySerialPort.isOpen())
    {
        try
        {
            mySerialPort.close();
        }
        catch (std::exception const& e)
        {
        }
    }

}

void NMEA::updateNMEA()
{

    std::string dateTimeString = Utilities::ttos(model->getTimestamp());

    std::string dateString = dateTimeString.substr(0, 8);
    std::string timeString = dateTimeString.substr(8, 6);

    const char *year = dateString.substr(0, 4).c_str();
    const char *mon  = dateString.substr(4, 2).c_str();
    const char *mday = dateString.substr(6, 2).c_str();

    const char *hour = timeString.substr(0, 4).c_str();
    const char *min  = timeString.substr(4, 2).c_str();
    const char *sec  = timeString.substr(6, 2).c_str();

    irr::f32 rudderAngle = model->getRudder();

    int engineRPM[] = {
        Utilities::round(model->getStbdEngineRPM()), // idx=1, odd (starboard)
        Utilities::round(model->getPortEngineRPM())  // idx=2, even (port)
    };

    irr::f32 lat = model->getLat();
    irr::f32 lon = model->getLong();

    irr::f32 cog = model->getCOG();
    irr::f32 sog = model->getSOG()*MPS_TO_KTS;

    irr::f32 hdg = model->getHeading();
    irr::f32 rot = model->getRateOfTurn()*RAD_PER_S_IN_DEG_PER_MINUTE;

    irr::f32 depth = model->getDepth();

    char eastWest = easting[lon < 0];
    char northSouth = northing[lat < 0];

    lat = fabs(lat);
    lon = fabs(lon);
    irr::f32 latMinutes = (lat - (int)lat)*60;
    irr::f32 lonMinutes = (lon - (int)lon)*60;
    irr::u8 latDegrees = (int) lat;
    irr::u8 lonDegrees = (int) lon;

    char messageBuffer[maxSentenceChars];
    for (int i = 0; i<maxSentenceChars; i++) { //Avoid error about variable size object initialization
        messageBuffer[i]=0;
    }

    switch (currentMessageType) { // EN 61162-1:2011
        case RMC: // 8.3.69 Recommended minimum navigation information
            snprintf(messageBuffer,maxSentenceChars,"$GPRMC,%s%s%s.00,A,%02u%06.3f,%c,%03u%06.3f,%c,%.1f,%.1f,%s%s%s,,,A,S",hour,min,sec,latDegrees,latMinutes,northSouth,lonDegrees,lonMinutes,eastWest,sog,cog,year,mon,mday); //FIXME: SOG -> knots, COG->degrees
            messageToSend.append(addChecksum(std::string(messageBuffer)));
            break;
        case GLL: // 8.3.36 Geographic position – Latitude/longitude
            snprintf(messageBuffer,maxSentenceChars,"$GPGLL,%02u%06.3f,%c,%03u%06.3f,%c,%s%s%s.00,A,A",latDegrees,latMinutes,northSouth,lonDegrees,lonMinutes,eastWest,hour,min,sec);
            messageToSend.append(addChecksum(std::string(messageBuffer)));
            break;
        case GGA: // 8.3.35 Global positioning system (GPS) fix data
            snprintf(messageBuffer,maxSentenceChars,"$GPGGA,%s%s%s.00,%02u%06.3f,%c,%03u%06.3f,%c,1,12,0.0,0.0,M,0.0,M,,",hour,min,sec,latDegrees,latMinutes,northSouth,lonDegrees,lonMinutes,eastWest); //Hardcoded NMEA Quality 8, Satellites 8, HDOP 0.9
            messageToSend.append(addChecksum(std::string(messageBuffer)));
            break;
        case RSA: // 8.3.73 Rudder sensor angle
            snprintf(messageBuffer,maxSentenceChars,"$IIRSA,%.1f,A,,V",rudderAngle); // starboard (or single), A is valid, port sensor is null, thus V for invalid
            messageToSend.append(addChecksum(std::string(messageBuffer)));
            break;
        case RPM: // 8.3.72 Revolutions
            for (int i=0; i<2; i++) {
                snprintf(messageBuffer,maxSentenceChars,"$IIRPM,S,%d,%d,100,A",i+1,engineRPM[i]); // 'S' is for shaft, '100' is pitch (fixed)
                messageToSend.append(addChecksum(std::string(messageBuffer)));
            }
            break;
        case TTM: // 8.3.85 Tracked target message
            if (model->getARPATracks() > 0) {
                //To think about/add: Lost contacts? Manually aquired contacts?
                for (int i=0; i<model->getARPATracks(); i++) {
                    ARPAContact contact = model->getARPATrack(i);
                    ARPAEstimatedState state = contact.estimate;
                    snprintf(messageBuffer,maxSentenceChars,"$RATTM,%02d,%.1f,%.1f,T,%.1f,%.1f,T,%.1f,%.1f,N,TGT%02d,T,,%s.00,A",
                        state.displayID,
                        state.range,
                        state.bearing,
                        state.speed,
                        state.absHeading,
                        state.cpa,
                        state.tcpa,
                        state.displayID,
                        timeString.c_str()
                    );
                    messageToSend.append(addChecksum(std::string(messageBuffer)));
                }
            }
            break;
        /*
        case RSD: // 8.3.74 Radar system data
            snprintf(messageBuffer,maxSentenceChars,"$RARSD,");
            messageToSend.append(addChecksum(std::string(messageBuffer)));
            break;
        */
        case ZDA: // 8.3.106 Time and date
            snprintf(messageBuffer,maxSentenceChars,"$RAZDA,%s%s%s.00,%s,%s,%s,00,00",hour,min,sec,mday,mon,year);
            messageToSend.append(addChecksum(std::string(messageBuffer)));
            break;
        /*
        case OSD: // 8.3.64 Own ship data
            snprintf(messageBuffer,maxSentenceChars,"$RAOSD,");
            messageToSend.append(addChecksum(std::string(messageBuffer)));
            break;
        /*
        /*
        case POS: // 8.3.65 Device position and ship dimensions report or configuration command
            snprintf(messageBuffer,maxSentenceChars,"$INPOS,");
            messageToSend.append(addChecksum(std::string(messageBuffer)));
            break;
        */
        case DTM: // 8.3.27 Datum reference
            snprintf(messageBuffer,maxSentenceChars,"$RADTM,W84,,,,,,,");
            messageToSend.append(addChecksum(std::string(messageBuffer)));
            break;
        case HDT: // 8.3.44 Heading true
            snprintf(messageBuffer,maxSentenceChars,"$HEHDT,%.1f,T",hdg); // T = true north
            messageToSend.append(addChecksum(std::string(messageBuffer)));
            break;
        case DPT: //Depth
            snprintf(messageBuffer,maxSentenceChars,"$SDDPT,%.1f,,",depth); //Depth, Offset from transducer: Positive - distance from transducer to water line, or Negative - distance from transducer to keel, max depth measurable
            messageToSend.append(addChecksum(std::string(messageBuffer)));
            break;
        case ROT: // 8.3.71 Rate of turn
            snprintf(messageBuffer,maxSentenceChars,"$TIROT,%.1f,A",rot);  // A = data valid
            messageToSend.append(addChecksum(std::string(messageBuffer)));
            break;
        /*
        case VTG: // 8.3.98 Course over ground and ground speed
            snprintf(messageBuffer,maxSentenceChars,"$VDVTG,");
            messageToSend.append(addChecksum(std::string(messageBuffer)));
            break;
        */
        /*
        case HRM: // _, Heel angle, roll period, and roll amplitude
            snprintf(messageBuffer,maxSentenceChars,"$IIHRM,");
            messageToSend.append(addChecksum(std::string(messageBuffer)));
            break;
        */
        /*
        case HBT: // 8.3.42 Heartbeat supervision sentence (for engine room)
            snprintf(messageBuffer,maxSentenceChars,"$ERHBT,");
            messageToSend.append(addChecksum(std::string(messageBuffer)));
            break;
        */
        /*
        case VDM: // 8.3.90 AIS VHF data-link message (6-bit, iaw ITU-R M.1371)
            snprintf(messageBuffer,maxSentenceChars,"!AIVDM,");
            messageToSend.append(addChecksum(std::string(messageBuffer)));
            break;
        */
        /*
        case VDO: // 8.3.91 AIS VHF data-link own-vessel report (6-bit, iaw ITU-R M.1371)
            snprintf(messageBuffer,maxSentenceChars,"!AIVDO,");
            messageToSend.append(addChecksum(std::string(messageBuffer)));
            break;
        */
        default:
            break;
    }

    currentMessageType += 1;
    currentMessageType %= maxMessages;
}

void NMEA::sendNMEASerial()
{
    if (mySerialPort.isOpen())
    {
        mySerialPort.write(messageToSend);
    }
}

void NMEA::sendNMEAUDP()
{    
    if (messageToSend != "") {
        try {
            asio::ip::udp::socket socket(io_service);
            socket.open(asio::ip::udp::v4());
            socket.send_to(asio::buffer(messageToSend),receiver_endpoint);
            messageToSend = "";
        } catch (std::exception& e) {
            device->getLogger()->log(e.what());
        }
    }
}

std::string NMEA::addChecksum(std::string messageIn)
{
    char checksumBuffer[3];
    //Get checksum
    unsigned char checksum=0;
    irr::u8 s = messageIn.length();
    for(int i = 1; i<s; i++)
    {
        checksum^= messageIn.at(i);
    }
    snprintf(checksumBuffer,sizeof(checksumBuffer),"%02X",checksum);
    return messageIn + "*" + std::string(checksumBuffer) + "\r\n";
}
