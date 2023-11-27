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
#include "Autopilot.hpp"
#include "NMEASentences.hpp"
#include "SimulationModel.hpp"
#include "Constants.hpp"
#include "Utilities.hpp"
#include "AIS.hpp"
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

NMEA::NMEA(SimulationModel* model, std::string serialPortName, irr::u32 serialBaudrate, std::string udpHostname, std::string udpPortName, std::string udpListenPortName, irr::IrrlichtDevice* dev) : autopilot(model) //Constructor
{
    //link to model so network can interact with model
    this->model = model; //Link to the model
    device = dev; //Store pointer to irrlicht device
   
    messageQueue = {};
    lastSendEvent = 0;
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

    // set up listening thread
    terminateNmeaReceive = 0;
    receivedNmeaMessages = std::vector<std::string>();
    std::thread* receiveThreadObject = 0;
    receiveThreadObject = new std::thread(&NMEA::ReceiveThread, this, udpListenPortName);
    
    // create send socket
    socket = new asio::ip::udp::socket(io_service);

    //Set up serial
    if (!serialPortName.empty() && (serialBaudrate > 0))
    {
        try
        {
            serial::Timeout timeout = serial::Timeout::simpleTimeout(50);

            mySerialPort.setPort(serialPortName);
            mySerialPort.setBaudrate(serialBaudrate);
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

    // stop the NMEA receive thread
    terminateNmeaReceiveMutex.lock();
    terminateNmeaReceive = 1;
    terminateNmeaReceiveMutex.unlock();

}

void NMEA::ReceiveThread(std::string udpListenPortName)
{
    
    // setup socket
    asio::io_context io_context;
    asio::ip::udp::socket rcvSocket(io_context);

    try 
    {
        irr::u16 port = std::stoi(udpListenPortName);
        rcvSocket.open(asio::ip::udp::v4());
        rcvSocket.bind(asio::ip::udp::endpoint(asio::ip::udp::v4(), port));
        std::cout << "Listening for NMEA messages on " << rcvSocket.local_endpoint().address().to_string() << ":" << port << std::endl;
    } catch (std::exception e) 
    {
        std::cerr << e.what() << ". In NMEA::ReceiveThread()" << std::endl;
        return;
    }

    for (;;) 
    {
        
        try
        {
        
            // terminate thread?
            terminateNmeaReceiveMutex.lock();
            if (terminateNmeaReceive != 0)
            {
                terminateNmeaReceiveMutex.unlock();
                break;
            }
            terminateNmeaReceiveMutex.unlock();

            int bufferSize = 128;
            char * buf = new char[bufferSize]();
            
            // set socket timeout as in AISOverUDP
            #ifdef WIN32
            DWORD timeout = 1000;
            setsockopt(rcvSocket.native_handle(), SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(DWORD))!=0;
            #else
            struct timeval tv = { 1, 0 };
            setsockopt(rcvSocket.native_handle(), SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
            #endif
            
            // read from socket
            #ifdef WIN32
            int nread = ::recv(rcvSocket.native_handle(), buf, bufferSize,0);
            #else
            ssize_t nread = ::read(rcvSocket.native_handle(), buf, bufferSize);
            #endif

            if (nread > 0) 
            {
                // first char $ or !, otherwise ignore
                if (!(buf[0] == '$' || buf[0] == '!')) continue;

                // convert char buffer to string and add it to shared vector
                // subsequent processing is handled by NMEA::receive
                std::string message(buf);
                receivedNmeaMessagesMutex.lock();
                receivedNmeaMessages.push_back(message);
                receivedNmeaMessagesMutex.unlock();
            }

        } catch (std::exception e) 
        {
            std::cerr << e.what() << std::endl;
        }
    }
}

void NMEA::receive()
{
    // check the receivedNmeaMessages shared vector for new messages
    // if it is non-empty, parse the messages
    receivedNmeaMessagesMutex.lock();
    if (!receivedNmeaMessages.empty())
    {
        for (int i=0; i < receivedNmeaMessages.size(); i++)
        {
            std::string message = receivedNmeaMessages[i];

            // get all sentences and strip \r\n
            std::vector<std::string> sentences;
            int last_pos = 0; 
            int pos = message.find("\r\n");
            while (pos != std::string::npos)
            {
                int sentence_len = pos - last_pos;
                sentences.push_back(message.substr(last_pos, sentence_len));
                last_pos = pos;
                pos = message.find("\r\n", pos+3);
            }
            
            // iterate over sentences and handle them one by one
            for (int i=0; i < sentences.size(); i++)
            {
                std::string sentence = sentences[i];

                if (sentence.length() < 10) continue;
               
                // parse the provided checksum and verify it
                irr::u32 providedChecksum;
                irr::u32 checksum;
                try 
                {
                    providedChecksum = std::stoi(sentence.substr(sentence.length()-2, 2), 0, 16);
                    checksum = sentence.at(1);
                    for (auto character : sentence.substr(2, sentence.length()-5)) checksum ^= character;
                    if (checksum != providedChecksum) 
                    {
                        std::cerr << "invalid checksum: " << sentence << " expected " << std::hex << checksum << std::endl;
                        continue;
                    }
                } catch (const std::invalid_argument& e) { continue;
                } catch (const std::out_of_range& e) { continue; }


                // construct vector of fields
                std::vector<std::string> fields;
                char last_char;
                std::string field = "";
                for (int i=7; i < sentence.length(); i++) 
                {
                    last_char = sentence[i];
                    if (last_char == '*') break;
                    if (last_char == ',') 
                    {
                        fields.push_back(field);
                        field = "";
                    } else 
                    {
                        field += last_char;
                    }
                }
                fields.push_back(field);

                std::string type = sentence.substr(0, 1);

                if (!type.compare("!")) continue; // AIS sentence

                if (!type.compare("$")) 
                { // normal sentence
                    if (!sentence.substr(1,1).compare("P"))
                    {
                        // proprietary sentence
                        continue;
                    } else
                    {
                        std::string id = sentence.substr(3, 3);

                        if (!id.compare("APB"))
                        { // autopilot sentence B 

                            if (fields.size() != 14) continue; // we expect exactly 14 fields

                            APB apb;
                            try 
                            {
                                apb.status = fields[0][0];
                                apb.warning = fields[1][0];
                                apb.cross_track_error = std::stof(fields[2]);
                                apb.direction = fields[3][0];
                                apb.cross_track_units = fields[4][0];
                                apb.arrival_circle_entered = fields[5][0];
                                apb.perpendicular_passed = fields[6][0];
                                apb.bearing_orig_to_dest = std::stof(fields[7]);
                                apb.bearing_orig_to_dest_type = fields[8][0];
                                apb.dest_waypoint_id = fields[9];
                                apb.bearing_to_dest = std::stof(fields[10]);
                                apb.bearing_orig_to_dest_type = fields[11][0];
                                apb.heading_to_dest = std::stof(fields[12]);
                                apb.heading_to_dest_type = fields[13][0];
                            } catch (const std::invalid_argument& e)
                            {
                                std::cerr << "error while parsing a float value for APB" << std::endl;
                                continue;
                            }

                            autopilot.receiveAPB(apb);

                        } else if (!id.compare("RMB"))
                        { // recommended minimum navigation information B

                            if (fields.size() != 13 && fields.size() != 14) continue; // 13 or 14 fields based on NMEA version

                            RMB rmb;
                            try {
                                rmb.status = fields[0][0];
                                rmb.cross_track_error = std::stof(fields[1]);
                                rmb.direction = fields[2][0];
                                rmb.dest_waypoint_id = fields[3];
                                rmb.orig_waypoint_id = fields[4];
                                rmb.dest_waypoint_latitude = fields[5];
                                rmb.dest_waypoint_latitude_dir = fields[6][0];
                                rmb.dest_waypoint_longitude = fields[7];
                                rmb.dest_waypoint_longitude_dir = fields[8][0];
                                rmb.range_to_dest = std::stof(fields[9]);
                                rmb.bearing_to_dest = std::stof(fields[10]);
                                rmb.dest_closing_velocity = std::stof(fields[11]);
                                rmb.arrival_status = fields[12][0];
                                rmb.faa_mode = '\0';
                                if (fields.size() == 14) rmb.faa_mode = fields[13][0];
                            } catch (const std::invalid_argument& e)
                            {
                                std::cerr << "error while parsing a float value for RMB" << std::endl;
                                continue;
                            }

                            autopilot.receiveRMB(rmb);
                        }
                    }
                }
            }
        }
        receivedNmeaMessages.clear();
    }
    receivedNmeaMessagesMutex.unlock();
}


void NMEA::updateNMEA()
{
    char messageBuffer[maxSentenceChars];
    for (int i = 0; i<maxSentenceChars; i++) { //Avoid error about variable size object initialization
        messageBuffer[i]=0;
    }

    irr::u32 now = device->getTimer()->getTime();

    // AIS messages are scheduled based on amount of otherShips and their speed
    // check each frame if a new report should be sent
    if (model->getNumberOfOtherShips() >= 0) { // only consider AIS if there are other ships
        std::string messageToSend = "";
        // which ships are ready to send?
        std::vector<irr::u32> readyShips = AIS::getReadyShips(model, now);
        for (auto ship : readyShips) {
            // 8.3.90 AIS VHF data-link message (6-bit, iaw ITU-R M.1371)
            // Position Report Class A
            int fragments = 1;
            int fragmentNumber = 1;
            char radioChannel = 'B';
            std::string data;
            int fillBits;
            bool done;
            std::tie(data, fillBits) = AIS::generateClassAReport(model, ship);

            snprintf(messageBuffer,maxSentenceChars,"!AIVDM,%d,%d,,%c,%s,%d",
                    fragments,
                    fragmentNumber,
                    radioChannel,
                    data.c_str(),
                    fillBits
                    );
            messageToSend.append(addChecksum(std::string(messageBuffer)));
            if (messageToSend.length() > 800) { // ensure we don't build too big of a UDP packet
                messageQueue.push_back(messageToSend);
                messageToSend = "";
            }
        }
        readyShips.clear();
        if (messageToSend != "") {
            messageQueue.push_back(messageToSend);
        }
    }

    // if sufficient time elapsed since the last sensor report was sent,
    // construct and send sentence(s) for the next sensor
    if (now - lastSendEvent < sensorReportInterval) {
        return;
    }

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


    switch (currentMessageType) { // EN 61162-1:2011
        case RMC: // 8.3.69 Recommended minimum navigation information
        {
            snprintf(messageBuffer,maxSentenceChars,"$GPRMC,%s%s%s.00,A,%02u%06.3f,%c,%03u%06.3f,%c,%.1f,%.1f,%s%s%s,,,A,S",hour,min,sec,latDegrees,latMinutes,northSouth,lonDegrees,lonMinutes,eastWest,sog,cog,year,mon,mday); //FIXME: SOG -> knots, COG->degrees
            messageQueue.push_back(addChecksum(std::string(messageBuffer)));
            break;
        }
        case GLL: // 8.3.36 Geographic position – Latitude/longitude
        {
            snprintf(messageBuffer,maxSentenceChars,"$GPGLL,%02u%06.3f,%c,%03u%06.3f,%c,%s%s%s.00,A,A",latDegrees,latMinutes,northSouth,lonDegrees,lonMinutes,eastWest,hour,min,sec);
            messageQueue.push_back(addChecksum(std::string(messageBuffer)));
            break;
        }
        case GGA: // 8.3.35 Global positioning system (GPS) fix data
        {
            snprintf(messageBuffer,maxSentenceChars,"$GPGGA,%s%s%s.00,%02u%06.3f,%c,%03u%06.3f,%c,1,12,0.0,0.0,M,0.0,M,,",hour,min,sec,latDegrees,latMinutes,northSouth,lonDegrees,lonMinutes,eastWest); //Hardcoded NMEA Quality 8, Satellites 8, HDOP 0.9
            messageQueue.push_back(addChecksum(std::string(messageBuffer)));
            break;
        }
        case RSA: // 8.3.73 Rudder sensor angle
        {    
            snprintf(messageBuffer,maxSentenceChars,"$IIRSA,%.1f,A,,V",rudderAngle); // starboard (or single), A is valid, port sensor is null, thus V for invalid
            messageQueue.push_back(addChecksum(std::string(messageBuffer)));
            break;
        }
        case RPM: // 8.3.72 Revolutions
        {
            std::string messageToSend = "";
            for (int i=0; i<2; i++) {
                snprintf(messageBuffer,maxSentenceChars,"$IIRPM,S,%d,%d,100,A",i+1,engineRPM[i]); // 'S' is for shaft, '100' is pitch (fixed)
                messageToSend.append(addChecksum(std::string(messageBuffer)));
            }
            messageQueue.push_back(messageToSend);
            break;
        }
        case TTM: // 8.3.85 Tracked target message
        {
            if (model->getARPATracksSize() > 0) {
                std::string messageToSend = "";
                //To think about/add: Lost contacts? Manually aquired contacts?
                for (int i=0; i<model->getARPATracksSize(); i++) {
                    ARPAContact contact = model->getARPAContactFromTrackIndex(i);
                    ARPAEstimatedState state = contact.estimate;
                    snprintf(messageBuffer,maxSentenceChars,"$RATTM,%02d,%.1f,%.1f,T,%.1f,%.1f,T,%.1f,%.1f,N,TGT%02d,T,,%s.00,A",
                        state.displayID - 1,
                        state.range,
                        state.bearing,
                        state.speed,
                        state.absHeading,
                        state.cpa,
                        state.tcpa,
                        state.displayID - 1,
                        timeString.c_str()
                    );
                    messageToSend.append(addChecksum(std::string(messageBuffer)));
                }
                if (messageToSend != "") messageQueue.push_back(messageToSend);
            }
            break;
        }
        /*
        case RSD: // 8.3.74 Radar system data
            snprintf(messageBuffer,maxSentenceChars,"$RARSD,");
            messageToSend.append(addChecksum(std::string(messageBuffer)));
            break;
        */
        case ZDA: // 8.3.106 Time and date
        {
            snprintf(messageBuffer,maxSentenceChars,"$RAZDA,%s%s%s.00,%s,%s,%s,00,00",hour,min,sec,mday,mon,year);
            messageQueue.push_back(addChecksum(std::string(messageBuffer)));
            break;
        }
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
        {
            snprintf(messageBuffer,maxSentenceChars,"$RADTM,W84,,,,,,,");
            messageQueue.push_back(addChecksum(std::string(messageBuffer)));
            break;
        }
        case HEHDT: // 8.3.44 Heading true
        {
            snprintf(messageBuffer,maxSentenceChars,"$HEHDT,%.1f,T",hdg); // T = true north
            messageQueue.push_back(addChecksum(std::string(messageBuffer)));
            break;
        }
        case GPHDT: // 8.3.44 Heading true
        {
            snprintf(messageBuffer,maxSentenceChars,"$GPHDT,%.1f,T",hdg); // T = true north
            messageQueue.push_back(addChecksum(std::string(messageBuffer)));
            break;
        }
        case DPT: //Depth
        {
            snprintf(messageBuffer,maxSentenceChars,"$SDDPT,%.1f,,",depth); //Depth, Offset from transducer: Positive - distance from transducer to water line, or Negative - distance from transducer to keel, max depth measurable
            messageQueue.push_back(addChecksum(std::string(messageBuffer)));
            break;
        }
        case TIROT: // 8.3.71 Rate of turn
        {
            snprintf(messageBuffer,maxSentenceChars,"$TIROT,%.1f,A",rot);  // A = data valid
            messageQueue.push_back(addChecksum(std::string(messageBuffer)));
            break;
        }
        case GPROT: // 8.3.71 Rate of turn
        {
            snprintf(messageBuffer,maxSentenceChars,"$GPROT,%.1f,A",rot);  // A = data valid
            messageQueue.push_back(addChecksum(std::string(messageBuffer)));
            break;
        }
        case HEROT: // 8.3.71 Rate of turn
        {
            snprintf(messageBuffer,maxSentenceChars,"$HEROT,%.1f,A",rot);  // A = data valid
            messageQueue.push_back(addChecksum(std::string(messageBuffer)));
            break;
        }
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
        case VDO: // 8.3.91 AIS VHF data-link own-vessel report (6-bit, iaw ITU-R M.1371)
            snprintf(messageBuffer,maxSentenceChars,"!AIVDO,");
            messageToSend.append(addChecksum(std::string(messageBuffer)));
            break;
        */
        default:
            break;
    }

    lastSendEvent = now;

    currentMessageType += 1;
    currentMessageType %= maxMessages;
}

void NMEA::clearQueue()
{
    messageQueue.clear();
}

void NMEA::sendNMEASerial()
{
    if (mySerialPort.isOpen())
    {
        for (auto message : messageQueue)
        {
            mySerialPort.write(message);
        }
    }
}

void NMEA::sendNMEAUDP()
{    
    if (!messageQueue.empty()) {
        try {
            if (!socket->is_open()) {
                socket->open(asio::ip::udp::v4());
                socket->set_option(asio::socket_base::broadcast(true));
            }
            for (auto message : messageQueue)
            {
                socket->send_to(asio::buffer(message), receiver_endpoint);
            }
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
