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

#include "Line.hpp"
#include <iostream>

Line::Line()
{
    lineNominalLength = 0;
    lineExtension = 0;
    lineBreakingStrain = 0;
    lineBreakingTension = 0;
    shipNominalMass = 0;
    keepSlack = false;
    heaveIn = false;
    isSelected = false;
    lineName = "In progress";

    // Null pointers for start and end scene nodes
    lineStart = 0;
    lineEnd = 0;
    lineVisualisation1 = 0;
    lineVisualisation2 = 0;

    // ID and type of start and end
    startNodeType = 0 ; //0: Unknown, 1: Own ship, 2: Other ship, 3: Buoy, 4: Land object
    endNodeType = 0; //0: Unknown, 1: Own ship, 2: Other ship, 3: Buoy, 4: Land object
    startNodeID = 0;
    endNodeID = 0;
}

Line::~Line()
{
    // Empty, as this also gets called when a copy is made and removed in vector operations
}

void Line::clearLine()
{
    // Call before we remove the line from the parent vector
    if (lineStart) {
        lineStart->remove();
        lineStart = 0;
    }
    if (lineEnd) {
        lineEnd->remove();
        lineEnd = 0;
    }
    if (lineVisualisation1) {
        lineVisualisation1->remove();
        lineVisualisation1 = 0;
    }
    if (lineVisualisation2) {
        lineVisualisation2->remove();
        lineVisualisation2 = 0;
    }
}

void Line::setStart(irr::scene::ISceneNode* lineStart, int nodeType, int id) 
{
    this->lineStart = lineStart;
    startNodeType = nodeType;
    startNodeID = id;
}

void Line::setEnd(irr::scene::ISceneNode* lineEnd, irr::f32 shipMass, int nodeType, int id)
{
    this->lineEnd = lineEnd;
    shipNominalMass = shipMass;
    endNodeType = nodeType;
    endNodeID = id;

    if (nodeType == 2) {
        lineName = "Line to vessel";
    } else if (nodeType == 3) {
        lineName = "Line to buoy";
    } else if (nodeType == 4) {
        lineName = "Line to land";
    } else {
        lineName = "Line";
    }

    //Check we have a non-null start and end, and if so, set nominal length to the distance between them
    if (lineStart && lineEnd && lineStart->getParent()) {
        
        // Find distance between start and end
        lineStart->getParent()->updateAbsolutePosition();
        lineStart->updateAbsolutePosition();

        // We don't check before if the lineEnd has a parent, as this isn't absolutely required
        if (lineEnd->getParent()) {
            lineEnd->getParent()->updateAbsolutePosition();
        }
        lineEnd->updateAbsolutePosition();

        irr::core::vector3df startPosAbs = lineStart->getAbsolutePosition();
        irr::core::vector3df endPosAbs = lineEnd->getAbsolutePosition();
        
        irr::core::vector3df lineVectorAbs = endPosAbs - startPosAbs;
        lineNominalLength = lineVectorAbs.getLength();
        lineExtension = 0; // Initialise

        // make a node to visualise the line itself
        lineVisualisation1 = lineStart->getSceneManager()->addCubeSceneNode(1.0); // Will be scaled and aligned later. No parent as we will control with absolute position
        lineVisualisation2 = lineStart->getSceneManager()->addCubeSceneNode(1.0); // Will be scaled and aligned later. No parent as we will control with absolute position

        //Set lighting to use diffuse and ambient, so lighting of untextured models works
        if(lineVisualisation1->getMaterialCount()>0) {
            for(irr::u32 mat=0;mat<lineVisualisation1->getMaterialCount();mat++) {
                lineVisualisation1->getMaterial(mat).ColorMaterial = irr::video::ECM_DIFFUSE_AND_AMBIENT;
            }
        }
        if(lineVisualisation2->getMaterialCount()>0) {
            for(irr::u32 mat=0;mat<lineVisualisation2->getMaterialCount();mat++) {
                lineVisualisation2->getMaterial(mat).ColorMaterial = irr::video::ECM_DIFFUSE_AND_AMBIENT;
            }
        }

       // Set line properties proportional to the ship size
       lineBreakingStrain = 0.25; // Initial guess
       lineBreakingTension = shipNominalMass * 9.81; // Based on maximum 1g acceleration due to line 

    }
}

void Line::setNominalLength(irr::f32 lineNominalLength)
{
    this->lineNominalLength = lineNominalLength;
}

std::string Line::getLineName() const
{
    return lineName;
}

void Line::setLineName(std::string lineName)
{
    this->lineName = lineName;
}

bool Line::getKeepSlack() const
{
    return keepSlack;
}

void Line::setKeepSlack(bool keepSlack)
{
    this->keepSlack = keepSlack;
    if (keepSlack) {
        heaveIn = false;
    }
}

bool Line::getHeaveIn() const
{
    return heaveIn;
}

void Line::setHeaveIn(bool heaveIn)
{
    this->heaveIn = heaveIn;
    if (heaveIn) {
        keepSlack = false;
    }
}

void Line::setSelected(bool selected)
{
    this->isSelected = selected;
}

irr::f32 Line::getLineStartX() const // Relative position
{
    if (lineStart) {
        return lineStart->getPosition().X;
    } else {
        return 0.0;
    }
}

irr::f32 Line::getLineStartY() const
{
    if (lineStart) {
        return lineStart->getPosition().Y;
    } else {
        return 0.0;
    }
}

irr::f32 Line::getLineStartZ() const
{
    if (lineStart) {
        return lineStart->getPosition().Z;
    } else {
        return 0.0;
    }
}

irr::f32 Line::getLineEndX() const
{
    if (lineEnd) {
        return lineEnd->getPosition().X;
    } else {
        return 0.0;
    }
}

irr::f32 Line::getLineEndY() const
{
    if (lineEnd) {
        return lineEnd->getPosition().Y;
    } else {
        return 0.0;
    }
}

irr::f32 Line::getLineEndZ() const
{
    if (lineEnd) {
        return lineEnd->getPosition().Z;
    } else {
        return 0.0;
    }
}

int Line::getLineStartType() const //0: Unknown, 1: Own ship, 2: Other ship, 3: Buoy, 4: Land object
{
    return startNodeType;
}

int Line::getLineEndType() const //0: Unknown, 1: Own ship, 2: Other ship, 3: Buoy, 4: Land object
{
    return endNodeType;
}

int Line::getLineStartID() const
{
    return startNodeID;
}

int Line::getLineEndID() const
{
    return endNodeID;
}

irr::f32 Line::getLineNominalLength() const
{
    return lineNominalLength;
}

void Line::setLineNominalLength(irr::f32 lineNominalLength)
{
    this->lineNominalLength = lineNominalLength;
}

irr::f32 Line::getLineBreakingTension() const
{
    return lineBreakingTension;
}

void Line::setLineBreakingTension(irr::f32 lineBreakingTension)
{
    this->lineBreakingTension = lineBreakingTension;
}


irr::f32 Line::getLineBreakingStrain() const
{
    return lineBreakingStrain;
}

void Line::setLineBreakingStrain(irr::f32 lineBreakingStrain)
{
    this->lineBreakingStrain=lineBreakingStrain;
}

irr::f32 Line::getLineNominalShipMass() const
{
    return shipNominalMass;
}

void Line::setLineNominalShipMass(irr::f32 shipNominalMass)
{
    this->shipNominalMass=shipNominalMass;
}

void Line::update(irr::f32 deltaTime) // Calculate the force and torque acting on the ownship in the local coordinate system
{
    
    if (lineStart && lineEnd && lineStart->getParent()) {
        
        // Find distance between start and end
        lineStart->getParent()->updateAbsolutePosition();
        lineStart->updateAbsolutePosition();

        // We don't check before if the lineEnd has a parent, as this isn't absolutely required
        if (lineEnd->getParent()) {
            lineEnd->getParent()->updateAbsolutePosition();
        }
        lineEnd->updateAbsolutePosition();

        irr::core::vector3df startPosAbs = lineStart->getAbsolutePosition();
        irr::core::vector3df endPosAbs = lineEnd->getAbsolutePosition();
        
        irr::core::vector3df lineVectorAbs = endPosAbs - startPosAbs;
        irr::f32 lineExtensionPrevious = lineExtension;
        irr::f32 lineActualLength = lineVectorAbs.getLength();
        
        if (keepSlack) {
            lineNominalLength = lineActualLength;
        }

        lineExtension = lineActualLength - lineNominalLength;
        irr::f32 lineExtensionChange = lineExtension - lineExtensionPrevious;

        // Find force in local coordinate system (i.e. force magnitude * local unit vector)
        irr::f32 forceMagnitude = 0;
        if (lineActualLength > 0 && lineNominalLength > 0 && lineBreakingStrain > 0 && lineBreakingTension > 0) {
            // Valid line parameters
            
            if (lineExtension > 0) {
                irr::f32 lineStiffness = lineBreakingTension / (lineNominalLength * lineBreakingStrain);
                
                // Upper limit to stiffness for stability (based on 10m length) - Todo, could be improved?
                if (lineStiffness > (lineBreakingTension / (10 * lineBreakingStrain))) {
                    lineStiffness = lineBreakingTension / (10 * lineBreakingStrain);
                }

                // Ramp in line stiffness for low extensions
                irr::f32 strainProportion = lineExtension / (lineNominalLength * lineBreakingStrain); // Ratio of actual strain to breaking strain
                if (strainProportion < 0.1) {
                    // Linear increase of stiffness up to full stiffness at 10% of breaking strain
                    lineStiffness = lineStiffness * strainProportion / 0.1;
                }


                // Calculate the stiffness based force
                forceMagnitude = lineExtension * lineStiffness;

                // Add damping (50% of critical) here
                if (deltaTime > 0 && shipNominalMass > 0 && lineStiffness > 0) {
                    irr::f32 lineExtensionSpeed = lineExtensionChange / deltaTime;
                    irr::f32 criticalDamping = 2*sqrt(lineStiffness * shipNominalMass);
                    forceMagnitude += lineExtensionSpeed * 0.5 * criticalDamping;
                }

                // Avoid force magnitude going negative with damping
                if (forceMagnitude < 0) {
                    forceMagnitude = 0;
                }
    
            }
            // Reduce line length for next time if 'heave in' is active
            if (!keepSlack && heaveIn) {
                irr::f32 haulInSpeed = 0;
                if (forceMagnitude <= 0) {
                    // 1m/s heave in speed if unloaded
                    haulInSpeed = 1.0;
                } else if (forceMagnitude < 0.1 * lineBreakingTension) {
                    // linear reduction in speed up to zero speed at 10% of line breaking tension
                    haulInSpeed = 1.0 - forceMagnitude/(0.1*lineBreakingTension);
                }
                
                // Don't allow length to get shorter than 1m while hauling in
                if (lineNominalLength > 1.0 + haulInSpeed * deltaTime) {
                    lineNominalLength -= haulInSpeed * deltaTime;
                }

            }  

        }

        // Debugging output
        //std::cout << "Line tension: " << forceMagnitude << " Nominal length: " << lineNominalLength << " Actual distance: " << lineActualLength << std::endl;
        
        // Transform positions to the own ship (start) local coordinate system
        irr::core::vector3df lineStartLocal;
        irr::core::vector3df lineEndLocal;
        irr::core::matrix4 worldToLocal;

        if (startNodeType == 1) {
            // Own ship as start (normal case)
            lineStartLocal = lineStart->getPosition(); //Relative position already
            lineEndLocal = endPosAbs; //Initially the absolute position, will be transformed
            worldToLocal = lineStart->getParent()->getAbsoluteTransformation();
            worldToLocal.makeInverse();
            worldToLocal.transformVect(lineEndLocal);

            irr::core::vector3df lineVectorUnitLocal = lineEndLocal - lineStartLocal;
            lineVectorUnitLocal.normalize();

            localForceVector = lineVectorUnitLocal * forceMagnitude;
            // Find torque in local coordinate system (i.e. cross product of local start position vector with local force vector)
            localTorqueVector = lineStartLocal.crossProduct(localForceVector);
        } else if (endNodeType == 1) {
            // Own ship as end (may occur in network cases)
            lineStartLocal = startPosAbs; //Initially the absolute position, will be transformed
            lineEndLocal = lineEnd->getPosition(); //Relative position already
            worldToLocal = lineEnd->getParent()->getAbsoluteTransformation();
            worldToLocal.makeInverse();
            worldToLocal.transformVect(lineStartLocal);

            irr::core::vector3df lineVectorUnitLocal = lineEndLocal - lineStartLocal;
            lineVectorUnitLocal.normalize();
            
            localForceVector = lineVectorUnitLocal * -1.0 * forceMagnitude;
            // Find torque in local coordinate system (i.e. cross product of local end position vector with local force vector)
            localTorqueVector = lineEndLocal.crossProduct(localForceVector);
        } else {
            // Not connected to own ship, no force or torque
            localForceVector = irr::core::vector3df(0.0, 0.0, 0.0);
            localTorqueVector = irr::core::vector3df(0.0, 0.0, 0.0);
        }

        // Visualisation: Update 3d drawing of the line: Vey simple initially.
        if (lineVisualisation1 && lineVisualisation2) {
            
            // Find mid point between start and end
            irr::core::vector3df lineCentrePoint = (startPosAbs + endPosAbs)/2.0;
            if (lineActualLength < lineNominalLength) {
                // Droop
                lineCentrePoint.Y -= (lineNominalLength - lineActualLength)/2.0;
            }

            // Move line visualisation to mid point
            lineVisualisation1->setPosition((startPosAbs + lineCentrePoint)/2.0);
            lineVisualisation2->setPosition((lineCentrePoint + endPosAbs)/2.0);

            irr::video::SColor lineColour = irr::video::SColor(255, 255, 255, 255);
            
            // set x & y size based on strength what's selected
            irr::f32 xySize = 0;
            if (lineBreakingTension > 0) {
                xySize = sqrt(lineBreakingTension) * 0.00001;
            }
            if (xySize < 0.030) {
                // Min size: 3cm
                xySize = 0.030;
            }
            if (xySize > 0.300) {
                // Max size: 30cm
                xySize = 0.300;
            }
            if (isSelected) {
                xySize *= 2;
                lineColour = irr::video::SColor(255, 255, 0, 0);
            }
            
            // Apply colour to vertex (TODO: Store scene manager, to reduce duplication)
            lineVisualisation1->getSceneManager()->getMeshManipulator()->setVertexColors(lineVisualisation1->getMesh(),lineColour);
            lineVisualisation2->getSceneManager()->getMeshManipulator()->setVertexColors(lineVisualisation2->getMesh(),lineColour);

            // set z scale to line length
            lineVisualisation1->setScale(irr::core::vector3df(xySize,xySize,startPosAbs.getDistanceFrom(lineCentrePoint)));
            lineVisualisation2->setScale(irr::core::vector3df(xySize,xySize,endPosAbs.getDistanceFrom(lineCentrePoint)));

            // Align between points
            irr::core::quaternion quat1;
            irr::core::quaternion quat2;
            quat1.rotationFromTo(irr::core::vector3df(0.0,0.0,1.0), (lineCentrePoint-startPosAbs));
            quat2.rotationFromTo(irr::core::vector3df(0.0,0.0,1.0), (endPosAbs-lineCentrePoint));
            irr::core::vector3df eulerAngle1;
            irr::core::vector3df eulerAngle2;
            quat1.toEuler(eulerAngle1);
            quat2.toEuler(eulerAngle2);
            lineVisualisation1->setRotation(eulerAngle1*irr::core::RADTODEG);
            lineVisualisation2->setRotation(eulerAngle2*irr::core::RADTODEG);
        }

    }
}

irr::core::vector3df Line::getLocalForceVector() // Call after update() to retrieve result
{
    return localForceVector;
}

irr::core::vector3df Line::getLocalTorqueVector() // Call after update() to retrieve result
{
    return localTorqueVector;
}
