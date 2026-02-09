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

#include "Lines.hpp"
#include "Line.hpp"
#include "irrlicht.h"

Lines::Lines()
{
    selectedLine = -1; // None selected
}

Lines::~Lines()
{
}

// Add a line, which will be undefined initially
void Lines::addLine(SimulationModel* model, bool networkLine) 
{
    Line thisLine(model);
    if (networkLine) {
        networkLines.push_back(thisLine);
    } else {
        lines.push_back(thisLine);
    }
}

//Set the start point of the most recently added line
void Lines::setLineStart(irr::scene::ISceneNode* lineStart,  int nodeType, int id, bool networkLine, int lineID) 
{
    std::vector<Line>* thisLines;
    if (networkLine) {
        thisLines = &networkLines;
    } else {
        thisLines = &lines;
    }
    
    if(thisLines->empty()) {
        return;
    }

    if (lineID == -1) {
        // Default, last in list
        thisLines->back().setStart(lineStart, nodeType, id);
    } else {
        if (thisLines->size() > lineID && lineID >= 0) {
            std::vector<Line>::iterator it = thisLines->begin() + lineID;
            it->setStart(lineStart, nodeType, id);
        }
    }
}

//Set the end point of the most recently added line
void Lines::setLineEnd(irr::scene::ISceneNode* lineEnd, float shipMass, int nodeType, int id, float lengthFactor, bool networkLine, int lineID)
{
    std::vector<Line>* thisLines;
    if (networkLine) {
        thisLines = &networkLines;
    } else {
        thisLines = &lines;
    }
    
    if(thisLines->empty()) {
        return;
    }
    if (lineID == -1) {
        // Default, last in list
        thisLines->back().setEnd(lineEnd, shipMass, nodeType, id, lengthFactor);
    } else {
        if (thisLines->size() > lineID && lineID >= 0) {
            std::vector<Line>::iterator it = thisLines->begin() + lineID;
            it->setEnd(lineEnd, shipMass, nodeType, id, lengthFactor);
        }
    }

}

// Number of lines (ignoring additional lines added from other ships due to networking)
int Lines::getNumberOfLines(bool networkLine) const
{
    
    const std::vector<Line>* thisLines;
    if (networkLine) {
        thisLines = &networkLines;
    } else {
        thisLines = &lines;
    }
    
    return thisLines->size();
}

// Return a vector of the current names
std::vector<std::string> Lines::getLineNames()
{
    std::vector<std::string> namesVector;
    for(std::vector<Line>::iterator it = lines.begin(); it != lines.end(); ++it) {
        namesVector.push_back(it->getLineName());
    }

    return namesVector;
}

bool Lines::getKeepSlack(int lineID, bool networkLine)
{
    
    std::vector<Line>* thisLines;
    if (networkLine) {
        thisLines = &networkLines;
    } else {
        thisLines = &lines;
    }
    
    if (thisLines->size() > lineID && lineID >= 0) {
        std::vector<Line>::iterator it = thisLines->begin() + lineID;
        return it->getKeepSlack();
    } else {
        // default
        return false;
    }
    
}

void Lines::setKeepSlack(int lineID, bool keepSlack, bool networkLine)
{
    std::vector<Line>* thisLines;
    if (networkLine) {
        thisLines = &networkLines;
    } else {
        thisLines = &lines;
    }
    if (thisLines->size() > lineID && lineID >= 0) {
        std::vector<Line>::iterator it = thisLines->begin() + lineID;
        it->setKeepSlack(keepSlack);
    }   
}

bool Lines::getHeaveIn(int lineID, bool networkLine)
{
    
    std::vector<Line>* thisLines;
    if (networkLine) {
        thisLines = &networkLines;
    } else {
        thisLines = &lines;
    }
    
    if (thisLines->size() > lineID && lineID >= 0) {
        std::vector<Line>::iterator it = thisLines->begin() + lineID;
        return it->getHeaveIn();
    } else {
        // default
        return false;
    }
    
}

void Lines::setHeaveIn(int lineID, bool heaveIn, bool networkLine)
{
    std::vector<Line>* thisLines;
    if (networkLine) {
        thisLines = &networkLines;
    } else {
        thisLines = &lines;
    }
    if (thisLines->size() > lineID && lineID >= 0) {
        std::vector<Line>::iterator it = thisLines->begin() + lineID;
        it->setHeaveIn(heaveIn);
    }   
}

// Reset a line without removing it
void Lines::clearLine(int lineID, bool networkLine)
{
    std::vector<Line>* thisLines;
    if (networkLine) {
        thisLines = &networkLines;
    } else {
        thisLines = &lines;
    }
    
    if (thisLines->size() > lineID && lineID >= 0) {
        std::vector<Line>::iterator it = thisLines->begin() + lineID;
        it->clearLine();
    }   
}

// Remove a line from the list
void Lines::removeLine(int lineID, bool networkLine) {
    
    std::vector<Line>* thisLines;
    if (networkLine) {
        thisLines = &networkLines;
    } else {
        thisLines = &lines;
    }
    
    if (thisLines->size() > lineID && lineID >= 0) {
        std::vector<Line>::iterator it = thisLines->begin() + lineID;
        it->clearLine();
        thisLines->erase(it);
    }
}

void Lines::setSelectedLine(int lineID) {
    selectedLine = lineID;

    for (int i = 0; i < lines.size(); i++) {
        std::vector<Line>::iterator it = lines.begin() + i;
        if (i == selectedLine) {
            it->setSelected(true);
        } else {
            it->setSelected(false);
        }
    }

}

int Lines::getSelectedLine() const {
    return selectedLine;
}

float Lines::getLineStartX(int lineID, bool networkLine)
{
    
    std::vector<Line>* thisLines;
    if (networkLine) {
        thisLines = &networkLines;
    } else {
        thisLines = &lines;
    }
    
    if (thisLines->size() > lineID && lineID >= 0) {
        std::vector<Line>::iterator it = thisLines->begin() + lineID;
        return it->getLineStartX();
    } else {
        // default
        return false;
    }
}

float Lines::getLineStartY(int lineID, bool networkLine)
{
    std::vector<Line>* thisLines;
    if (networkLine) {
        thisLines = &networkLines;
    } else {
        thisLines = &lines;
    }
    
    if (thisLines->size() > lineID && lineID >= 0) {
        std::vector<Line>::iterator it = thisLines->begin() + lineID;
        return it->getLineStartY();
    } else {
        // default
        return false;
    }
}

float Lines::getLineStartZ(int lineID, bool networkLine)
{
    std::vector<Line>* thisLines;
    if (networkLine) {
        thisLines = &networkLines;
    } else {
        thisLines = &lines;
    }
    
    if (thisLines->size() > lineID && lineID >= 0) {
        std::vector<Line>::iterator it = thisLines->begin() + lineID;
        return it->getLineStartZ();
    } else {
        // default
        return false;
    }
}

float Lines::getLineEndX(int lineID, bool networkLine)
{
    std::vector<Line>* thisLines;
    if (networkLine) {
        thisLines = &networkLines;
    } else {
        thisLines = &lines;
    }
    
    if (thisLines->size() > lineID && lineID >= 0) {
        std::vector<Line>::iterator it = thisLines->begin() + lineID;
        return it->getLineEndX();
    } else {
        // default
        return false;
    }
}

float Lines::getLineEndY(int lineID, bool networkLine)
{
    std::vector<Line>* thisLines;
    if (networkLine) {
        thisLines = &networkLines;
    } else {
        thisLines = &lines;
    }
    
    if (thisLines->size() > lineID && lineID >= 0) {
        std::vector<Line>::iterator it = thisLines->begin() + lineID;
        return it->getLineEndY();
    } else {
        // default
        return false;
    }
}

float Lines::getLineEndZ(int lineID, bool networkLine)
{
    std::vector<Line>* thisLines;
    if (networkLine) {
        thisLines = &networkLines;
    } else {
        thisLines = &lines;
    }
    
    if (thisLines->size() > lineID && lineID >= 0) {
        std::vector<Line>::iterator it = thisLines->begin() + lineID;
        return it->getLineEndZ();
    } else {
        // default
        return false;
    }
}

int Lines::getLineStartType(int lineID, bool networkLine) //0: Unknown, 1: Own ship, 2: Other ship, 3: Buoy, 4: Land object, 5: Terrain
{
    std::vector<Line>* thisLines;
    if (networkLine) {
        thisLines = &networkLines;
    } else {
        thisLines = &lines;
    }
    
    if (thisLines->size() > lineID && lineID >= 0) {
        std::vector<Line>::iterator it = thisLines->begin() + lineID;
        return it->getLineStartType();
    } else {
        // default
        return false;
    }
}

int Lines::getLineEndType(int lineID, bool networkLine) //0: Unknown, 1: Own ship, 2: Other ship, 3: Buoy, 4: Land object, 5: Terrain
{
    std::vector<Line>* thisLines;
    if (networkLine) {
        thisLines = &networkLines;
    } else {
        thisLines = &lines;
    }
    
    if (thisLines->size() > lineID && lineID >= 0) {
        std::vector<Line>::iterator it = thisLines->begin() + lineID;
        return it->getLineEndType();
    } else {
        // default
        return false;
    }
}

int Lines::getLineStartID(int lineID, bool networkLine)
{
    std::vector<Line>* thisLines;
    if (networkLine) {
        thisLines = &networkLines;
    } else {
        thisLines = &lines;
    }
    
    if (thisLines->size() > lineID && lineID >= 0) {
        std::vector<Line>::iterator it = thisLines->begin() + lineID;
        return it->getLineStartID();
    } else {
        // default
        return false;
    }
}

int Lines::getLineEndID(int lineID, bool networkLine)
{
    std::vector<Line>* thisLines;
    if (networkLine) {
        thisLines = &networkLines;
    } else {
        thisLines = &lines;
    }
    
    if (thisLines->size() > lineID && lineID >= 0) {
        std::vector<Line>::iterator it = thisLines->begin() + lineID;
        return it->getLineEndID();
    } else {
        // default
        return false;
    }
}

float Lines::getLineNominalLength(int lineID, bool networkLine)
{
    std::vector<Line>* thisLines;
    if (networkLine) {
        thisLines = &networkLines;
    } else {
        thisLines = &lines;
    }
    
    if (thisLines->size() > lineID && lineID >= 0) {
        std::vector<Line>::iterator it = thisLines->begin() + lineID;
        return it->getLineNominalLength();
    } else {
        // default
        return false;
    }
}

void Lines::setLineNominalLength(int lineID, float lineNominalLength, bool networkLine)
{
    std::vector<Line>* thisLines;
    if (networkLine) {
        thisLines = &networkLines;
    } else {
        thisLines = &lines;
    }
    if (thisLines->size() > lineID && lineID >= 0) {
        std::vector<Line>::iterator it = thisLines->begin() + lineID;
        it->setLineNominalLength(lineNominalLength);
    }
}

float Lines::getLineBreakingTension(int lineID, bool networkLine)
{
    std::vector<Line>* thisLines;
    if (networkLine) {
        thisLines = &networkLines;
    } else {
        thisLines = &lines;
    }
    
    if (thisLines->size() > lineID && lineID >= 0) {
        std::vector<Line>::iterator it = thisLines->begin() + lineID;
        return it->getLineBreakingTension();
    } else {
        // default
        return false;
    }
}

void Lines::setLineBreakingTension(int lineID, float lineBreakingTension, bool networkLine)
{
    std::vector<Line>* thisLines;
    if (networkLine) {
        thisLines = &networkLines;
    } else {
        thisLines = &lines;
    }
    if (thisLines->size() > lineID && lineID >= 0) {
        std::vector<Line>::iterator it = thisLines->begin() + lineID;
        it->setLineBreakingTension(lineBreakingTension);
    }
}

float Lines::getLineBreakingStrain(int lineID, bool networkLine)
{
    std::vector<Line>* thisLines;
    if (networkLine) {
        thisLines = &networkLines;
    } else {
        thisLines = &lines;
    }
    
    if (thisLines->size() > lineID && lineID >= 0) {
        std::vector<Line>::iterator it = thisLines->begin() + lineID;
        return it->getLineBreakingStrain();
    } else {
        // default
        return false;
    }
}

void Lines::setLineBreakingStrain(int lineID, float lineBreakingStrain, bool networkLine)
{
    std::vector<Line>* thisLines;
    if (networkLine) {
        thisLines = &networkLines;
    } else {
        thisLines = &lines;
    }
    if (thisLines->size() > lineID && lineID >= 0) {
        std::vector<Line>::iterator it = thisLines->begin() + lineID;
        it->setLineBreakingStrain(lineBreakingStrain);
    }
}

float Lines::getLineNominalShipMass(int lineID, bool networkLine)
{
    std::vector<Line>* thisLines;
    if (networkLine) {
        thisLines = &networkLines;
    } else {
        thisLines = &lines;
    }
    
    if (thisLines->size() > lineID && lineID >= 0) {
        std::vector<Line>::iterator it = thisLines->begin() + lineID;
        return it->getLineNominalShipMass();
    } else {
        // default
        return false;
    }
}

void Lines::setLineNominalShipMass(int lineID, float lineNominalShipMass, bool networkLine)
{
    std::vector<Line>* thisLines;
    if (networkLine) {
        thisLines = &networkLines;
    } else {
        thisLines = &lines;
    }
    if (thisLines->size() > lineID && lineID >= 0) {
        std::vector<Line>::iterator it = thisLines->begin() + lineID;
        it->setLineNominalShipMass(lineNominalShipMass);
    }
}

void Lines::update(float deltaTime) {
    for(std::vector<Line>::iterator it = lines.begin(); it != lines.end(); ++it) {
        it->update(deltaTime);
    }
    
    for(std::vector<Line>::iterator it = networkLines.begin(); it != networkLines.end(); ++it) {
        it->update(deltaTime);
    }
}

bc::graphics::Vec3 Lines::getOverallForceLocal() {
    // Find sum of forces on own ship in local coordinate system
    // Call after update()
    bc::graphics::Vec3 forceSum;
    for(std::vector<Line>::iterator it = lines.begin(); it != lines.end(); ++it) {
        forceSum += it->getLocalForceVector();
    }
    for(std::vector<Line>::iterator it = networkLines.begin(); it != networkLines.end(); ++it) {
        forceSum += it->getLocalForceVector();
    }
    return forceSum;
}

bc::graphics::Vec3 Lines::getOverallTorqueLocal() {
    // Find sum of torques on own ship in local coordinate system
    // Call after update()
    bc::graphics::Vec3 torqueSum;
    for(std::vector<Line>::iterator it = lines.begin(); it != lines.end(); ++it) {
        torqueSum += it->getLocalTorqueVector();
    }
    for(std::vector<Line>::iterator it = networkLines.begin(); it != networkLines.end(); ++it) {
        torqueSum += it->getLocalTorqueVector();
    }
    return torqueSum;
}