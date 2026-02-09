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

#ifndef __LINE_HPP_INCLUDED__
#define __LINE_HPP_INCLUDED__

#include "graphics/Types.hpp"
#include <string>
#include <cstdint>

// Forward declarations
namespace irr {
    namespace scene {
        class ISceneManager;
        class ISceneNode;
        class IMeshSceneNode;
    }
}
class SimulationModel;

class Line
{
    public:
        Line(SimulationModel* model);
        virtual ~Line();
        void clearLine(); // Call before we remove the line from the parent vector
        void setStart(irr::scene::ISceneNode* lineStart, int nodeType, int id); // Must always be on own ship
        void setEnd(irr::scene::ISceneNode* lineEnd, float shipMass, int nodeType, int id, float lengthFactor); // Remote connection point
        void setNominalLength(float lineNominalLength);

        std::string getLineName() const;
        void setLineName(std::string lineName);
        bool getKeepSlack() const;
        void setKeepSlack(bool keepSlack);
        bool getHeaveIn() const;
        void setHeaveIn(bool heaveIn);

        void setSelected(bool selected);

        float getLineStartX() const;
        float getLineStartY() const;
        float getLineStartZ() const;
        float getLineEndX() const;
        float getLineEndY() const;
        float getLineEndZ() const;
        int getLineStartType() const; //0: Unknown, 1: Own ship, 2: Other ship, 3: Buoy, 4: Land object
        int getLineEndType() const; //0: Unknown, 1: Own ship, 2: Other ship, 3: Buoy, 4: Land object
        int getLineStartID() const;
        int getLineEndID() const;
        float getLineNominalLength() const;
        void setLineNominalLength(float lineNominalLength);
        float getLineBreakingTension() const;
        void setLineBreakingTension(float lineBreakingTensio);
        float getLineBreakingStrain() const;
        void setLineBreakingStrain(float lineBreakingStrain);
        float getLineNominalShipMass() const;
        void setLineNominalShipMass(float shipNominalMass);


        void update(float deltaTime); // Calculate the force and torque acting on the ownship in the local coordinate system
        bc::graphics::Vec3 getLocalForceVector(); // Call after update() to retrieve result
        bc::graphics::Vec3 getLocalTorqueVector(); // Call after update() to retrieve result

    private:
        SimulationModel* model;
        irr::scene::ISceneNode* lineStart;
        irr::scene::ISceneNode* lineEnd;
        irr::scene::IMeshSceneNode* lineVisualisation1;
        irr::scene::IMeshSceneNode* lineVisualisation2;
        float shipNominalMass;
        float lineNominalLength;
        float lineExtension;
        float lineBreakingStrain;
        float lineBreakingTension;
        std::string lineName;
        bool keepSlack;
        bool heaveIn;
        bool isSelected;
        int startNodeType; //0: Unknown, 1: Own ship, 2: Other ship, 3: Buoy, 4: Land object
        int endNodeType; //0: Unknown, 1: Own ship, 2: Other ship, 3: Buoy, 4: Land object
        int startNodeID;
        int endNodeID;
        bc::graphics::Vec3 localForceVector;
        bc::graphics::Vec3 localTorqueVector;

};

#endif