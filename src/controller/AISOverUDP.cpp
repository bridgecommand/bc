/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2020 James Packer

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

#include "AISOverUDP.hpp"
#include <vector>
#include <iostream>
#include <mutex>
#include "libs/aisparser/c/src/portable.h"
#include "libs/aisparser/c/src/nmea.h"
#include "libs/aisparser/c/src/sixbit.h"
#include "libs/aisparser/c/src/vdm_parse.h"

extern int terminateAISThread;
extern std::mutex terminateAISThread_mutex;
extern std::vector<AISData> aisDataVector;
extern std::mutex aisDataVectorMutex;

AISOverUDP::AISOverUDP(int port) {
    _port = port;
}

void AISOverUDP::AISThread()
{
    
    asio::io_context io_context;
    asio::ip::udp::socket socket(io_context);
    try {
      socket.open(asio::ip::udp::v4());
      socket.bind(asio::ip::udp::endpoint(asio::ip::udp::v4(), _port));
      std::cout << "AIS set up on port " << _port << std::endl;
    } catch (std::exception& e) {
      std::cerr << e.what() << std::endl;
      return;
    }

    ais_state ais = {};
    aismsg_1 msg_1 = {};
    aismsg_2 msg_2 = {};
    aismsg_3 msg_3 = {};
    aismsg_5 msg_5 = {};

    for (;;)
    {
      
      //Check if we've been asked to stop
      terminateAISThread_mutex.lock();
      if (terminateAISThread!=0) {
        terminateAISThread_mutex.unlock();
        break;
      }
      terminateAISThread_mutex.unlock();
      
      try {

        //unsigned char buf[128];
        char buf[128];

        //std::cout << "Waiting for data 1." << std::endl;

        //This is all with low level socket functions, not actually asio. Needs to be checked on Windows!

        //Set timeout
        #ifdef WIN32
        DWORD timeout = 1000;
        setsockopt(socket.native_handle(), SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(DWORD))!=0;
        #else
        struct timeval tv = { 1, 0 };
        setsockopt(socket.native_handle(), SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        #endif

        //Read from UDP socket
        #ifdef WIN32
        int nread = ::recv(socket.native_handle(), buf, sizeof(buf),0);
        #else
        ssize_t nread = ::read(socket.native_handle(), buf, sizeof(buf));
        #endif

        if (nread>0) {
          //std::cout << "Received: " << buf << std::endl;

          if (assemble_vdm( &ais, buf ) == 0) {
            ais.msgid = (unsigned char) get_6bit( &ais.six_state, 6 );
            //std::cout << "Recieved AIS Message type " << (int)ais.msgid << std::endl;
            if( (ais.msgid == 1)&&(parse_ais_1( &ais, &msg_1 ) == 0) ) {  
              double lat;
              double lon;
              pos2ddd(msg_1.latitude,msg_1.longitude,&lat,&lon); //Convert to decimal lat and long
              //std::cout << "Received AIS Message type 1: COG:" << msg_1.cog << " SOG:" << msg_1.sog << " MMSI:" << msg_1.userid << " Lat:" << lat << " Long: " << lon << std::endl;
              AISData aisData = {};
              aisData.mmsi = msg_1.userid;
              aisData.cog = msg_1.cog;
              aisData.sog = msg_1.sog;
              aisData.messageID = 1;
              aisData.latitude = lat;
              aisData.longitude = lon;
              aisDataVectorMutex.lock();
              aisDataVector.push_back(aisData);
              aisDataVectorMutex.unlock();
            } else if( (ais.msgid == 2)&&(parse_ais_2( &ais, &msg_2 ) == 0) ) {  
              double lat;
              double lon;
              pos2ddd(msg_2.latitude,msg_2.longitude,&lat,&lon); //Convert to decimal lat and long
              //std::cout << "Received AIS Message type 2: COG:" << msg_2.cog << " SOG:" << msg_2.sog << " MMSI:" << msg_2.userid << " Lat:" << lat << " Long: " << lon << std::endl;
              AISData aisData = {};
              aisData.mmsi = msg_2.userid;
              aisData.cog = msg_2.cog;
              aisData.sog = msg_2.sog;
              aisData.messageID = 2;
              aisData.latitude = lat;
              aisData.longitude = lon;
              aisDataVectorMutex.lock();
              aisDataVector.push_back(aisData);
              aisDataVectorMutex.unlock();
            } else if( (ais.msgid == 3)&&(parse_ais_3( &ais, &msg_3 ) == 0) ) {  
              double lat;
              double lon;
              pos2ddd(msg_3.latitude,msg_3.longitude,&lat,&lon); //Convert to decimal lat and long
              //std::cout << "Received AIS Message type 3: COG:" << msg_3.cog << " SOG:" << msg_3.sog << " MMSI:" << msg_3.userid << " Lat:" << lat << " Long: " << lon << std::endl;
              AISData aisData = {};
              aisData.mmsi = msg_3.userid;
              aisData.cog = msg_3.cog;
              aisData.sog = msg_3.sog;
              aisData.messageID = 3;
              aisData.latitude = lat;
              aisData.longitude = lon;
              aisDataVectorMutex.lock();
              aisDataVector.push_back(aisData);
              aisDataVectorMutex.unlock();
            } else if( (ais.msgid == 5)&&(parse_ais_5( &ais, &msg_5 ) == 0) ) {  
              //std::cout << "Received AIS Message type 5: MMSI:" << msg_5.userid << " Name:" << msg_5.name << std::endl;
              AISData aisData = {};
              aisData.mmsi = msg_5.userid;
              aisData.name = std::string(msg_5.name);
              aisData.messageID = 5;
              aisDataVectorMutex.lock();
              aisDataVector.push_back(aisData);
              aisDataVectorMutex.unlock();
            }
          }

        }

      } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
      }
    }

    std::cout << "Ending AIS UDP" << std::endl;

}
