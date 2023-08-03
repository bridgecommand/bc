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

#include "irrlicht.h"
#include <string>

//Forward declarations
class SimulationModel;

class Line
{
    public:
        Line();
        virtual ~Line();
        void clearLine(); // Call before we remove the line from the parent vector
        void setStart(irr::scene::ISceneNode* lineStart, int nodeType, int id); // Must always be on own ship
        void setEnd(irr::scene::ISceneNode* lineEnd, irr::f32 shipMass, int nodeType, int id); // Remote connection point
        void setNominalLength(irr::f32 lineNominalLength);

        std::string getLineName() const;
        void setLineName(std::string lineName);
        bool getKeepSlack() const;
        void setKeepSlack(bool keepSlack);
        bool getHeaveIn() const;
        void setHeaveIn(bool heaveIn);

        void setSelected(bool selected);

        irr::f32 getLineStartX() const;
        irr::f32 getLineStartY() const;
        irr::f32 getLineStartZ() const;
        irr::f32 getLineEndX() const;
        irr::f32 getLineEndY() const;
        irr::f32 getLineEndZ() const;
        int getLineStartType() const; //0: Unknown, 1: Own ship, 2: Other ship, 3: Buoy, 4: Land object
        int getLineEndType() const; //0: Unknown, 1: Own ship, 2: Other ship, 3: Buoy, 4: Land object
        int getLineStartID() const;
        int getLineEndID() const;
        irr::f32 getLineNominalLength() const;
        void setLineNominalLength(irr::f32 lineNominalLength);
        irr::f32 getLineBreakingTension() const;
        void setLineBreakingTension(irr::f32 lineBreakingTensio);
        irr::f32 getLineBreakingStrain() const;
        void setLineBreakingStrain(irr::f32 lineBreakingStrain);
        irr::f32 getLineNominalShipMass() const;
        void setLineNominalShipMass(irr::f32 shipNominalMass);


        void update(irr::f32 deltaTime); // Calculate the force and torque acting on the ownship in the local coordinate system
        irr::core::vector3df getLocalForceVector(); // Call after update() to retrieve result
        irr::core::vector3df getLocalTorqueVector(); // Call after update() to retrieve result
        
    private:
        irr::scene::ISceneNode* lineStart;
        irr::scene::ISceneNode* lineEnd;
        irr::scene::IMeshSceneNode* lineVisualisation1;
        irr::scene::IMeshSceneNode* lineVisualisation2;
        irr::f32 shipNominalMass;
        irr::f32 lineNominalLength;
        irr::f32 lineExtension;
        irr::f32 lineBreakingStrain;
        irr::f32 lineBreakingTension;
        std::string lineName;
        bool keepSlack;
        bool heaveIn;
        bool isSelected;
        int startNodeType; //0: Unknown, 1: Own ship, 2: Other ship, 3: Buoy, 4: Land object
        int endNodeType; //0: Unknown, 1: Own ship, 2: Other ship, 3: Buoy, 4: Land object
        int startNodeID;
        int endNodeID;
        irr::core::vector3df localForceVector;
        irr::core::vector3df localTorqueVector;
        
};

#endif