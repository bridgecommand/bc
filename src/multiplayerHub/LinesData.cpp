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

#include "LinesData.hpp"
#include "../Constants.hpp"
#include "../Utilities.hpp"

//#include <iostream>

LinesData::LinesData(unsigned int numberOfShips)
{
    std::vector<LineData> emptyShipDataEntry;
    for (unsigned int i = 0; i<numberOfShips; i++) {
        linesData.push_back(emptyShipDataEntry);
    }
}

int LinesData::getNumberOfLines() 
{
    int overallLinesSize = 0;
    // find combined size of all
    for (int i = 0; i < linesData.size(); i++) {
        overallLinesSize += linesData.at(i).size();
    }
    return overallLinesSize;
}

int LinesData::getNumberOfOtherLines(int thisPeer)
{
    int overallLinesSize = 0;
    // find combined size of all, excluding ones from this peer
    for (int i = 0; i < linesData.size(); i++) {
        if (i != thisPeer) {
            overallLinesSize += linesData.at(i).size();
        }
    }
    return overallLinesSize;
}

void LinesData::setLineDataSize(unsigned int shipNumber, unsigned int numberOfLines)
{
    if (linesData.size() > shipNumber) {
    
        // Make larger if required
        while (numberOfLines > linesData.at(shipNumber).size()) 
        {
            LineData emptyLineDataEntry;
            linesData.at(shipNumber).push_back(emptyLineDataEntry);
        }

        // Make smaller if required
        while (numberOfLines < linesData.at(shipNumber).size()) 
        {
            linesData.at(shipNumber).pop_back();
        }
    }
}

void LinesData::setLineData(unsigned int shipNumber, unsigned int lineNumber, int startType, int endType, int startID, int endID, int keepSlack, int heaveIn, irr::f32 startX, irr::f32 startY, irr::f32 startZ, irr::f32 endX, irr::f32 endY, irr::f32 endZ, irr::f32 nominalLength, irr::f32 breakingTension, irr::f32 breakingStrain, irr::f32 nominalShipMass)
{
    if (linesData.size() > shipNumber) {
        // Call setLineDataSize before so this is true.   
        if (lineNumber < linesData.at(shipNumber).size()) {
            linesData.at(shipNumber).at(lineNumber).startType = startType;
            linesData.at(shipNumber).at(lineNumber).endType = endType;
            linesData.at(shipNumber).at(lineNumber).startID = startID;
            linesData.at(shipNumber).at(lineNumber).endID = endID;
            linesData.at(shipNumber).at(lineNumber).keepSlack = keepSlack;
            linesData.at(shipNumber).at(lineNumber).heaveIn = heaveIn;
            linesData.at(shipNumber).at(lineNumber).startX = startX;
            linesData.at(shipNumber).at(lineNumber).startY = startY;
            linesData.at(shipNumber).at(lineNumber).startZ = startZ;
            linesData.at(shipNumber).at(lineNumber).endX = endX;
            linesData.at(shipNumber).at(lineNumber).endY = endY;
            linesData.at(shipNumber).at(lineNumber).endZ = endZ;
            linesData.at(shipNumber).at(lineNumber).nominalLength = nominalLength;
            linesData.at(shipNumber).at(lineNumber).breakingTension = breakingTension;
            linesData.at(shipNumber).at(lineNumber).breakingStrain = breakingStrain;
            linesData.at(shipNumber).at(lineNumber).nominalShipMass = nominalShipMass;
        }

    }
    
}

std::string LinesData::getLineDataString(const unsigned int& shipNumber)
{
    // shipNumber is the ID we are sending the data to
    std::string linesDataString = "";

    for (int i = 0; i < linesData.size(); i++) {
        // i is the ID this data has been sent by
        if (i != shipNumber) { 
            // Don't add line feedback to our own ship, as the lines already exist there!
            for (int j = 0; j < linesData.at(i).size(); j++ ) {
                linesDataString.append(Utilities::lexical_cast<std::string>(linesData.at(i).at(j).startX));
                linesDataString.append(",");
                linesDataString.append(Utilities::lexical_cast<std::string>(linesData.at(i).at(j).startY));
                linesDataString.append(",");
                linesDataString.append(Utilities::lexical_cast<std::string>(linesData.at(i).at(j).startZ));
                linesDataString.append(",");
                linesDataString.append(Utilities::lexical_cast<std::string>(linesData.at(i).at(j).endX));
                linesDataString.append(",");
                linesDataString.append(Utilities::lexical_cast<std::string>(linesData.at(i).at(j).endY));
                linesDataString.append(",");
                linesDataString.append(Utilities::lexical_cast<std::string>(linesData.at(i).at(j).endZ));
                linesDataString.append(",");

                int originalStartType = linesData.at(i).at(j).startType;
                int originalStartID = linesData.at(i).at(j).startID;
                int modifiedStartType = 0;
                int modifiedStartID = 0;
                if (originalStartType == 2) {
                    // 'Other ship' start - modify from global ID to ID for this peer
                    if (originalStartID == shipNumber) {
                        // Refering to the ship we are sending the data to, so 'own ship'
                        modifiedStartType = 1;
                        modifiedStartID = 0;
                    } else {
                        modifiedStartType = originalStartType;
                        if (originalStartID > shipNumber) {
                            modifiedStartID = originalStartID - 1;
                        } else {
                            modifiedStartID = originalStartID;
                        }
                    }    
                } else {
                    // Others (buoys, land objects) don't need modification
                    modifiedStartType = originalStartType;
                    modifiedStartID = originalStartID;
                }

                int originalEndType = linesData.at(i).at(j).endType;
                int originalEndID = linesData.at(i).at(j).endID;
                int modifiedEndType = 0;
                int modifiedEndID = 0;
                if (originalEndType == 2) {
                    // 'Other ship' end - modify from global ID to ID for this peer
                    if (originalEndID == shipNumber) {
                        // Refering to the ship we are sending the data to, so 'own ship'
                        modifiedEndType = 1;
                        modifiedEndID = 0;
                    } else {
                        modifiedEndType = originalEndType;
                        if (originalEndID > shipNumber) {
                            modifiedEndID = originalEndID - 1;
                        } else {
                            modifiedEndID = originalEndID;
                        }
                    }
                } else {
                    // Others (buoys, land objects) don't need modification
                    modifiedEndType = originalEndType;
                    modifiedEndID = originalEndID;
                }

                linesDataString.append(Utilities::lexical_cast<std::string>(modifiedStartType));
                linesDataString.append(",");
                linesDataString.append(Utilities::lexical_cast<std::string>(modifiedEndType));
                linesDataString.append(",");
                linesDataString.append(Utilities::lexical_cast<std::string>(modifiedStartID));
                linesDataString.append(",");
                linesDataString.append(Utilities::lexical_cast<std::string>(modifiedEndID));
                linesDataString.append(",");
                linesDataString.append(Utilities::lexical_cast<std::string>(linesData.at(i).at(j).nominalLength));
                linesDataString.append(",");
                linesDataString.append(Utilities::lexical_cast<std::string>(linesData.at(i).at(j).breakingTension));
                linesDataString.append(",");
                linesDataString.append(Utilities::lexical_cast<std::string>(linesData.at(i).at(j).breakingStrain));
                linesDataString.append(",");
                linesDataString.append(Utilities::lexical_cast<std::string>(linesData.at(i).at(j).nominalShipMass));
                linesDataString.append(",");
                linesDataString.append(Utilities::lexical_cast<std::string>(linesData.at(i).at(j).keepSlack));
                linesDataString.append(",");
                linesDataString.append(Utilities::lexical_cast<std::string>(linesData.at(i).at(j).heaveIn));
                linesDataString.append("|");
            }
        }
    }
    
    //std::cout << "Sending to: " << shipNumber << ": " << linesDataString << std::endl;

    return linesDataString;
}
