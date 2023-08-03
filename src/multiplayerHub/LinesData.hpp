/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2023 James Packer

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

#ifndef __LINESDATA_HPP_INCLUDED__
#define __LINESDATA_HPP_INCLUDED__

#include <vector>
#include <string>
#include "irrlicht.h"


struct LineData {
    public:
    irr::f32 startX, startY, startZ, endX, endY, endZ, nominalLength, breakingTension, breakingStrain, nominalShipMass;
    int startType, endType, startID, endID, keepSlack, heaveIn; //0: Unknown, 1: Own ship, 2: Other ship, 3: Buoy, 4: Land object
    LineData():startX(0),startY(0),startZ(0),endX(0),endY(0),endZ(0),keepSlack(0),heaveIn(0),nominalLength(0),breakingTension(0),breakingStrain(0),nominalShipMass(0),startType(0),endType(0),startID(0),endID(0){}
};

//hold information about lines (mooring and towing)

class LinesData {

    public:
    LinesData(unsigned int numberOfShips);

    int getNumberOfLines(); // Overall number of lines from all peers
    int getNumberOfOtherLines(int thisPeer); // Overall number of lines from all peers, excluding this one
    // setLineData will add an entry if lineNumber >= getNumberOfLines
    void setLineDataSize(unsigned int shipNumber, unsigned int numberOfLines);
    void setLineData(unsigned int shipNumber, unsigned int lineNumber, int startType, int endType, int startID, int endID, int keepSlack, int heaveIn, irr::f32 startX, irr::f32 startY, irr::f32 startZ, irr::f32 endX, irr::f32 endY, irr::f32 endZ, irr::f32 nominalLength, irr::f32 breakingTension, irr::f32 breakingStrain, irr::f32 nominalShipMass);
    std::string getLineDataString(const unsigned int& shipNumber); // Peer number is the one to send data to

    private:
    std::vector<std::vector<LineData>> linesData;

};

#endif // __LINESDATA_HPP_INCLUDED__

