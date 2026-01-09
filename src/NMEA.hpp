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

#include <mutex>
#include <string>
#include <asio.hpp> //For UDP
#include "../lib/serial/serial.h"
#include "Autopilot.hpp"
#include "OtherShips.hpp"
#include "OwnShip.hpp"
#include "Terrain.hpp"
#include "Wind.hpp"
#include "RadarCalculation.hpp"

class NMEA {

public:

  NMEA();
  NMEA(OwnShip *aOwnShip, OtherShips *aOtherShips, Terrain *aTerrain, Wind *aWind, RadarCalculation *aRadarCalc);
  ~NMEA();
  void Init(std::string serialPortName, irr::u32 serialBaudrate, std::string udpHostname, std::string udpPortName, std::string udpListenPortName);
  void updateNMEA(sTime& aTime);
  void sendNMEASerial();
  void sendNMEAUDP();
  void clearQueue();
  void ReceiveThread(std::string udpListenPortName);
  void receive();
  // not implemented: RSD, OSD, POS, VTG, HRM, VDO, HBT
  enum NMEAMessage { RMC=0, GPROT, GLL, RSA, RPM, VHW, VTG, GPHDT, HEROT, TTM, GGA, ZDA, DTM, HEHDT, WIMWV, WIMWR, TIROT, DPT};


private:
  Autopilot mAutopilot;

  OwnShip *mOwnShip;
  OtherShips *mOtherShips;
  Terrain *mTerrain;
  Wind *mWind;
  RadarCalculation *mRadarCalc;
  
  serial::Serial mySerialPort;
  irr::u32 lastSendEvent; // when was the last time an NMEA message was sent
  static const irr::u32 sensorReportInterval = 100; // milliseconds between sensor reports
  std::vector<std::string> messageQueue;
  std::string messageToSend;
  std::string addChecksum(std::string messageIn);
  const int maxMessages = (DPT - RMC) + 1; // how many messages are defined
  static const int maxSentenceChars = 79+1+1; // iaw EN 61162-1:2011 + start char + null termination
  const char northing[2] = {'N', 'S'};
  const char easting[2] = {'E', 'W'};
  int currentMessageType; // sequentially send different sentences
  asio::io_service io_service;
  asio::ip::udp::endpoint receiver_endpoint;
  asio::ip::udp::socket* socket;

  irr::u32 terminateNmeaReceive;
  std::mutex terminateNmeaReceiveMutex;
  std::vector<std::string> receivedNmeaMessages;
  std::mutex receivedNmeaMessagesMutex;
};

#endif // __NMEA_HPP_INCLUDED__
