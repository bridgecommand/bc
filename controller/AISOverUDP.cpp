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

AISOverUDP::AISOverUDP(int port) {
    _port = port;
}

void AISOverUDP::AISThread()
{
    asio::io_context io_context;
    asio::ip::udp::socket socket(io_context);
    socket.open(asio::ip::udp::v4());
    socket.bind(asio::ip::udp::endpoint(asio::ip::udp::v4(), _port));

    std::cout << "Set up on port " << _port << std::endl;

    for (;;)
    {
      try {
        
        unsigned char buf[128];

        std::cout << "Waiting for data 1." << std::endl;
        
        //This is all with low level socket functions, not actually asio. Needs to be checked on Windows!
        
        //Set timeout
        struct timeval tv = { 1, 0 };
        setsockopt(socket.native_handle(), SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        
        //Read from UDP socket
        ssize_t nread = ::read(socket.native_handle(), buf, sizeof(buf));
        
        if (nread>0) {
          std::cout << "Received: " << buf << std::endl;
        }

      } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
      }
    }

}