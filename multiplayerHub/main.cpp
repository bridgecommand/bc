/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2016 James Packer

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

// main.cpp

#include <iostream>

// Include the Irrlicht header
#include "irrlicht.h"
#include "../Utilities.hpp"
#include "Network.hpp"

//Mac OS:
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

int main()
{

    //Mac OS:
	#ifdef __APPLE__
    //Find starting folder
    char exePath[1024];
    uint32_t pathSize = sizeof(exePath);
    std::string exeFolderPath = "";
    if (_NSGetExecutablePath(exePath, &pathSize) == 0) {
        std::string exePathString(exePath);
        size_t pos = exePathString.find_last_of("\\/");
        if (std::string::npos != pos) {
            exeFolderPath = exePathString.substr(0, pos);
        }
    }
    //change up from BridgeCommand.app/Contents/MacOS/mh.app/Contents/MacOS to BridgeCommand.app/Contents/Resources
    exeFolderPath.append("/../../../../Resources");
    //change to this path now, so ini file is read
    chdir(exeFolderPath.c_str());
    //Note, we use this again after the createDevice call
	#endif

    //User read/write location - look in here first and the exe folder second for files
    std::string userFolder = Utilities::getUserDir();

    std::cout << "User folder is " << userFolder <<std::endl;

    /*Overview:
    Load scenario, including initial positions of each player
    Start as an enet client
    Connect to each multiplayer pc (each as a server)
    Send scenario information to each multiplayer pc (Tailored to include all other players as other ships)
    Send first update to each pc, based on initial positions
    Then loop:
        Get feedback from each pc for current position and heading
        Use this to update internal model
        Send out update to each pc, including other ship positions
    */



    std::string hostnames;
    std::cout << "Please enter comma separated list of multiplayer PC host names:" << std::endl;
    std::cin >> hostnames;

    int port = 18304; //TODO: Read in from ini file

    Network network(port);
    network.connectToServer(hostnames);

    //Send initial scenario information (reliable packet)
    network.sendString("SCN1#a) Buoyage#SimpleEstuary#11#6#11#2010#6#18#1#0.5#4#Waverley,10,-9.98083,50.0388,180#Waverley|-9.983|50.029|20?12?10",true,0);

    //Send initial scenario update

    //Start main loop, listening for updates from PCs and sending out scenario update

    return(0);

}
