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

#include "graphics/Types.hpp"
#include <string>
#include <vector>

// Forward declarations
namespace irr {
    namespace scene {
        class ISceneNode;
    }
}
class SimulationModel;
class Line;

class Lines
{
    public:
        Lines();
        virtual ~Lines();
        void addLine(SimulationModel* model, bool networkLine = false); // Add a line, which will be undefined
        void setLineStart(irr::scene::ISceneNode* lineStart, int nodeType, int id, bool networkLine, int lineID); //Set the start point of the line (-1 for lineID is most recently added line)
        void setLineEnd(irr::scene::ISceneNode* lineEnd, float shipMass, int nodeType, int id, float lengthFactor, bool networkLine, int lineID); //Set the end point (-1 for lineID is the most recently added line)
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

        
        float getLineStartX(int lineID, bool networkLine = false);
        float getLineStartY(int lineID, bool networkLine = false);
        float getLineStartZ(int lineID, bool networkLine = false);
        float getLineEndX(int lineID, bool networkLine = false);
        float getLineEndY(int lineID, bool networkLine = false);
        float getLineEndZ(int lineID, bool networkLine = false);
        int getLineStartType(int lineID, bool networkLine = false); //0: Unknown, 1: Own ship, 2: Other ship, 3: Buoy, 4: Land object
        int getLineEndType(int lineID, bool networkLine = false); //0: Unknown, 1: Own ship, 2: Other ship, 3: Buoy, 4: Land object
        int getLineStartID(int lineID, bool networkLine = false);
        int getLineEndID(int lineID, bool networkLine = false);
        float getLineNominalLength(int lineID, bool networkLine = false);
        void setLineNominalLength(int lineID, float lineNominalLength, bool networkLine = false);
        float getLineBreakingTension(int lineID, bool networkLine = false);
        void setLineBreakingTension(int lineID, float lineBreakingTension, bool networkLine = false);
        float getLineBreakingStrain(int lineID, bool networkLine = false);
        void setLineBreakingStrain(int lineID, float lineBreakingStrain, bool networkLine = false);
        float getLineNominalShipMass(int lineID, bool networkLine = false);
        void setLineNominalShipMass(int lineID, float lineNominalShipMass, bool networkLine = false);
        
        void update(float deltaTime);
        bc::graphics::Vec3 getOverallForceLocal(); // Find sum of forces on own ship in local coordinate system, call after update()
        bc::graphics::Vec3 getOverallTorqueLocal(); // Find sum of torques on own ship in local coordinate system, call after update()

    private:
        std::vector<Line> lines;
        std::vector<Line> networkLines;
        int selectedLine;
};

#endif