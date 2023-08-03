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

#ifndef __LINES_HPP_INCLUDED__
#define __LINES_HPP_INCLUDED__

#include "irrlicht.h"
#include <string>
#include <vector>

//Forward declarations
class SimulationModel;
class Line;

class Lines
{
    public:
        Lines();
        virtual ~Lines();
        void addLine(bool networkLine = false); // Add a line, which will be undefined
        void setLineStart(irr::scene::ISceneNode* lineStart, int nodeType, int id, bool networkLine = false, int lineID = -1); //Set the start point of the line (default is most recently added line)
        void setLineEnd(irr::scene::ISceneNode* lineEnd, irr::f32 shipMass, int nodeType, int id, bool networkLine = false, int lineID = -1); //Set the end point (default is the most recently added line)
        void clearLine(int lineID, bool networkLine = false);
        void removeLine(int lineID, bool networkLine = false);
        void setSelectedLine(int lineID);
        int getSelectedLine() const;
        int getNumberOfLines(bool networkLine = false) const; // Number of lines (ignoring additional lines added from other ships due to networking by default)
        std::vector<std::string> getLineNames();
        bool getKeepSlack(int lineID, bool networkLine = false);
        void setKeepSlack(int lineID, bool keepSlack, bool networkLine = false);
        bool getHeaveIn(int lineID, bool networkLine = false);
        void setHeaveIn(int lineID, bool heaveIn, bool networkLine = false);

        
        irr::f32 getLineStartX(int lineID, bool networkLine = false);
        irr::f32 getLineStartY(int lineID, bool networkLine = false);
        irr::f32 getLineStartZ(int lineID, bool networkLine = false);
        irr::f32 getLineEndX(int lineID, bool networkLine = false);
        irr::f32 getLineEndY(int lineID, bool networkLine = false);
        irr::f32 getLineEndZ(int lineID, bool networkLine = false);
        int getLineStartType(int lineID, bool networkLine = false); //0: Unknown, 1: Own ship, 2: Other ship, 3: Buoy, 4: Land object
        int getLineEndType(int lineID, bool networkLine = false); //0: Unknown, 1: Own ship, 2: Other ship, 3: Buoy, 4: Land object
        int getLineStartID(int lineID, bool networkLine = false);
        int getLineEndID(int lineID, bool networkLine = false);
        irr::f32 getLineNominalLength(int lineID, bool networkLine = false);
        void setLineNominalLength(int lineID, irr::f32 lineNominalLength, bool networkLine = false);
        irr::f32 getLineBreakingTension(int lineID, bool networkLine = false);
        void setLineBreakingTension(int lineID, irr::f32 lineBreakingTension, bool networkLine = false);
        irr::f32 getLineBreakingStrain(int lineID, bool networkLine = false);
        void setLineBreakingStrain(int lineID, irr::f32 lineBreakingStrain, bool networkLine = false);
        irr::f32 getLineNominalShipMass(int lineID, bool networkLine = false);
        void setLineNominalShipMass(int lineID, irr::f32 lineNominalShipMass, bool networkLine = false);
        
        void update(irr::f32 deltaTime);
        irr::core::vector3df getOverallForceLocal(); // Find sum of forces on own ship in local coordinate system, call after update()
        irr::core::vector3df getOverallTorqueLocal(); // Find sum of torques on own ship in local coordinate system, call after update()

    private:
        std::vector<Line> lines;
        std::vector<Line> networkLines;
        int selectedLine;
};

#endif