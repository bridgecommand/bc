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
    //int port = 35678; //TODO: Remove hardcoding
    asio::io_context io_context;
    asio::ip::udp::socket socket(io_context, asio::ip::udp::endpoint(asio::ip::udp::v4(), _port));

    std::cout << "Set up on port " << _port << std::endl;

    for (;;)
    {
      try {
        asio::ip::udp::endpoint remote_endpoint;
        std::vector<char> buffer;

        std::cout << "Waiting for data 1." << std::endl;

        // Wait for data to arrive on socket
        socket.receive_from(asio::null_buffers(), remote_endpoint);

        std::cout << "Waiting for data 2." << std::endl;

        // Get datagram size
        decltype(socket)::bytes_readable readable(true);
        socket.io_control(readable);
        auto length = readable.get();

        std::cout << "Waiting for data 3." << std::endl;

        // Read the datagram
        buffer.resize(length); // Assume buffer is a std::vector<char>
        socket.receive_from(asio::buffer(buffer.data(), buffer.size()),remote_endpoint);
        std::string receivedString(buffer.begin(), buffer.end());
        std::cout << "Received: " << receivedString << std::endl;

      } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
      }
    }

}